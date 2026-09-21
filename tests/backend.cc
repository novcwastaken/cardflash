#include <iostream>
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#define CARDFLASH_BACKEND_DEBUG
#include "../src/backend/backend.hh"
#include "../src/backend/backend.cc"

bool operator==(const Cardflash::Set& a, const Cardflash::Set& b) {
    bool base = a.author == b.author &&
        a.title == b.title &&
        a.subject == b.subject &&
        a.IsSetFinalized() == b.IsSetFinalized() &&
        a.GetRefCards().size() == b.GetRefCards().size() &&
        a.connect_correct.size() == b.connect_correct.size() &&
        a.learn_correct.size() == b.learn_correct.size();

    // If these parameters don't match the cards shouldn't be matched either
    if (!base) return base;

    auto cards_a = a.GetRefCards();
    auto cards_b = b.GetRefCards();
    for (size_t i = 0; i < cards_a.size(); ++i) {
        if (
            cards_a[i].GetFront() != cards_b[i].GetFront() ||
            cards_a[i].GetBack() != cards_b[i].GetBack() ||
            cards_a[i].learning_status != cards_b[i].learning_status
        )
            return false;
    }

    for (size_t i = 0; i < a.connect_correct.size(); ++i) {
        if (a.connect_correct[i] != b.connect_correct[i]) {
            return false;
        }
    }

    for (size_t i = 0; i < a.learn_correct.size(); ++i) {
        if (a.learn_correct[i] != b.learn_correct[i]) {
            return false;
        }
    }

    return true;
}

#define SINGLE_SECTION_MODE

TEST_CASE("(De)serialization of Set", "[serde]") {
    Cardflash::Set set("John Doe", "Cool Title", "Even cooler subject");
    set.Expand({
        Cardflash::Card("Front", "Back"),
        Cardflash::Card("Cool Front", "Cool Back"),
        Cardflash::Card("Mrow", "Mrow(Back)", Cardflash::CardLearningStatus::Know),
        Cardflash::Card(
            "What year did ww2 end?",
            "The year after the year before ww2 ended",
            Cardflash::CardLearningStatus::Learning
        ),
        Cardflash::Card(
            "Front (but cooler)",
            "Back (even cooler)",
            Cardflash::CardLearningStatus::Unknown
        ),
        Cardflash::Card("John", "Doe", Cardflash::CardLearningStatus::Know),
        Cardflash::Card(
            "Ran out of ideas",
            "Ran out of ideas (but back)",
            Cardflash::CardLearningStatus::Unknown
        ),
        Cardflash::Card(
            "Love is in the air?",
            "Wrong! Gas leak!",
            Cardflash::CardLearningStatus::Learning
        ),
        Cardflash::Card(
            "What year did ww2 end?",
            "The year after the year before ww2 ended",
            Cardflash::CardLearningStatus::Learning
        ),
        Cardflash::Card(
            "Front (but cooler)",
            "Back (even cooler)",
            Cardflash::CardLearningStatus::Unknown
        ),
        Cardflash::Card("John", "Doe", Cardflash::CardLearningStatus::Know),
        Cardflash::Card(
            "Ran out of ideas",
            "Ran out of ideas (but back)",
            Cardflash::CardLearningStatus::Unknown
        ),
        Cardflash::Card(
            "Love is in the air?",
            "Wrong! Gas leak!",
            Cardflash::CardLearningStatus::Learning
        ),
    });

    // Still doesn't work properly
    set.connect_correct.insert(set.connect_correct.end(), {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    // set.learn_correct.insert(set.learn_correct.end(), {1, 2, 3, 4, 5, 6, 7, 8, 10});

    REQUIRE(set.IsSetFinalized());

    SECTION("Serde test") {
        auto ser = set.Serialize();
        Cardflash::Set deser(ser);

        // std::cout << "Set: " << set.DebugFmt() <<
        //     "Deserialized: " << deser.DebugFmt() << std::endl;

        REQUIRE((set == deser));
    }

#ifndef SINGLE_SECTION_MODE
    SECTION("Serde test: no subject") {
        set.subject = "";

        auto ser = set.Serialize();
        Cardflash::Set deser(ser);

        // std::cout << "Set: " << set.DebugFmt() <<
        //     "Deserialized: " << deser.DebugFmt() << std::endl;

        REQUIRE((set == deser));
    }


    SECTION("Serde test: no cards") {
        set.cards.clear();
        set.are_cards_ready = false;

        REQUIRE_THROWS_AS(set.Serialize(), Cardflash::SetNotFinalized);
    }
#endif
}