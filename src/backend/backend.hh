#pragma once
#ifndef __BACKEND_HPP_GUARD__
#define __BACKEND_HPP_GUARD__

#include "SDL3/SDL_stdinc.h"
#include "uuid_v4.h"

#include <map>
#include <atomic>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace Cardflash {
    class EmptyString : public std::runtime_error {
        public:
            explicit EmptyString(const std::string& msg)
                   : std::runtime_error(msg) {}
    };

    class DeserializationError : public std::runtime_error {
        public:
            explicit DeserializationError(const std::string& msg)
                   : std::runtime_error(msg) {}
    };

    class SetNotFinalized : public std::runtime_error {
        public:
            explicit SetNotFinalized(const std::string& msg)
                : std::runtime_error(msg) {}
    };

    class SingletonAlreadyInited : public std::runtime_error {
        public:
            explicit SingletonAlreadyInited(const std::string& msg)
                : std::runtime_error(msg) {}
    };

    class ScanRunning : public std::runtime_error {
        public:
            explicit ScanRunning()
                : std::runtime_error(
                    "A scan was already started!"
                ) {}
    };

    class NotAllRefsReturned : public std::runtime_error {
        public:
            explicit NotAllRefsReturned(const std::string& msg)
                : std::runtime_error(msg) {}
    };

    enum CardLearningStatus : uint8_t {
        Unknown = 0b00000011, // 0
        Learning = 0b00000001, // 1
        Know = 0b00000010, // 2
    };

    class Card {
        std::string front, back;
        public:
            CardLearningStatus learning_status;

            /// Throws a EmptyString if either front or back
            /// has a lenght of 0.
            Card(std::string front, std::string back);

            Card(std::string front, std::string back, CardLearningStatus lstatus);

            const std::string& GetFront() const;
            const std::string& GetBack() const;

            /// Throws EmptyString if s.length() is 0
            const void SetFront(std::string s);

            /// Throws EmptyString if s.length() is 0
            const void SetBack(std::string s);

            /// Returns the length of the front and back
            inline const size_t GetFrontAndBackSize() const;

            inline const std::string LearningStatusFmt() const;
    };

    class Set {
        #ifdef CARDFLASH_BACKEND_DEBUG
        public:
        #endif

        /// False if the set was created but the cards still
        /// holds no members. Becomes true when the first Expand()
        /// is called.
        bool are_cards_ready = false;

        std::vector<Card> cards;

        UUIDv4::UUID uuid;

        Sint64 last_opened_timestamp = 0;

        #ifndef CARDFLASH_BACKEND_DEBUG
        public:
        #endif
            std::string author, title, subject;

            /// The correct answers from the user in learn mode.
            /// One element represents one run in learn mode.
            std::vector<uint16_t> learn_correct;
            /// The correct answers from the user in connect mode.
            /// One element represents one run in connect mode.
            std::vector<uint16_t> connect_correct;

            /// Throws an EmptyString if either title of author is empty
            Set(std::string author, std::string title, std::string subject);

            /// Constructs a set from a Serialize()-d array of bytes.
            /// May throw DeserializationError if the input isn't valid
            /// or an EmptyString if a card is being initialized with
            /// empty back or front.
            Set(std::vector<uint8_t>& serialized);

            const UUIDv4::UUID& GetUUID() const;

            /// Returns whether the set is ready to be read (finalized).
            /// Calling GetRefCards while this is false will cause SetNotFinalized
            /// to be thrown!
            const bool IsSetFinalized() const;

            const void Expand(Card with);

            const void Expand(std::vector<Card> with);

            const void Expand(std::vector<Card>& with);

            const void SetLastOpenedTimestamp();

            const int64_t GetLastOpenedTimestamp() const;

            /// Returns a static reference to the internal card vector.
            /// Do not modify it!
            ///
            /// Throws SetNotFinalized if IsSetFinalized is false.
            /// To finalize a set call Expand at least once!
            const std::vector<Card>& GetRefCards() const;


            /// Serializes the object into an array of bytes.
            ///
            /// Throws:
            ///     SetNotFinalized if the set is not finalized!
            ///
            /// Serialized representation:
            ///
            /// Header (each value is u16 unless stated otherwise):
            ///     16 bytes: UUID
            ///     8 bytes: last opened unix timestamp (signed)
            ///     SizeOf(Title)
            ///     SizeOf(Author)
            ///     SizeOf(Subject)
            ///     SizeOf(LearnCorrect)
            ///     SizeOf(ConnectCorrect)
            ///     SizeOf(LearningStatus)
            ///     Learn- / ConnectCorrect size bitflag: 0x1 | 0x2
            ///         (if set the indicated set only uses u8, if not u16)
            /// Content:
            ///     u8 array (text): Title
            ///     u8 array (text): Author
            ///     u8 array (text): Subject
            ///     u8/16 (see header) array (binary vector): Learn Correct
            ///     u8/16 (see header) array (binary vector): Connect Correct
            ///     Bitfield array (see later): Learning Status
            ///     Card[N].Front   (string) + \n
            ///     Card[N].Back    (string) + \n
            ///
            /// Learning Status bitfield array:
            /// 1 byte stores 4 cards' learning status (left -> right = [0] -> [n]):
            ///     11: Unknown
            ///     10: Known
            ///     01: Still learning
            ///     00: Not occupied
            ///
            /// Min size is 37 bytes!
            std::vector<uint8_t> Serialize();

            /// Returns a debug string
            const std::string DebugFmt() const;
    };

    enum class BufferState : short {
        /// A worker may be dispatched, the buffer is empty
        Empty,
        /// A worker is currently dispatched, modifying the
        /// buffer may lead to UB!
        Working,
        /// The worker finished working, and the buffer is
        /// ready to be consumed.
        Ready,
        /// The worker finished working, but something went wrong.
        /// The buffer shouldn't be read!
        Failed,
        /// Disabled means no buffer activity may be done.
        /// This should be set when the set should not change
        Disabled
    };

    /// A singleton object responsible for managing sets.
    class SetManager {
        std::vector<Set> sets;

        // An atomic buffer mechanism for Scan to be able
        // to read the directory and produce an update set
        // array.
        std::atomic<BufferState> buffer_state {BufferState::Empty};
        std::vector<Set> buffer;
        bool did_scan_fail { false };

        /// Stores pointers "handed out" by GetSet
        std::map<Set*, size_t> refcount;

        /// Returns true if all the references are returned (refcount is empty)
        bool AllSetRefsReturned() const;

        BufferState CheckBufferState();
        void DisableBuffer();


        public:
            /// Set by the manager or anything borrowing a set
            /// when something in a set's property changes. The
            /// manager never sets it to false, nor it uses it for
            /// any internal machinery.
            bool sets_changed = false;

            /// Throws SingletonAlreadyInited if SetManager
            /// was already instantiated
            SetManager();

            /// Scans the storage directory for new sets.
            /// This might mutate the buffer, and thus may only be called
            /// if there are no mutable refrences out there.
            void Scan();

            /// Returns whether the last scan failed and
            /// moves the buffer to the sets.
            bool DidScanFail();

            /// Returns whether a scan is in progress.
            bool IsScanning();

            /// Returns whether scanning is currently disabled!
            bool IsScanDisabled();

            /// Gets an immutable pointer to the Sets.
            /// Should be dropped after every frame, and a new
            /// ref be acquired at the start of the frame!
            ///
            /// Throws NotAllRefsReturned if not all references
            /// acquired with GetSet is returned (see DropSetRef).
            const std::vector<Set>& GetSetsRef();

            /// Returns a clone of the current Sets.
            ///
            /// Calling this often will cause a lot of
            /// memory usage, so dont.
            std::vector<Set> GetSetsClone();

            /// Gets a mutable reference to a Set specified by the index
            /// in the Sets. Disables starting a new scan until all the
            /// references has been dropped (see DropRef()).
            ///
            /// Throws ScanRunning if a scan is running.
            ///
            /// Throws an std::out_of_range if the index is greater
            /// than the number of stored sets.
            ///
            /// No ref by GetSetsRef should be active at the same time
            /// a Set reference is live. This is not enforced, but all Sets
            /// must be returned with DropSetRef before a new ref
            /// to sets could be acquired!
            Set& GetSet(size_t index);

            /// Removes the internal reference counting of a Set.
            ///
            /// Explodes if a passed in ref wasn't borrowed (or was
            ///     alrady dropped!)
            ///
            /// Using a reference after this is called is UB! (pls dont :3)
            void DropSetRef(Set &set);

            /// Saves a set after it has been modified.
            ///
            /// Don't forget to return the ref (with DropSetRef)
            /// if you don't need a mutable reference anymore.
            void Save(Set &set);


            /// Adds a set to the set collection / saves it.
            ///
            /// Using this while a scan is running or while any
            /// references are (mutable or not) is UB. If any mutable
            /// set references are still out (Not al returned it with
            /// DropSetRef) throws a NotAllRefsReturned!
            ///
            /// It's also forbidden to call this while  ScanRunning()
            /// is true. If done a ScanRunning will be thrown!
            void AddSet(Set set);

            /// Search through the sets and return a list of results.
            ///
            /// Special attributes are accepted:
            ///     @title / @t             searches through titles
            ///     @author / @from / @a    searches authors
            ///     @subject / @s           searches through subjects
            ///     @card                   searches through all cards' content
            ///         (only done by this flag, not by default)
            void Search(std::string query);

            /// Try to import a card located at path
            void Import(std::string path);

            /// Opens a file dialog for the user to chose where to save a set
            void Export(const Set& set);
    };
}

#endif // __BACKEND_HPP_GUARD__
