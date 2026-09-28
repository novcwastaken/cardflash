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

        man.AddSet(one);
        man.Scan();
        while (man.IsScanning());

        Set &oneref = man.GetSet(0);
        oneref.title = "Actually, this is cooler!";
        man.Save(oneref);
        man.DropSetRef(oneref);

        Set two = Set("Meow", "Meow (but title)", "Mewo (subject)");
        two.Expand(Card("Me", "Ow"));
        man.AddSet(two);
        while (man.IsScanning());

        auto list = man.GetSetsRef();
        // std::cout << "Things to be listed: (" << list.size() << ")" << std::endl;
        // for (const auto& i : list) {
        //     std::cout << i.DebugFmt() << std::endl;
        // }
        // std::cout << "That's it!" << std::endl;

        Set &tworef = man.GetSet(1);
        oneref = man.GetSet(0);
        oneref.author = "Gunter";
        man.Save(oneref);

        tworef.subject = "COOL AS SUBJECT FOR THE 2ND THING!";

        man.DropSetRef(oneref);

        // Blows up
        // list = man.GetSetsRef();

        // Blows up
        // Set three = Set("Three", "Gunter (three)", "IDFK");
        // two.Expand(Card("Ow", "wO"));
        // man.AddSet(three);

        // Doesn't do anything (scan is disabled)
        man.Scan();

        man.DropSetRef(tworef);

        REQUIRE(!man.IsScanDisabled());
        man.Scan();
        while (man.IsScanning());
    }
#endif

}