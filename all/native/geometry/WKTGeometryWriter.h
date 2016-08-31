/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_WKTGEOMETRYWRITER_H_
#define _CARTO_WKTGEOMETRYWRITER_H_

#ifdef _CARTO_WKBT_SUPPORT

#include "core/MapPos.h"

#include <memory>
#include <mutex>

namespace carto {
    class Geometry;

    /**
     * Generates Well Known Text (WKT) representation of the geometry.
     * Supports both 2D and 3D coordinate serialization.
     */
    class WKTGeometryWriter {
    public:
        /**
         * Constructs a new WKTGeometryWriter objects with default settings.
         */
        WKTGeometryWriter();

        /**
         * Returns the state of Z coordinate serialization.
         * @return True if Z coordinate is stored in WKT output, false otherwise. The default is false.
         */
        bool getZ() const;
        /**
         * Sets the state of Z coordinate serialization.
         * @param z True when Z coordinate should be serialized, false otherwise.
         */
        void setZ(bool z);

        /**
         * Creates a WKT string corresponding to the specified geometry.
         * @param geometry The geometry to write.
         * @return The corresponding WKT string.
         * @throws std::runtime_error If writing fails.
         */
        std::string writeGeometry(const std::shared_ptr<Geometry>& geometry) const;

    private:
        bool _z;

        mutable std::mutex _mutex;
    };

}

#endif

#endif
