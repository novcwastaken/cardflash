#include "backend.hh"
#include <iterator>

namespace Cardflash {
    // class Card
    inline Card::Card(std::string front, std::string back) {
        if (front.length() == 0 || back.length() == 0)
            throw EmptyString("Tried to init a Card with the front or back being an empty string");

        this->front = std::move(front);
        this->back = std::move(back);
    }

    inline const std::string& Card::GetFront() const {
        return this->front;
    }

    inline const std::string& Card::GetBack() const {
        return this->back;
    }

    inline const void Card::SetFront(std::string s) {
        if (s.length() == 0)
            throw(EmptyString("Called SetFront with s being 0 long!"));

        this->front = s;
    }

    inline const void Card::SetBack(std::string s) {
        if (s.length() == 0)
            throw(EmptyString("Called SetBack with s being 0 long!"));

        this->back = s;
    }
    // end class Card


    // class Set
    inline Set::Set(std::string author, std::string title, std::string subject) {
        if (author.length() == 0 || title.length() == 0)
            throw(EmptyString("Tried to create a set with no author or title!"));

        this->author = author;
        this->title = title;
        this->subject = subject;
    }

    inline const bool Set::AreCardsReady() const {
        return this->are_cards_ready;
    }

    inline const void Set::Expand(Card with)  {
        if (!this->are_cards_ready) {
            this->are_cards_ready = true;
            this->cards = std::vector(1, with);
        } else {
            this->cards.push_back(with);
        }
    }

    inline const void Set::Expand(std::vector<Card> with) {
        if (!this->are_cards_ready) {
            this->are_cards_ready = true;
            this->cards = std::move(with);
        } else {
            this->cards.insert(
                this->cards.end(),
                std::make_move_iterator(with.begin()),
                std::make_move_iterator(with.end())
            );
        }
    }

    inline const void Set::Expand(std::vector<Card>& with) {
        this->are_cards_ready = true;
        this->cards.insert(this->cards.end(), with.begin(), with.end());
    }

    inline const std::vector<Card>& Set::GetRefCards() const {
        return this->cards;
    }
    // end class Set
}