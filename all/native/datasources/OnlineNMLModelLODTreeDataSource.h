/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ONLINENMLMODELLODTREEDATASOURCE_H_
#define _CARTO_ONLINENMLMODELLODTREEDATASOURCE_H_

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "datasources/NMLModelLODTreeDataSource.h"

namespace carto {

    /**
     * Online NML LOD Tree data source. This data source needs special
     * online service and uses custom protocol.
     */
    class OnlineNMLModelLODTreeDataSource : public NMLModelLODTreeDataSource {
    public:
        /**
         * Constructs an OnlineNMLModelLODTreeDataSource object.
         * @param serviceURL The service connection point.
         */
        explicit OnlineNMLModelLODTreeDataSource(const std::string& serviceURL);
        virtual ~OnlineNMLModelLODTreeDataSource();
    
        virtual std::vector<MapTile> loadMapTiles(const std::shared_ptr<CullState>& cullState);
        virtual std::shared_ptr<NMLModelLODTree> loadModelLODTree(const MapTile& mapTile);
        virtual std::shared_ptr<nml::Mesh> loadMesh(long long meshId);
        virtual std::shared_ptr<nml::Texture> loadTexture(long long textureId, int level);
    
    private:
        class DataInputStream {
        public:
            DataInputStream(const std::vector<unsigned char>& data);
    
            unsigned char readByte();
            int readInt();
            long long readLongLong();
            float readFloat();
            double readDouble();
            std::string readString();
            std::vector<unsigned char> readBytes(std::size_t count);
        
        private:
            const std::vector<unsigned char>& _data;
            std::size_t _offset;
        };
    
        std::string _serviceURL;
    };
    
}

#endif

#endif
