/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */


#ifndef _CARTO_PACKAGEMANAGERTILEDATASOURCE_H_
#define _CARTO_PACKAGEMANAGERTILEDATASOURCE_H_

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "datasources/TileDataSource.h"
#include "packagemanager/PackageManager.h"

namespace carto {

    /**
     * A tile data source that loads tiles from package manager.
     */
    class PackageManagerTileDataSource : public TileDataSource {
    public:
        /**
         * Constructs a PackageManagerTileDataSource object.
         * @param packageManager The package manager that is used to retrieve requested tiles.
         */
        explicit PackageManagerTileDataSource(const std::shared_ptr<PackageManager>& packageManager);
        virtual ~PackageManagerTileDataSource();

        /**
         * Returns the package manager instance used by the data source.
         * @return The package manager instance used by the data source.
         */
        std::shared_ptr<PackageManager> getPackageManager() const;

        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);

    protected:
        class PackageManagerListener : public PackageManager::OnChangeListener {
        public:
            PackageManagerListener(PackageManagerTileDataSource& dataSource);
        		
            virtual void onPackagesChanged();
        		
        private:
            PackageManagerTileDataSource& _dataSource;
        };

        std::shared_ptr<PackageManager> _packageManager;

    private:
        std::shared_ptr<PackageManagerListener> _packageManagerListener;
    };

}

#endif

#endif
