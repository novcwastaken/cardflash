#include "SDL3/SDL_iostream.h"
#include "SDL3/SDL_stdinc.h"
#include "backend.hh"

#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_asyncio.h"
#include <SDL3/SDL_main.h>

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>

namespace Cardflash {
    #ifndef CARDFLASH_SET_MANAGER_DEBUG
    static bool is_set_manager_inited = false;
    #endif

    // Class SetManager
    SetManager::SetManager() {
        #ifndef CARDFLASH_SET_MANAGER_DEBUG
        if (is_set_manager_inited) throw(SingletonAlreadyInited(
            "Tried to instantiate SetManager singleton, but the it was already instantiated!"
        ));

        is_set_manager_inited = true;
        #endif // SET_MANAGER_DEBUG
    }

    BufferState SetManager::CheckBufferState() {
        auto state = this->buffer_state.load();

        if (state == BufferState::Ready) {
            this->sets = std::move(this->buffer);
            this->buffer.clear();

            this->sets_changed = true;

            this->buffer_state.store(BufferState::Empty);
        } else if (state == BufferState::Failed) {
            this->buffer.clear();
            this->did_scan_fail = true;

            this->buffer_state.store(BufferState::Empty);
        }

        return state;
    }

    void SetManager::DisableBuffer() {
        auto state = CheckBufferState();

        if (state == BufferState::Working)
            throw(std::runtime_error(
                "Tried to disable buffer while the scan was running"
            ));

        this->buffer_state.store(BufferState::Disabled);
    }

    const std::vector<Set>& SetManager::GetSetsRef() {
        if (!this->AllSetRefsReturned()) throw(NotAllRefsReturned(
            "Tried to get a Sets reference while still borrowing individual Sets!"
        ));

        this->CheckBufferState();
        return this->sets;
    }

    std::vector<Set> SetManager::GetSetsClone() {
        this->CheckBufferState();
        return this->sets;
    }

    Set* SetManager::GetSet(size_t index) {
        if (this->sets.size() == 0) throw (std::out_of_range(
            "GetSet: tried to get a set with a set size of 0."
        ));

        if (index >= this->sets.size()) throw (std::out_of_range(
            "GetSet: index is out of the range of the Set array!"
        ));

        if (this->IsScanning()) throw (ScanRunning());

        this->DisableBuffer();

        Set* ptr = &this->sets[index];
        try {
            (this->refcount.at(ptr))++;
        } catch (const std::out_of_range& _) {
            this->refcount.insert( {ptr, 1} );
        }

        return ptr;
    }

    void SetManager::DropSetRef(Set *set) {
        try {
            // A number describing how many handed out pointers
            // are active for an object
            size_t set_ref_count = this->refcount.at(set);
            if (set_ref_count - 1 == 0) {
                this->refcount.erase(set);

                if (this->refcount.empty()) {
                    this->buffer.clear();
                    this->buffer_state.store(BufferState::Empty);
                }
            } else {
                --this->refcount[set];
            }
        } catch (const std::out_of_range& _) {
            throw(std::runtime_error(
                "Dropped a SetRef that wasn't borrowed!"
            ));
        }
    }

    bool SetManager::AllSetRefsReturned() const {
        return this->refcount.empty();
    }

    bool SetManager::DidScanFail() {
        this->CheckBufferState();
        return this->did_scan_fail;
    }

    bool SetManager::IsScanning() {
        auto result = this->CheckBufferState();
        return result == BufferState::Working;
    }

    bool SetManager::IsScanDisabled() {
        auto result = this->CheckBufferState();
        return result == BufferState::Disabled;
    }

    void SetManager::Save(Set *set) {
        // Open <SET.UUID>.cardflash
        char *userpath = SDL_GetPrefPath(NULL, "cardflash");

        size_t user_path_size = strlen(userpath);

        // Realloc so original path + 36 (uuid string) + 10 (.cardflash) + 1 (nullterm)
        char *filepath = (char*)SDL_realloc(userpath, user_path_size + 36 + 10 + 1);
        if (!filepath) throw (std::runtime_error(
            "Failed to allocate memory for the name of the set to be saved!"
        ));

        std::string uuid_str = set->GetUUID().str();

        memcpy(filepath + user_path_size, &uuid_str[0], 36);
        memcpy(filepath + user_path_size + 36, ".cardflash", 11);

        SDL_IOStream *file = SDL_IOFromFile(filepath, "wb");
        SDL_free(filepath);

        // Run serialize
        std::vector<uint8_t> serialized = set->Serialize();

        // Write data into file
        SDL_WriteIO(file, &serialized.front(), serialized.size());

        // Close file
        SDL_CloseIO(file);

        // Profit :+1:
    }

    void SetManager::AddSet(Set set) {
        if (this->IsScanning()) throw(ScanRunning());
        if (!this->AllSetRefsReturned()) throw(NotAllRefsReturned(
            "Tried to add set while some reference is not returned!"
        ));

        this->sets_changed = true;
        this->sets.push_back(set);
        this->Save(&this->sets[this->sets.size() - 1]);
    }

    //  Scanning
    struct ScanThreadData {
        std::atomic<BufferState>* buffer_state;
        std::vector<Set>* buffer;
    };

