/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_POOLALLOCATOR_H_
#define _CARTO_VT_POOLALLOCATOR_H_

#include <cstdlib>
#include <algorithm>

namespace carto { namespace vt {
    class PoolAllocator {
    public:
        PoolAllocator() : _first(nullptr) {
            void* buffer = std::malloc(DEFAULT_BLOCK_SIZE);
            try {
                _first = new Block(buffer, (buffer != NULL ? DEFAULT_BLOCK_SIZE : 0), nullptr);
            }
            catch (...) {
                std::free(buffer);
                throw;
            }
        }
        
        PoolAllocator(const PoolAllocator&) = delete;

        ~PoolAllocator() {
            while (_first) {
                Block* next = _first->next;
                std::free(_first->buffer);
                delete _first;
                _first = next;
            }
        }

        void reset() {
            for (Block* current = _first->next; current != nullptr;) {
                Block* next = current->next;
                std::free(current->buffer);
                delete current;
                current = next;
            }
            _first->allocated = 0;
            _first->next = nullptr;
        }

        void* allocate(std::size_t size) {
            size = (size + 7) & ~7; // keep 8-byte alignment
            if (_first->allocated + size > _first->size) {
                if (!reallocate(size)) {
                    return NULL;
                }
            }
            void* ptr = reinterpret_cast<unsigned char*>(_first->buffer) + _first->allocated;
            _first->allocated += size;
            return ptr;
        }

        PoolAllocator& operator = (const PoolAllocator&) = delete;

    private:
        const int DEFAULT_BLOCK_SIZE = 65536;

        struct Block {
            void* const buffer;
            std::size_t const size;
            std::size_t allocated;
            Block* next;

            Block(void* buffer, std::size_t size, Block* next) : buffer(buffer), size(size), allocated(0), next(next) { }
        };

        bool reallocate(std::size_t size) {
            std::size_t blockSize = std::max(size, _first->size * 2); // increase block size two-fold each time reallocation is required
            void* buffer = std::malloc(blockSize);
            if (buffer == NULL) {
                return false;
            }
            try {
                _first = new Block(buffer, blockSize, _first);
                return true;
            }
            catch (...) {
                std::free(buffer);
                return false;
            }
        }

        Block* _first;
    };
} }

#endif
