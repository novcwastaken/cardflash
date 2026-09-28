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
#include <vector>

namespace Cardflash {
    static bool is_set_manager_inited = false;

    // Class SetManager
    SetManager::SetManager() {
        #ifndef SET_MANAGER_DEBUG
        if (is_set_manager_inited) throw(SingletonAlreadyInited(
            "Tried to instantiate SetManager singleton, but the it was already instantiated!"
        ));

        is_set_manager_inited = true;
        #endif // SET_MANAGER_DEBUG
    }

    const std::vector<Set>* SetManager::GetSetsRef() {
        BufferState state = this->buffer_state.load();

        if (state == BufferState::Failed) {
            this->did_scan_fail = true;
            this->buffer.clear();

            state = BufferState::Empty;
        } else if (state == BufferState::Ready) {
            this->sets = std::move(this->buffer);
            this->buffer.clear();

            this->did_scan_fail = false;
        }

        return &(this->sets);
    }

    bool SetManager::DidScanFail() {
        BufferState state = this->buffer_state.load();

        if (state == BufferState::Failed) {
            this->did_scan_fail = true;
            this->buffer.clear();

            state = BufferState::Empty;
        } else if (state == BufferState::Ready) {
            this->sets = std::move(this->buffer);
            this->buffer.clear();

            this->did_scan_fail = false;
        }

        return this->did_scan_fail;
    }

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

        std::cout << "Directory callback: dirname: " << dirname
            << " fname: " << fname << std::endl;

        const char *dot { strrchr(fname, '.') };
        if (dot && strcmp(dot, "cardflash")) {
            char *path { reinterpret_cast<char*>(malloc(strlen(dirname) + strlen(fname) + 1)) };
            if (!path) {
                return SDL_ENUM_FAILURE;
            }
            strcpy(path, dirname);
            strcat(path, fname);

            // This will open / read the file into a buffer that is
            // allocted internally
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
            std::cout << "Scan worker thread spawned!" << std::endl;

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
                Uint64 size = outcome.bytes_transferred;

                std::vector<uint8_t> vec (buff, buff + size);

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

        std::cout << "Scan worker thread dead!" << std::endl;
        return 0;
    }

    void SetManager::Scan() {
        BufferState state = this->buffer_state
            .exchange(BufferState::Working);

        std::cout << "Scan: state: " << (short)state << std::endl;

        switch (state) {
            case BufferState::Ready:
                this->sets.clear();
                this->sets = std::move(this->buffer);

                this->buffer.clear();
                break;
            case BufferState::Failed:
                this->buffer.clear();
                break;
            case BufferState::Working:
                return;
            case BufferState::Empty:
                this->buffer.clear();
                break;
        }

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
    // end class SetManager
}