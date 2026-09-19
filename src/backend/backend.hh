#pragma once
#include <stdexcept>
#ifndef __BACKEND_HPP_GUARD__
#define __BACKEND_HPP_GUARD__

#include <string>
#include <vector>
#include <cstdint>

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

    class Card {
        std::string front, back;
        public:
            /// Throws a EmptyString if either front or back
            /// has a lenght of 0.
            Card(std::string front, std::string back);

            const std::string& GetFront() const;
            const std::string& GetBack() const;

            /// Throws EmptyString if s.length() is 0
            const void SetFront(std::string s);

            /// Throws EmptyString if s.length() is 0
            const void SetBack(std::string s);

            /// Returns the length of the front and back
            inline const size_t GetFrontAndBackSize() const;
    };

    class Set {
        /// False if the set was created but the cards still
        /// holds no members. Becomes true when the first Expand()
        /// is called.
        bool are_cards_ready = false;
        public:
            std::string author, title, subject;
            std::vector<Card> cards;

            /// Throws an EmptyString if either title of author is empty
            Set(std::string author, std::string title, std::string subject);

            /// Constructs a set from a Serialize()-d array of bytes. Throws
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
            /// Serialized representation:
            /// AUTHOR \n TITLE \n SUBJECT \n
            /// CARD[n].FRONT \n CARD[n].BACK \n
            /// CARD[n].FRONT \n CARD[n].BACK \n
            std::vector<uint8_t> Serialize();
    };
}

#endif // __BACKEND_HPP_GUARD__
