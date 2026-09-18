#include <string>
#include <vector>

#include "backend.hpp"

namespace Cardflash {
    class Set {
        private:
            std::string name;
            std::string author;
            std::string subject;

            std::vector<Cardflash::Card> cards;
    };
}