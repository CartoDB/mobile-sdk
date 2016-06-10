/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOVISLOADER_H_
#define _CARTO_CARTOVISLOADER_H_

#include "core/Variant.h"
#include "components/DirectorPtr.h"

#include <memory>
#include <mutex>
#include <string>

#include <picojson/picojson.h>

namespace carto {
    class Layer;
    class BaseMapView;
    class CartoUIBuilder;
    class CartoMapsService;

    class CartoVisLoader {
    public:
        CartoVisLoader();
        virtual ~CartoVisLoader();

        std::shared_ptr<CartoUIBuilder> getCartoUIBuilder() const;
        void setCartoUIBuilder(const std::shared_ptr<CartoUIBuilder>& cartoUIBuilder);

        // TODO: fix?
        void loadVis(BaseMapView* mapView, const std::string& visURL) const;

    private:
        void configureMapsService(CartoMapsService& mapsService, const picojson::value& options) const;

        int getMinZoom(const picojson::value& options) const;
        int getMaxZoom(const picojson::value& options) const;

        void createLayer(std::vector<std::shared_ptr<Layer> >& layers, const picojson::value& layerConfig) const;

        ThreadSafeDirectorPtr<CartoUIBuilder> _cartoUIBuilder;
    };

}

#endif
