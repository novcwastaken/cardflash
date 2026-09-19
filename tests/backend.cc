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
        a.GetRefCards().size() == b.GetRefCards().size();

    // If these parameters don't match the cards shouldn't be matched either
    if (!base) return base;

    auto cards_a = a.GetRefCards();
    auto cards_b = b.GetRefCards();
    for (size_t i = 0; i < cards_a.size(); ++i) {
        if (
        cards_a[i].GetFront() != cards_b[i].GetFront() ||
        cards_a[i].GetBack() != cards_b[i].GetBack())
            return false;
    }

    return true;
}

TEST_CASE("(De)serialization of Set", "[serde]") {
    Cardflash::Set set("John Doe", "Cool Title", "Even cooler subject");
    set.Expand({
        Cardflash::Card("Front", "Back"),
        Cardflash::Card("Cool Front", "Cool Back")
    });

    REQUIRE(set.IsSetFinalized());

    SECTION("Serde test") {
        auto ser = set.Serialize();
        Cardflash::Set deser(ser);

        // std::cout << "Set: " << set.DebugFmt() <<
        //     "Deserialized: " << deser.DebugFmt() << std::endl;

        REQUIRE((set == deser));
    }

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


}