#include <catch2/catch_test_macros.hpp>

#define CARDFLASH_SET_MANAGER_DEBUG
#include "../src/backend/backend.hh"
// #include "../src/backend/backend.cc"
#include "../src/backend/set_manager.cc"

using namespace Cardflash;

#define GENERAL_USECASE_1 true

TEST_CASE("Set manager", "[setmanager]") {
    SetManager man = SetManager();

#if GENERAL_USECASE_1
    SECTION("General usecase 1") {
        Set one = Set("John Doe", "Very Cool Title", "Cool Subject");
        one.Expand(Card("Front", "Back"));
        one.SetLastOpenedTimestamp();
        REQUIRE(one.IsSetFinalized());
        man.AddSet(one);

        Set *oneref = man.GetSet(0);
        REQUIRE(man.IsScanDisabled());
        REQUIRE_THROWS_AS(man.GetSetsRef(), NotAllRefsReturned);
        oneref->title = "Actually, this is cooler!";
        man.Save(oneref);
        man.DropSetRef(oneref);

        Set two = Set("Meow", "Meow (but title)", "Mewo (subject)");
        two.Expand(Card("Me", "Ow"));
        man.AddSet(two);

        // Refs must cannot be reassigend!
        Set *onereftwo = man.GetSet(0);
        onereftwo->author = "Gunter";

        REQUIRE(man.IsScanDisabled());
        REQUIRE_THROWS_AS(man.GetSetsRef(), NotAllRefsReturned);
        REQUIRE_THROWS_AS(man.AddSet(two), NotAllRefsReturned);
        // This was already dropped, and thus shouldn't be able
        // to return it again!
        REQUIRE_THROWS(man.DropSetRef(oneref));

        Set *tworef = man.GetSet(1);
        tworef->subject = "COOL AS SUBJECT FOR THE 2ND THING!";

        REQUIRE(man.IsScanDisabled());
        REQUIRE_THROWS_AS(man.GetSetsRef(), NotAllRefsReturned);

        man.Save(onereftwo);
        man.Save(tworef);

        man.DropSetRef(onereftwo);
        man.DropSetRef(tworef);

        REQUIRE(!man.IsScanDisabled());
        REQUIRE_NOTHROW(man.GetSetsRef());

        REQUIRE_THROWS_AS(man.GetSet(10), std::out_of_range);
    }
#endif

}
