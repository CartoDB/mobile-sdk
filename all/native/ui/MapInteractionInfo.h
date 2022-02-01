/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPINTERACTIONINFO_H_
#define _CARTO_MAPINTERACTIONINFO_H_

namespace carto {

    /**
     * A container class that provides information about a map interaction performed from UI.
     */
    class MapInteractionInfo {
    public:
        /**
         * Constructs a MapInteractionInfo object from action attributes.
         * @param panAction The pan action flag.
         * @param zoomAction The zoom action flag.
         * @param rotateAction The rotate action flag.
         * @param tiltAction The tilt action flag.
         * @param animationStarted The animation started flag.
         */
        MapInteractionInfo(bool panAction, bool zoomAction, bool rotateAction, bool tiltAction, bool animationStarted = false);
        virtual ~MapInteractionInfo();

        /**
         * Returns true if the interaction included a map pan action.
         * @return True if the interaction included a map pan action.
         */
        bool isPanAction() const;
        /**
         * Returns true if the interaction included a zoom action.
         * @return True if the interaction included a zoom action.
         */
        bool isZoomAction() const;
        /**
         * Returns true if the interaction included a rotate action.
         * @return True if the interaction included a rotate action.
         */
        bool isRotateAction() const;
        /**
         * Returns true if the interaction included a tilt action.
         * @return True if the interaction included a tilt action.
         */
        bool isTiltAction() const;
        /**
         * Returns true if the interaction has started an animation.
         * @return True if the interaction has started an animation.
         */
        bool isAnimationStarted() const;
    
    private:
        bool _panAction;
        bool _zoomAction;
        bool _rotateAction;
        bool _tiltAction;
        bool _animationStarted;
    };
    
}

#endif
