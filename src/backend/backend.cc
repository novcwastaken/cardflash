#include "backend.hh"

#include <iterator>
#include <cstdint>
#include <format>
#include <optional>

#define EVIL_MODE true

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

    inline const size_t Card::GetFrontAndBackSize() const {
        return this->front.size() + this->back.size();
    }
    // end class Card


    // class Set
    inline Set::Set(std::string author, std::string title, std::string subject) {
        if (author.length() == 0 || title.length() == 0)
            throw(EmptyString("Tried to create a set with no author or title!"));

        this->author = std::move(author);
        this->title = std::move(title);
        this->subject = std::move(subject);
    }

    enum DeserStage : short {
        Author,
        Title,
        Subject,
        CardFront,
        CardBack
    };

    /// Deserialization
    inline Set::Set(std::vector<uint8_t>& in) {
        // Min size of a set is 8 bytes: ?\n?\n\n?\n?
        if (in.size() < 8)
        #if EVIL_MODE
            throw(DeserializationError(
                "Provided data is shorter than someone in the team!"
            ));
        #else  // !EVIL_MODE
            throw(DeserializationError(
                "Provided data is shorter than the minimum valid size!"
            ));
        #endif // EVIL_MODE

        DeserStage s = DeserStage::Author;
        std::string buff = std::string();
        std::optional<std::string> card_front_buffer = std::nullopt;

        for (size_t i = 0; i < in.size(); ++i) {
            char c = in[i];

            if (c == '\n' || i == in.size() - 1) {
                // std::cout << "State: " << s << " Buff: " << buff << std::endl;

                switch (s) {
                    case DeserStage::Author:
                        if (buff.empty())
                            throw(DeserializationError(
                                "Missing author field (field is empty)"
                            ));
                        this->author = std::move(buff);

                        break;
                    case DeserStage::Title:
                        if (buff.empty())
                            throw(DeserializationError(
                                "Missing title field (field is empty)"
                            ));
                        this->title = std::move(buff);

                        break;
                    case DeserStage::Subject:
                        if (buff.empty()) this->subject = "";
                        else this->subject = std::move(buff);

                        break;
                    case DeserStage::CardFront:
                        if (buff.empty())
                            throw(DeserializationError(
                                "Missing a card's front field (field is empty)"
                            ));
                        card_front_buffer = std::move(buff);

                        break;
                    case DeserStage::CardBack:
                        if (buff.empty())
                            throw(DeserializationError(
                                "Missing a card's back field (field is empty)"
                            ));

                        if (card_front_buffer) {
                            this->Expand(
                                Card(std::move(card_front_buffer.value()), std::move(buff))
                            );

                            card_front_buffer = std::nullopt;
                        } else throw std::runtime_error("Front card wasn't parsed");

                        break;
                }

                // Move forward, or if at the end of the modes, go back one
                s = static_cast<DeserStage>((s == DeserStage::CardBack) ? s - 1 : s + 1);
            } else
                buff.push_back(c);
        }
    }

    std::vector<uint8_t> Set::Serialize() {
        if (!this->are_cards_ready)
            throw(SetNotFinalized(
                "Sets are required to be finalized before being serialized!"
            ));

        // How much bytes to allocate for the output buffer
        size_t reserve_size =
            this->author.size() + 1 // Author + \n
            + this->title.size() + 1 // Title + \n
            + this->subject.size() + 1; // Subject + \n

        // + CARD[n].FRONT + \n + CARD[n].BACK + \n
        for (size_t i = 0; i < this->cards.size(); ++i) {
            reserve_size += this->cards[i].GetFrontAndBackSize() + 2;
        }

        std::vector<uint8_t> buff;
        buff.reserve(reserve_size);

        std::format_to(
            std::back_inserter(buff),
            "{}\n{}\n{}\n",
            this->author,
            this->title,
            this->subject
        );

        for (size_t i = 0; i < this->cards.size(); ++i) {
            std::format_to(
                std::back_inserter(buff),
                "{}\n{}\n",
                this->cards[i].GetFront(),
                this->cards[i].GetBack()
            );
        }

        if (reserve_size != buff.size())
            throw(std::runtime_error(std::format(
                "reserver size({}) != buff.size() ({})",
                reserve_size,
                buff.size()
            )));

        return buff;
    }


    inline const bool Set::IsSetFinalized() const {
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
        if (!this->are_cards_ready)
            throw(SetNotFinalized("The set is not ready to be read! (cards are not ready)"));

        return this->cards;
    }

    inline const std::string Set::DebugFmt() const {
        std::string buff;

        buff.append(std::format(
            "Title: {}, Author: {}, Subject: {}, Cards:",
            this->title,
            this->author,
            this->subject
        ));

        if (!this->are_cards_ready)
            buff.append(" NONE\n");
        else {
            buff.append("\n");
            for (size_t i = 0; i < this->cards.size(); ++i) {
                auto card = &this->cards[i];
                buff.append(std::format(
                    "\t{}. Front: {} -- Back: {}\n",
                    i,
                    card->GetFront(),
                    card->GetBack()
                ));
            }
        }

        return buff;
    }
    // end class Set
}