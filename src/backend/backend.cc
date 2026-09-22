#include "backend.hh"

// #include <bitset>
// #include <iostream>

#include <iostream>
#include <iterator>
#include <cstdint>
#include <format>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

#define EVIL_MODE true

namespace Cardflash {
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

    inline uint16_t LEBytesToU16(std::vector<uint8_t>::iterator iter) {
        return
            // High byte
            (uint16_t)( *(iter + 1) ) << 8
            // Low byte
            | *iter;
    }


    // class Card
    inline Card::Card(std::string front, std::string back) {
        if (front.length() == 0 || back.length() == 0)
            throw EmptyString("Tried to init a Card with the front or back being an empty string");

        this->front = std::move(front);
        this->back = std::move(back);
        this->learning_status = CardLearningStatus::Unknown;
    }

    inline Card::Card(std::string front, std::string back, CardLearningStatus lstatus) {
        if (front.length() == 0 || back.length() == 0)
            throw EmptyString("Tried to init a Card with the front or back being an empty string");

        this->front = std::move(front);
        this->back = std::move(back);
        this->learning_status = lstatus;
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
            default:
                buff = "????";
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
        Title,
        Author,
        Subject,
        LearnCorrect,
        ConnectCorrect,
        LearningStatus,
        CardFront,
        CardBack
    };

    /// Deserialization
    Set::Set(std::vector<uint8_t>& in) {
        // Min size of a serialized set is 13 bytes (just the header)
        // + 1 char title + 1 char author
        if (in.size() <= 15)
        #if EVIL_MODE
            throw(DeserializationError(
                "Data is shorter than someone in the team!"
            ));
        #else  // !EVIL_MODE
            throw(DeserializationError(
                "Data too short!"
            ));
        #endif // EVIL_MODE

        // ==== READ HEADER ====
        std::vector<uint8_t>::iterator iter = in.begin();

        uint16_t sizeof_title = LEBytesToU16(iter);
        uint16_t sizeof_author = LEBytesToU16(iter + 2);
        uint16_t sizeof_subject = LEBytesToU16(iter + 4);
        uint16_t sizeof_learn_correct = LEBytesToU16(iter + 6);
        uint16_t sizeof_connect_correct = LEBytesToU16(iter + 8);
        uint16_t sizeof_learning_status = LEBytesToU16(iter + 10); // 10 11
        iter += 12; // [12] == 13th element == last byte of the header
        bool is_learn_correct_u8 = *iter & 0x2;
        bool is_connect_correct_u8 = *iter & 0x1;

        // Header size is 13 bytes
        if (sizeof_title + sizeof_author + sizeof_subject + sizeof_learn_correct +
            sizeof_connect_correct + sizeof_learning_status > static_cast<uint16_t>(in.size()) - 13)
            throw(DeserializationError("Data body too small"));

        // Advance iter to the first data byte
        ++iter;

        // ==== Deser data ====

        this->title = std::string(iter, iter + sizeof_title);
        iter += sizeof_title;

        this->author = std::string(iter, iter + sizeof_author);
        iter += sizeof_author;

        this->subject = std::string(iter, iter + sizeof_subject);
        iter += sizeof_subject;

        if (this->title.empty() || this->author.empty())
            throw(EmptyString("Tried to create a set with no author or title!"));

        for (uint16_t i = 0; i < sizeof_learn_correct; ++i) {
            if (is_learn_correct_u8) {
                this->learn_correct.reserve(sizeof_learn_correct * 2);
                this->learn_correct.push_back(*(iter + i));
            } else {
                this->learn_correct.reserve(sizeof_learn_correct);
                this->learn_correct.push_back(LEBytesToU16(iter + (i * 2)));
            }
        }
        iter += sizeof_learn_correct;

        for (uint16_t i = 0; i < sizeof_connect_correct; ++i) {
            if (is_connect_correct_u8) {
                this->connect_correct.reserve(sizeof_connect_correct * 2);
                this->connect_correct.push_back(*(iter + i));
            } else {
                this->connect_correct.reserve(sizeof_connect_correct);
                this->connect_correct.push_back(LEBytesToU16(iter + (i * 2)));
            }
        }
        iter += sizeof_connect_correct;

        // ==== Learning status ====
        std::vector<uint8_t> learning_status_bytes(iter, iter + sizeof_learning_status);

        std::queue<CardLearningStatus> learning_status_queue;
        bool learning_status_finalized = false;


        for (const auto byte : learning_status_bytes) {
            // std::cout << "Lstatus Deser: byte: " << std::bitset<8>(byte) << std::endl;

            for (int8_t bit_offset = 6; bit_offset >= 0; bit_offset -= 2) {
                uint8_t raw = (byte >> (bit_offset)) & 0b00000011;

                if (raw != 0 && learning_status_finalized)
                    throw(DeserializationError(
                        "Invalid Learning Status byte:non padding\
                            byte after the first padding byte!"
                    ));
                else if (raw != 0){
                    learning_status_queue
                        .push(static_cast<CardLearningStatus>(raw));

                    // std::cout
                    //     << "Pushing Lstatus Deser: bit offset: "
                    //     << (int)bit_offset << "\tpushed: "
                    //     << std::bitset<2>(raw) << std::endl;
                }
                else learning_status_finalized = true;
            }
        }

        learning_status_bytes.clear();

        // Advance iter after the learning status
        iter += sizeof_learning_status;

        // ==== CARDS ====
        bool parsing_front = true;

        std::string buff;
        std::string front_buff;

        auto last = in.end();

        while (iter != last) {
            uint8_t byte = *iter;

            if (byte != '\n') {
                buff += byte;
            }  else {
                if (parsing_front) {
                    if (buff.length() == 0)
                        throw(DeserializationError(
                            "Missing a card's front size (field is empty)"
                        ));

                    front_buff = std::move(buff);
                    parsing_front = false;
                } else {
                    // Shouldn't happen
                    if (front_buff.length() == 0) {
                        throw(std::runtime_error("UB while parsing cards"));
                    }

                    if (buff.empty())
                        throw(DeserializationError(
                            "Missing a card's back field (field is empty)"
                        ));

                    if (learning_status_queue.size() == 0)
                        throw(DeserializationError(
                            "More cards than learning status"
                        ));
                    auto learning_status = learning_status_queue.front();

                    this->Expand(
                        Card(
                            std::move(front_buff),
                            std::move(buff),
                            learning_status
                        )
                    );

                    front_buff.clear();
                    buff.clear();

                    learning_status_queue.pop();
                    parsing_front = true;
                }
            }

            ++iter;
        }

        if (!learning_status_queue.empty()) throw(DeserializationError(
                "More card learning status than cards"
            ));

        if (!front_buff.empty()) throw(DeserializationError(
            "The last card had no back (corrupted data)"
        ));
    }

