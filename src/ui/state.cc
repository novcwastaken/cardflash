// The state of the UI

#include "state.hh"
#include "backend/backend.hh"
#include "uuid_v4.h"
#include <algorithm>
#include <imgui.h>
#include <cassert>

#define gato goto

namespace CardflashUI {
    void UiState::Render(int w, int h, GLFWwindow* window) {
        this->UpdateOrderedSets();

        if (this->screen == Screen::Menu) {
            for (size_t table_i = 0; table_i < this->ordered_sets_table_selection.size(); ++table_i) {
                if (!this->ordered_sets_table_selection[table_i].value) continue;
                this->ordered_sets_table_selection[table_i] = Bool {.value = false};

                UUIDv4::UUID target_uuid = this->ordered_sets[table_i].GetUUID();

                auto setref = this->man.GetSetsRef();
                for (size_t ref_i = 0; ref_i < setref.size(); ++ref_i) {
                    if (setref[ref_i].GetUUID() == target_uuid) {
                        this->tracked_set = this->man.GetSet(ref_i);
                        this->screen = Screen::SetView;
                        gato exitloop;
                    }
                }
            }
        }
        exitloop:

        float top_bar_height = this->TopBar(window);

        switch (this->screen) {
            case Screen::Menu:
                this->MainMenu(w, h, top_bar_height);
                break;
            case Screen::SetView:
                this->SetView(w, h, top_bar_height);
                break;
            case Screen::FlashcardView:
                this->FlashcardView(w, h, top_bar_height);
                break;
            case Screen::Editor:
                this->Editor(w, h, top_bar_height);
                break;
        }
    }

    void UiState::UpdateOrderedSets() {
        if (!this->man.sets_changed)
            return;


        this->ordered_sets = this->man.GetSetsClone();
        this->ordered_sets_table_selection.clear();
        this->ordered_sets_table_selection.resize(this->ordered_sets.size());

        this->man.sets_changed = false;

        std::sort(
            this->ordered_sets.begin(),
            this->ordered_sets.end(),
            [] (const Cardflash::Set& lhs, const Cardflash::Set& rhs) {
                return
                lhs.GetLastOpenedTimestamp()
                    > rhs.GetLastOpenedTimestamp();
            }
        );
    }

    void UiState::SetTrackedSet(Cardflash::Set* set) {
        // Make sure the tracked set doesn't have a value
        assert(!this->tracked_set.has_value());

        this->tracked_set = set;
    }

    void UiState::DropTrackedSet() {
        if (!this->tracked_set.has_value()) return;

        if (this->editor_temp_set.has_value()) this->editor_temp_set = std::nullopt;
        else this->man.DropSetRef(this->tracked_set.value());
        this->tracked_set = std::nullopt;

        // Clear editor buffers
        this->editor_temp_set = std::nullopt;
        this->editor_front_bufs.clear();
        this->editor_back_bufs.clear();
        this->editor_subject.fill(0);
        this->editor_author.fill(0);
        this->editor_title.fill(0);

        // Clear set view stuff
        this->set_view_should_update_statistics = true;
        this->set_view_cards_know = 0;
        this->set_view_cards_learning = 0;

        // Clear flashcard view stuff
        this->flashcard_view_is_shuffled = false;
        this->flashcard_view_is_current_card_revealed = false;
        this->flashcard_view_index_order_index = 0;
        this->flashcard_view_index_order.clear();
    }

    void UiState::SetSetAsTracked(Cardflash::Set set) {
        assert(!this->tracked_set.has_value());
        assert(!this->editor_temp_set.has_value());

        this->editor_temp_set = std::move(set);
        this->tracked_set = &this->editor_temp_set.value();
    }

    bool UiState::SaveTracked() {
        assert(this->tracked_set.has_value());

        if (this->editor_front_bufs.empty()) return false;

        // Make sure the title / author is set
        if (
            this->editor_title[0] == '\0' ||
            this->editor_author[0] == '\0'
        ) return false;

        this->tracked_set.value()->title = std::string(&(this->editor_title[0]));
        this->tracked_set.value()->author = std::string(&(this->editor_author[0]));
        this->tracked_set.value()->subject = std::string(&(this->editor_subject[0]));

        // It's important to run this before clearing and expanding
        // as if one of the buffer is empty (thorws empty string) the
        // original object would be corrupted, which is suboptimal
        std::vector<Cardflash::Card> cards;
        for (size_t i = 0; i < this->editor_front_bufs.size(); ++i) {
            try {
                cards.push_back(
                    Cardflash::Card(
                        std::string(this->editor_front_bufs[i].data()),
                        std::string(this->editor_back_bufs[i].data())
                    )
                );
            } catch (Cardflash::EmptyString& _) {
                return false;
            }
        }

        this->tracked_set.value()->Clear();
        this->tracked_set.value()->Expand(std::move(cards));

        if (this->editor_temp_set.has_value()) {
            this->man.AddSet(std::move(this->editor_temp_set.value()));
            this->editor_temp_set = std::nullopt;

            this->tracked_set = this->man.GetLastSet();
        }

        this->man.Save(this->tracked_set.value());
        this->man.sets_changed = true;

        return true;
    }

    void UiState::GetTrackedSetStatistics() {
        if (!this->set_view_should_update_statistics) return;
        assert(this->tracked_set.has_value());

        this->set_view_cards_know = 0;
        this->set_view_cards_learning = 0;

        assert(this->tracked_set.value()->IsSetFinalized());

        for (const Cardflash::Card& card : this->tracked_set.value()->GetRefCards()) {
            switch (card.learning_status) {
                case Cardflash::CardLearningStatus::Know:
                    ++this->set_view_cards_know;
                    // std::cout << "lstatus: know" << std::endl;
                    break;
                case Cardflash::CardLearningStatus::Learning:
                    ++this->set_view_cards_learning;
                    // std::cout << "lstatus: learn" << std::endl;
                    break;
                default:
                    // std::cout << "lstatus: idfk?" << std::endl;
                    break;
            }
        }

        this->set_view_should_update_statistics = false;
    }

    void UiState::ShuffleIndexOrder() {
        assert(this->tracked_set.has_value());

        this->flashcard_view_index_order_index = 0;
        this->flashcard_view_is_current_card_revealed = false;
        this->flashcard_view_is_shuffled = true;
        std::shuffle(
            flashcard_view_index_order.begin(),
            flashcard_view_index_order.end(),
            this->random_engine
        );
    }

    void UiState::LoadIndexOrder() {
        this->flashcard_view_index_order.clear();
        this->
            flashcard_view_index_order
            .reserve(
                this->
                    tracked_set
                    .value()
                    ->GetRefCards()
                    .size()
            );

        for (size_t i = 0; i < this->tracked_set.value()->GetRefCards().size(); ++i)
            this->flashcard_view_index_order.push_back(i);
    }
}
