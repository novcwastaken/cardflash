#pragma once
#include <stdexcept>
#ifndef __BACKEND_HPP_GUARD__
#define __BACKEND_HPP_GUARD__

#include <string>
#include <vector>

namespace Cardflash {
    class EmptyString : public std::runtime_error {
        public:
            explicit EmptyString(const std::string& msg)
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
    };

    class Set {
        /// False if the set was created but the cards still
        /// holds no members. Becomes true when the first Expand()
        /// is called.
        bool are_cards_ready = false;
        public:
            std::string author, title, subject;
            std::vector<Card> cards;

            /// Throws an EmptyString if title or author is empty,
            /// but subject may by empty
            Set(std::string author, std::string title, std::string subject);

            /// Returns whether the set is ready to be read. Calling get
            /// while this is false isn't correct.
            inline const bool AreCardsReady() const;

            inline const void Expand(Card with);

            inline const void Expand(std::vector<Card> with);

            inline const void Expand(std::vector<Card>& with);

            inline const std::vector<Card>& GetRefCards() const;
    };
}

#endif // __BACKEND_HPP_GUARD__