    // ====================================================================

    std::vector<uint8_t> Set::Serialize() {
        if (!this->are_cards_ready)
            throw(SetNotFinalized(
                "Sets are required to be finalized before being serialized!"
            ));


        bool is_learn_correct_u8 = true;
        for (const auto i : this->learn_correct) {
            if (i > 255) {
                is_learn_correct_u8 = false;
                break;
            }
        }
        size_t learn_correct_size =
            this->learn_correct.size() *
            ((is_learn_correct_u8) ? 1 : 2);


        bool is_connect_correct_u8 = true;
        for (const auto i : this->connect_correct) {
            if (i > 255) {
                is_connect_correct_u8 = false;
                break;
            }
        }
        size_t connect_correct_size =
            this->connect_correct.size() *
            ((is_connect_correct_u8) ? 1 : 2);

        // Card learning status size
        size_t bits_needed = this->cards.size() * 2; // Each card takes 2 bits
        size_t padded = bits_needed + (8 - (bits_needed % 8)); // Pad to bytes
        size_t learning_status_size = padded / 8; // Convert to bytes


        // How much bytes to allocate for the output buffer
        // This is neede so only 1 allocation ever happens
        // which will make the whole thing faster (probably)
        size_t reserve_size =
            // Header: 6 * u16 + 1 byte (bitflag)
            (2 * 6 + 1) +
            // Body: title + author + subject
            this->title.size() + this->author.size() + this->subject.size() +
            // Learn* / Connect* Correct sizes
            learn_correct_size +
            connect_correct_size +
            // The size needed for the learning status
            learning_status_size;

        // + Add size that will be taken up by the cards
        // CARD[n].FRONT + \n + CARD[n].BACK + \n
        size_t card_size_sum = 0;
        for (size_t i = 0; i < this->cards.size(); ++i) {
            card_size_sum += this->cards[i].GetFrontAndBackSize() + 2;
        }
        reserve_size += card_size_sum;

        // Evil ass buffer of absolute buffering
        std::vector<uint8_t> buff;
        buff.reserve(reserve_size);

        // ==== Header ====
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        uint8_t bitflag = 0 |
            (is_learn_correct_u8 ? 0x1 : 0) |
            (is_connect_correct_u8 ? 0x2 : 0);
        uint16_t header[] = {
            static_cast<uint16_t>(this->title.size()),
            static_cast<uint16_t>(this->author.size()),
            static_cast<uint16_t>(this->subject.size()),
            static_cast<uint16_t>(learn_correct_size),
            static_cast<uint16_t>(connect_correct_size),
            static_cast<uint16_t>(learning_status_size),
        };
        for (const uint16_t i : header) {
            uint8_t bytes[2];
            U16ToLEBytes(i, bytes);
            buff.insert(buff.end(), std::begin(bytes), std::end(bytes));
        }
        buff.insert(buff.end(), bitflag);

        // ==== Content ====
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        std::format_to(
            std::back_inserter(buff),
            "{}{}{}",
            this->title,
            this->author,
            this->subject
        );

        /// Learn Correct vector
        for (const auto value : this->learn_correct) {
            if (is_learn_correct_u8) {
                buff.insert(buff.end(), static_cast<uint8_t>(value));
            } else {
                uint8_t bytes[2];
                U16ToLEBytes(value, bytes);
                buff.insert(buff.end(), std::begin(bytes), std::end(bytes));
            }
        }

        /// Connect Correct vector
        for (const auto value : this->connect_correct) {
            if (is_learn_correct_u8) {
                buff.insert(buff.end(), static_cast<uint8_t>(value));
            } else {
                uint8_t bytes[2];
                U16ToLEBytes(value, bytes);
                buff.insert(buff.end(), std::begin(bytes), std::end(bytes));
            }
        }

        // Learning Status bitfield array
        // and the card front/back array

        // To save execution time both the learning status bitarray and the
        // cards are inserted at the same loop. lstatus iter points to the
        // end of the loop after the connect correct values.
        // Then the buffer is resized (it's already allocated memory, BUT
        // the internal .size() of the property can only be moved this way)
        // to the index after the last byte of the learning status array, and
        // the cards are inserted there.


        auto lstatus_iter = buff.end();
        buff.resize(reserve_size - card_size_sum);

        // Stores the bit offset of the learning status inside a byte
        unsigned short lstatus_bit_offset = 0;
        // Stores the byte offset from the start of the learning status array
        int lstatus_byte_offset = 0;

        for (size_t i = 0; i < this->cards.size(); ++i) {
            // ==== LEARNING STATUS ====

            // Iter to the current byte
            auto lstatus_byte_iter = lstatus_iter + lstatus_byte_offset;

            // Modify the correct bit of the byte
            *lstatus_byte_iter |=
                static_cast<uint8_t>(this->cards[i].learning_status)
                << (6 - lstatus_bit_offset);

            // Advance the offsets
            lstatus_bit_offset += 2;
            if (lstatus_bit_offset >= 8) {
               lstatus_bit_offset = 0;
               ++lstatus_byte_offset;
            }

            // ==== Cards ====
            std::format_to(
                std::back_inserter(buff),
                "{}\n{}\n",
                this->cards[i].GetFront(),
                this->cards[i].GetBack()
            );
        }

        // Safeguard against bugs / optimized out in release
        if (reserve_size != buff.size()) {
            #ifdef NDEBUG
                std::unreachable();
            #else
                throw(std::runtime_error(std::format(
                    "reserver size({}) != buff.size() ({})",
                    reserve_size,
                    buff.size()
                )));
            #endif // ifdef NDEBUG
        }

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
                    "\t{}. Front: {} -- Back: {} -- LearningStatus: {}\n",
                    i,
                    card->GetFront(),
                    card->GetBack(),
                    card->LearningStatusFmt()
                ));
            }
        }

        return buff;
    }
    // end class Set
}
