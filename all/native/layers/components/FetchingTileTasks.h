/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_FETCHINGTILETASKS_H_
#define _CARTO_FETCHINGTILETASKS_H_

#include <memory>
#include <vector>
#include <unordered_map>

namespace carto {

    template <typename Task>
    class FetchingTileTasks {
    public:
        FetchingTileTasks() : _fetchingTiles(), _mutex() { }
        
        std::vector<std::shared_ptr<Task> > get(long long tileId) const {
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = _fetchingTiles.find(tileId);
            return it != _fetchingTiles.end() ? it->second : std::vector<std::shared_ptr<Task> >();
        }
        
        void insert(long long tileId, const std::shared_ptr<Task>& task) {
            std::lock_guard<std::mutex> lock(_mutex);
            _fetchingTiles[tileId].push_back(task);
        }
        
        void remove(long long tileId, const std::shared_ptr<Task>& task) {
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = _fetchingTiles.find(tileId);
            if (it == _fetchingTiles.end()) {
                return;
            }
            std::vector<std::shared_ptr<Task> >& tasks = it->second;
            auto it2 = std::find(tasks.begin(), tasks.end(), task);
            if (it2 == tasks.end()) {
                return;
            }
            tasks.erase(it2);
            if (tasks.empty()) {
                _fetchingTiles.erase(it);
            }
        }
        
        std::vector<std::shared_ptr<Task> > getAll() const {
            std::lock_guard<std::mutex> lock(_mutex);
            std::vector<std::shared_ptr<Task> > tasks;
            for (auto it = _fetchingTiles.begin(); it != _fetchingTiles.end(); it++) {
                tasks.insert(tasks.end(), it->second.begin(), it->second.end());
            }
            return tasks;
        }
        
        int getPreloadingCount() const {
            std::lock_guard<std::mutex> lock(_mutex);
            int count = 0;
            for (auto it = _fetchingTiles.begin(); it != _fetchingTiles.end(); it++) {
                for (const std::shared_ptr<Task>& task : it->second) {
                    if (task->isPreloadingTile()) {
                        count++;
                    }
                }
            }
            return count;
        }
        
        int getVisibleCount() const {
            std::lock_guard<std::mutex> lock(_mutex);
            int count = 0;
            for (auto it = _fetchingTiles.begin(); it != _fetchingTiles.end(); it++) {
                for (const std::shared_ptr<Task>& task : it->second) {
                    if (!task->isPreloadingTile()) {
                        count++;
                    }
                }
            }
            return count;
        }

    private:
        std::unordered_map<long long, std::vector<std::shared_ptr<Task> > > _fetchingTiles;
        mutable std::mutex _mutex;
    };
    
}

#endif
