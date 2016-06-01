/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CULLSTATE_H_
#define _CARTO_CULLSTATE_H_

#include "core/MapEnvelope.h"
#include "graphics/ViewState.h"

#include <memory>
#include <vector>

namespace carto {
    class Projection;

    /**
     * View specific state determined by the culling process. This includes 
     * convex envelope of the visible area, view frustum, tiles and camera state.
     */
    class CullState {
    public:
        /** 
         * Constructs a CullState object from an envelope and a viewstate.
         * @param envelope The envelope.
         * @param viewState The view state.
         */
        CullState(const MapEnvelope& envelope, const ViewState& viewState);
        virtual ~CullState();
    
        /**
         * Returns an envelope for the visible area in given projection coordinates.
         * @param proj The projection for the envelope
         * @return The envelope for the visible area in the coordinate system of the given projection.
         */
        MapEnvelope getProjectionEnvelope(const std::shared_ptr<Projection>& proj) const;
        /**
         * Returns an envelope for the visible area.
         * @return The envelope for the visible area in the internal coordiante system.
         */
        const MapEnvelope& getEnvelope() const;
    
        /**
         * Returns a view state.
         * @return The view state.
         */
        const ViewState& getViewState() const;
    
    private:
        MapEnvelope _envelope;
        
        ViewState _viewState;
    };
    
}

#endif
