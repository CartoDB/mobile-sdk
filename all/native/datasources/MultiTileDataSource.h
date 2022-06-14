/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */


#ifndef _CARTO_LOCALPACKAGEMANAGERTILEDATASOURCE_H_
#define _CARTO_LOCALPACKAGEMANAGERTILEDATASOURCE_H_

#include "datasources/TileDataSource.h"
#include "packagemanager/PackageTileMask.h"

#include <memory>
#include <mutex>
#include <vector>
#include <optional>

namespace carto {

    /**
     * A tile data source that handles multiple data sources.
     */
    class MultiTileDataSource : public TileDataSource {
    public:
        /**
         * Constructs a PackageManagerTileDataSource object.
         * @param packageManager The package manager that is used to retrieve requested tiles.
         */
        explicit MultiTileDataSource();
        MultiTileDataSource(int maxOpenedPackages);
        virtual ~MultiTileDataSource();

        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);

        
        /**
         * Adds a new  data source to the  data source stack. The new  data source will be the last (and topmost)  data source.
         * @param datasource The data source to be added.
         */
        void add(const std::shared_ptr<TileDataSource>& datasource);

        /**
         * Adds a new  data source to the  data source stack. The new  data source will be the last (and topmost)  data source.
         * @param datasource The data source to be added.
         */
        void add(const std::shared_ptr<TileDataSource>& datasource, const std::string& tileMask);

        /**
         * Removes a data source from the sources stack.
         * @param datasource The data source to be removed.
         * @return True if the  data source was removed. False otherwise ( data source was not found).
         */
        bool remove(const std::shared_ptr<TileDataSource>& datasource);

    protected:
         enum ChangeType {
            PACKAGES_ADDED,
            PACKAGES_DELETED
        };

        mutable std::optional<int> _maxOpenedPackages;

        mutable std::vector<std::pair<std::shared_ptr<PackageTileMask>, std::shared_ptr<TileDataSource> > > _dataSources;
        mutable std::vector<std::pair<std::shared_ptr<PackageTileMask>, std::shared_ptr<TileDataSource> > > _cachedOpenDataSources;

        mutable std::mutex _mutex;

        void onPackagesChanged(ChangeType changeType);
    };

}

#endif
