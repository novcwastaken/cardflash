#pragma once
#include <atomic>
#ifndef __BACKEND_HPP_GUARD__
#define __BACKEND_HPP_GUARD__

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

            /// Returns whether the set is ready to be read (finalized).
            /// Calling GetRefCards while this is false will cause SetNotFinalized
            /// to be thrown!
            inline const bool IsSetFinalized() const;

            inline const void Expand(Card with);

            inline const void Expand(std::vector<Card> with);

            inline const void Expand(std::vector<Card>& with);

            /// Returns a static reference to the internal card vector.
            /// Do not modify it!
            ///
            /// Throws SetNotFinalized if IsSetFinalized is false.
            /// To finalize a set call Expand at least once!
            inline const std::vector<Card>& GetRefCards() const;


            /// Serializes the object into an array of bytes.
            ///
            /// Throws:
            ///     SetNotFinalized if the set is not finalized!
            ///
            /// Serialized representation:
            ///
            /// Header (each value is u16 unless stated otherwise):
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
            /// Min size is 13!
            std::vector<uint8_t> Serialize();

            /// Returns a debug string
            inline const std::string DebugFmt() const;
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
        Failed
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

        public:
            /// Throws SingletonAlreadyInited if SetManager
            /// was already instantiated
            SetManager();

            /// Scans the storage directory for new sets.
            /// No other method will run if a scan is running,
            /// but it'll run in the background in another thread.
            void Scan();

            /// Search through the sets and return a list of results.
            ///
            /// Special attributes are accepted:
            ///     @title / @t             searches through titles
            ///     @author / @from / @a    searches authors
            ///     @subject / @s           searches through subjects
            ///     @card                   searches through all cards (only done by this flag, not by default)
            void Search(std::string query);

            /// Try to import a card located at path
            void Import(std::string path);

            /// Opens a file dialog for the user to chose where to save a set
            void Export(const Set& set);

            /// Gets an immutable pointer to the Set.
            /// Should be dropped after every frame, and a new
            /// ref be acquired at the start of the frame!
            const std::vector<Set>* GetSetsRef();

            bool DidScanFail();


    };
}

#endif // __BACKEND_HPP_GUARD__
