/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_FETCHINGTILETASKS_H_
#define _CARTO_FETCHINGTILETASKS_H_

#include <unordered_map>

namespace carto {

    template <typename Task>
    class FetchingTileTasks {
    public:
        FetchingTileTasks() : _fetchingTiles(), _mutex() { }
        
        void add(long long tileId, const std::shared_ptr<Task>& task) {
            std::lock_guard<std::mutex> lock(_mutex);
            _fetchingTiles[tileId] = task;
        }
        
        bool exists(long long tileId) {
            std::lock_guard<std::mutex> lock(_mutex);
            return _fetchingTiles.find(tileId) != _fetchingTiles.end();
        }
        
        void remove(long long tileId) {
            std::lock_guard<std::mutex> lock(_mutex);
            _fetchingTiles.erase(tileId);
        }
        
        std::vector<std::shared_ptr<Task> > getTasks() const {
            std::lock_guard<std::mutex> lock(_mutex);
            std::vector<std::shared_ptr<Task> > tasks;
            for (const auto& pair : _fetchingTiles) {
                tasks.push_back(pair.second);
            }
            return tasks;
        }
        
        int getPreloadingCount() const {
            std::lock_guard<std::mutex> lock(_mutex);
            int count = 0;
            for (const auto& pair : _fetchingTiles) {
                if (pair.second->isPreloading()) {
                    count++;
                }
            }
            return count;
        }
        
        int getVisibleCount() const {
            std::lock_guard<std::mutex> lock(_mutex);
            int count = 0;
            for (const auto& pair : _fetchingTiles) {
                if (!pair.second->isPreloading()) {
                    count++;
                }
            }
            return count;
        }

    private:
        std::unordered_map<long long, std::shared_ptr<Task> > _fetchingTiles;
        mutable std::mutex _mutex;
    };
    
}

#endif
