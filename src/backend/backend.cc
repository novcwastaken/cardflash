#include "backend.hh"

#include <iostream>
#include <iterator>
#include <cstdint>
#include <format>
#include <optional>
#include <utility>

#define EVIL_MODE true

namespace Cardflash {
    // inline bool is_little_endian() {
    //     // 00000000 00000001
    //     uint16_t a = 1;
    //     // On littel endian this will point to the second half
    //     // which is 1, on big endian this will point to the first half
    //     uint8_t *p = reinterpret_cast<uint8_t*>(&a);

    //     return *p == 1;
    // }

    inline void U16ToLEBytes(uint16_t value, uint8_t destination[2]) {
        destination[0] = (uint8_t)(value & 0b0000000011111111); // Low byte
        destination[1] = (uint8_t)(value >> 8);                 // High byte
    }

    inline uint16_t LEBytesToU16(uint8_t value[2]) {
        return
            // High byte
            (uint16_t)(value[1]) << 8
            // Low byte
            | value[0];
    }


    // class Card
    inline Card::Card(std::string front, std::string back) {
        if (front.length() == 0 || back.length() == 0)
            throw EmptyString("Tried to init a Card with the front or back being an empty string");

        this->front = std::move(front);
        this->back = std::move(back);
        this->learning_status = CardLearningStatus::Unknown;
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

    inline const std::string Card::LearningStatusFmt() const {
        std::string buff;

        switch (this->learning_status) {
            case CardLearningStatus::Unknown:
                buff = "Unknown";
                break;
            case CardLearningStatus::Know:
                buff = "Know";
                break;
            case CardLearningStatus::Learning:
                buff = "Still learning";
                break;
        }

        return buff;
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
        LearnCorrect,
        ConnectCorrect,
        LearningStatus,
        CardFront,
        CardBack
    };

    /// Deserialization
    Set::Set(std::vector<uint8_t>& in) {
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
        std::vector<uint8_t> buff;

        std::optional<std::string> card_front_buffer = std::nullopt;

        std::vector<CardLearningStatus> card_learning_status;

        for (size_t i = 0; i < in.size(); ++i) {
            const char c = in[i];

            if (c == '\n') {
                // std::cout << "State: " << s << " Buff: " << buff << std::endl;

                switch (s) {
                    case DeserStage::Author:
                        if (buff.empty())
                            throw(DeserializationError(
                                "Missing author field (field is empty)"
                            ));
                        this->author = std::string(buff.begin(), buff.end());
                        buff.clear();

                        break;
                    case DeserStage::Title:
                        if (buff.empty())
                            throw(DeserializationError(
                                "Missing title field (field is empty)"
                            ));
                        this->title = std::string(buff.begin(), buff.end());
                        buff.clear();

                        break;
                    case DeserStage::Subject:
                        if (buff.empty()) this->subject = "";
                        else{
                            this->subject = std::string(buff.begin(), buff.end());
                            buff.clear();
                        }

                        break;
                    case DeserStage::LearnCorrect:
                        if (buff.empty()) break;


                        std::cout << "Learn stats buffer: " << std::endl;
                        for (const auto c : buff) {
                            if (c=='\n')
                                std::cout << "\\n";
                            else
                                std::cout << (char)c;
                        }
                        std::cout << std::endl;

                        if (buff.size() % 2 == 1) throw(DeserializationError(
                            "Learn statistic data is corrupted (odd number of bytes)"
                        ));

                        uint8_t learn_byte_buffer;
                        for (size_t i = 0; i < buff.size(); i += 1) {
                            if (i % 2 == 0) learn_byte_buffer = buff[i];
                            else {
                                uint8_t bytes[2] = {learn_byte_buffer, buff[i]};
                                this->learn_correct.insert(
                                    this->learn_correct.end(),
                                    LEBytesToU16(bytes)
                                );
                            }
                        }

                        buff.clear();

                        break;
                    case DeserStage::ConnectCorrect:
                        if (buff.empty()) break;

                        if (buff.size() % 2 == 1) throw(DeserializationError(
                            "Connect statistic data is corrupted (odd number of bytes)"
                        ));

                        uint8_t connect_byte_buffer;
                        for (size_t i = 0; i < buff.size(); i += 1) {
                            if (i % 2 == 0) connect_byte_buffer = buff[i];
                            else {
                                uint8_t bytes[2] = {connect_byte_buffer, buff[i]};
                                this->connect_correct.insert(
                                    this->learn_correct.end(),
                                    LEBytesToU16(bytes)
                                );
                            }
                        }

                        buff.clear();

                        break;
                    case DeserStage::LearningStatus:
                        if (buff.empty()) break;

                        // 111111** << 6 **000000 >> 6 000000**
                        // 1111**00 << 4 **000000 >> 6 000000**
                        // 11**0000 << 2 **000000 >> 6 0000000
                        // **000000 << 0 **000000 >> 6 0000000

                        for (const auto byte : buff) {
                            for (char bit_offset = 6; bit_offset >= 0; bit_offset -= 2) {
                                uint8_t raw = byte << bit_offset >> 6;
                                if (raw != 0)
                                    card_learning_status
                                        .push_back(static_cast<CardLearningStatus>(raw));
                            }
                        }

                        buff.clear();

                        break;
                    case DeserStage::CardFront:
                        if (buff.empty())
                            throw(DeserializationError(
                                "Missing a card's front field (field is empty)"
                            ));

                        card_front_buffer = std::string(buff.begin(), buff.end());
                        buff.clear();

                        break;
                    case DeserStage::CardBack:
                        if (buff.empty())
                            throw(DeserializationError(
                                "Missing a card's back field (field is empty)"
                            ));

                        if (card_front_buffer) {
                            this->Expand(
                                Card(
                                    std::move(card_front_buffer.value()),
                                    std::string(buff.begin(), buff.end())
                                )
                            );
                            card_front_buffer = std::nullopt;

                            buff.clear();

                            if (this->cards.size() <= card_learning_status.size()) {
                                this
                                    ->cards
                                    .end()
                                    ->learning_status =
                                        card_learning_status
                                        [this->cards.size() -1];
                            } else {
                                throw(DeserializationError(
                                    "There are more learning status stats than cards!"
                                ));
                            }
                        } else throw std::runtime_error("Front card wasn't parsed");

                        break;
                    default:
                        std::unreachable();
                }

                // Move forward, or if at the end of the modes, go back one
                s = static_cast<DeserStage>((s == DeserStage::CardBack) ? s - 1 : s + 1);
            } else {
                buff.push_back(c);
            }
        }
    }

    // ====================================================================

    std::vector<uint8_t> Set::Serialize() {
        if (!this->are_cards_ready)
            throw(SetNotFinalized(
                "Sets are required to be finalized before being serialized!"
            ));

        // How much bytes to allocate for the output buffer
        // This is neede so only 1 allocation ever happens
        // which will make the whole thing faster (probably)
        size_t reserve_size =
            this->author.size() + 1 // Author + \n
            + this->title.size() + 1 // Title + \n
            + this->subject.size() + 1 // Subject + \n
            + (this->learn_correct.size() * 2) + 1 // *2 because each value is 16 bit + \n
            + (this->connect_correct.size() * 2) + 1; // *2 because each value is 16 bit + \n

        // Card learning status size
        size_t bits_needed = this->cards.size() * 2; // Each card takes 2 bits
        size_t padded = bits_needed + (8 - (bits_needed % 8)); // Pad to bytes
        size_t lstatus_bytesize = padded / 8; // Get bytes
        lstatus_bytesize += 1; // + \n at the end
        reserve_size += lstatus_bytesize;

        // + CARD[n].FRONT + \n + CARD[n].BACK + \n
        size_t card_size_sum = 0;
        for (size_t i = 0; i < this->cards.size(); ++i) {
            card_size_sum += this->cards[i].GetFrontAndBackSize() + 2;
        }
        reserve_size += card_size_sum;

        // Evil ass buffer of absolute buffering
        std::vector<uint8_t> buff;
        buff.reserve(reserve_size);

        // AUTHOR\nTITLE\nSUBJECT\n
        std::format_to(
            std::back_inserter(buff),
            "{}\n{}\n{}\n",
            this->author,
            this->title,
            this->subject
        );

        /// little endia array of bytes:  LEARN_CORRECT\n
        for (size_t i = 0; i < this->learn_correct.size(); ++i) {
            uint8_t bytes[2];
            U16ToLEBytes(this->learn_correct[i], bytes);
            buff.insert(buff.end(), std::begin(bytes), std::end(bytes));
        }
        buff.push_back('\n');

        /// little endia array of bytes:  CONNECT_CORRECT\n
        for (size_t i = 0; i < this->connect_correct.size(); ++i) {
            uint8_t bytes[2];
            U16ToLEBytes(this->connect_correct[i], bytes);
            buff.insert(buff.end(), std::begin(bytes), std::end(bytes));
        }
        buff.push_back('\n');


        // array of bytes*: CARD[n].learningstatus\n
        // CARD[n].FRONT\nCARD[n].BACK\n
        // (warning: absolute schizo code)

        // Iterator to where the learning status starts
        auto lstatus_iter = buff.end();

        // 0 initializes the fields needed for the learningstatus
        // byte array
        buff.resize(reserve_size - 1 - card_size_sum + 1);

        // Stores the bits written in the current learningstatus bit
        unsigned short lstatus_bit_offset = 0;
        int lstatus_byte_offset = 0;

        for (size_t i = 0; i < this->cards.size(); ++i) {
            // 0 1 2 3 4 5 6 7      8 9 10 11 12 13 14 15
            if (i % 8 == 0 && i != 0)
                lstatus_byte_offset += 1;
            // Iter to the current byte
            auto lstatus_byte_iter = (lstatus_iter + lstatus_byte_offset);

            // 00000000 >> 6 00000000 | 000000 11 / 10 / 01 << 6 **000000 offset: 0
            // 11000000 >> 4 00110000 | 000000 11 / 10 / 01 << 4 11**0000 offset: 2
            // 11110000 >> 2 00111100 | 000000 11 / 10 / 01 << 2 1111**00 offset: 4
            // 11111100 >> 0 11111100 | 000000 11 / 10 / 01 << 0 111111** offset: 6
            *lstatus_byte_iter = *lstatus_byte_iter >> (6-lstatus_bit_offset)
                | static_cast<uint8_t>(this->cards[i].learning_status)
                << (6-lstatus_bit_offset);

            lstatus_bit_offset += 2;
            if (lstatus_bit_offset >= 8)
               lstatus_bit_offset = 0;

            // Add the terminating \n after the learningstatus
            if (i == this->cards.size() - 1)
                //     else
                *(lstatus_byte_iter + 1) = '\n';


            std::format_to(
                std::back_inserter(buff),
                "{}\n{}\n",
                this->cards[i].GetFront(),
                this->cards[i].GetBack()
            );
        }

        if (reserve_size != buff.size()) {
            // for (const auto c : buff) {
            //     if (c=='\n')
            //         std::cout << "\\n";
            //     else
            //         std::cout << (char)c;
            // }
            // std::cout << std::endl;

            throw(std::runtime_error(std::format(
                "reserver size({}) != buff.size() ({})",
                reserve_size,
                buff.size()
            )));
        }

        // std::cout << "Serialization successfull!" << std::endl;
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