    struct DirectoryCallbackUData {
        uint32_t *task_count;
        SDL_AsyncIOQueue *queue;
    };

    SDL_EnumerationResult DirectoryCallback (void *userdata, const char *dirname, const char *fname) {
        DirectoryCallbackUData *data = reinterpret_cast<DirectoryCallbackUData*>(userdata);

        // std::cout << "Directory callback: dirname: " << dirname
        //     << " fname: " << fname << std::endl;

        const char *dot { strrchr(fname, '.') };
        if (dot && strcmp(dot, "cardflash")) {
            char *path { reinterpret_cast<char*>(malloc(strlen(dirname) + strlen(fname) + 1)) };
            if (!path) {
                return SDL_ENUM_FAILURE;
            }
            strcpy(path, dirname);
            strcat(path, fname);

            // This will open / read queue the file to be read
            // Self allocates buffer!
            bool success = SDL_LoadFileAsync(path, data->queue, NULL);
            free(path);

            if (!success) {
                return SDL_ENUM_FAILURE;
            }

            ++*(data->task_count);
        }

        return SDL_ENUM_CONTINUE;
    }

    int SDLCALL ScanWorkerThread(void *ptr) {
        ScanThreadData *data = reinterpret_cast<ScanThreadData*>(ptr);
        try {
            // std::cout << "Scan worker thread spawned!" << std::endl;

            SDL_AsyncIOQueue *ioqueue = SDL_CreateAsyncIOQueue();
            if (!ioqueue) {
                data->buffer_state->store(BufferState::Failed);
                delete data;
                return 0;
            }

            // This returns a path to where it's safe / idiomatic on the
            // given platform to write to. On linux this is something like
            // ~/.local/share/cardflash on windows it's somewhere in appdata.
            //
            // Userpath must be freed!
            char *userpath = SDL_GetPrefPath(NULL, "cardflash");
            if (!userpath) {
                data->buffer_state->store(BufferState::Failed);
                delete data;
                return 0;
            }

            // The count to how much tasks are in the async io queue as
            // that does not track such information (why would it)
            uint32_t task_count = 0;
            DirectoryCallbackUData *udata = new DirectoryCallbackUData {
                .task_count = &task_count,
                .queue = ioqueue
            };

            bool success = SDL_EnumerateDirectory(
                userpath,
                DirectoryCallback,
                reinterpret_cast<void*>(udata)
            );
            SDL_free(userpath);

            // std::cout << "ENUMERATED DIR? " << success
            //     << " Count: " << task_count << std::endl;

            if (!success) {
                data->buffer_state->store(BufferState::Failed);
                delete data;
                return 0;
            }

            // Wait for all the files to be read and deserialize them
            SDL_AsyncIOOutcome outcome;
            while (task_count > 0) {
                // Sleep the thread until something completes
                if (!SDL_WaitAsyncIOResult(ioqueue, &outcome, -1)) {
                    continue;
                }
                --task_count;

                if (outcome.result == SDL_ASYNCIO_FAILURE) {
                    std::cout << "Failure while reading a file!" << std::endl;
                    continue;
                }

                uint8_t *buff = reinterpret_cast<uint8_t*>(outcome.buffer);
                size_t size = static_cast<size_t>(outcome.bytes_transferred);

                std::vector<uint8_t> vec (buff, buff + size);
                SDL_free(buff);

                try {
                    data->buffer->push_back(Set(vec));
                } catch (const DeserializationError& e) {
                    std::cout << "An error occured while deserializing a set: "
                        << e.what() << std::endl;
                } catch (const EmptyString & e) {
                    std::cout << "EmptyString error while deserializing a set: "
                        << e.what() << std::endl;
                }
            }


            data->buffer_state->store(BufferState::Ready);
        } catch (const std::exception& e) {
            std::cerr << "Scan worker thread encountreed an exception: "
                << e.what() << std::endl;

            data->buffer_state->store(BufferState::Failed);
        } catch (...) {
            std::cerr
                << "Scan worker thread encountered an unkown exception!" << std::endl;

            data->buffer_state->store(BufferState::Failed);
        }

        delete data;

        // std::cout << "Scan worker thread dead!" << std::endl;
        return 0;
    }

    void SetManager::Scan() {
        BufferState state = this->buffer_state.load();

        // std::cout << "Scan: state: " << (short)state << std::endl;

        switch (state) {
            case BufferState::Ready:
                this->sets.clear();
                this->sets = std::move(this->buffer);

                this->buffer.clear();
                break;
            case BufferState::Failed:
                this->buffer.clear();
                break;
            case BufferState::Empty:
                this->buffer.clear();
                break;
            case BufferState::Disabled:
                // std::cout << "Called Scan while being disabled!" << std::endl;
            case BufferState::Working:
                return;
        }

        this->buffer_state.store(BufferState::Working);
        this->did_scan_fail = false;

        ScanThreadData *data = new ScanThreadData{
            .buffer_state = &(this->buffer_state),
            .buffer = &(this->buffer)
        };
        SDL_CreateThread(
            ScanWorkerThread,
            "SetManager::Scan worker thread",
            reinterpret_cast<void*>(data)
        );
    }
    //  !Scanning
    // end class SetManager
}
