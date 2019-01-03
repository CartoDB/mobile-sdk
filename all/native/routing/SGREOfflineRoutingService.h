/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SGREOFFLINEROUTINGSERVICE_H_
#define _CARTO_SGREOFFLINEROUTINGSERVICE_H_

#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "core/Variant.h"
#include "routing/RoutingService.h"

#include <memory>
#include <mutex>
#include <string>

#include <picojson/picojson.h>

namespace carto {
    namespace sgre {
        class RuleList;
        class RouteFinder;
    }

    class FeatureCollection;
    class Projection;

    /**
     * An offline routing service that uses SGRE routing engine.
     * Note: this class is experimental and may change or even be removed in future SDK versions.
     */
    class SGREOfflineRoutingService : public RoutingService {
    public:
        /**
         * Constructs a new SGREOfflineRoutingService instance from a given GeoJSON and rule list.
         * @param geoJSON The GeoJSON variant specifying features used for the routing graph.
         * @param config A configuration specifying various routing metrics and rules.
         * @throws std::runtime_error If an error occured during rule list parsing.
         */
        SGREOfflineRoutingService(const Variant& geoJSON, const Variant& config);
        /**
         * Constructs a new SGREOfflineRoutingService instance from a given feature collection and rule list.
         * @param projection Projection for the features in featureCollection. Can be null if the coordinates are based on WGS84.
         * @param featureCollection The feature collection used for the routing graph.
         * @param config A configuration specifying various routing metrics and rules.
         * @throws std::runtime_error If an error occured during rule list parsing.
         */
        SGREOfflineRoutingService(const std::shared_ptr<Projection>& projection, const std::shared_ptr<FeatureCollection>& featureCollection, const Variant& config);
        virtual ~SGREOfflineRoutingService();

        /**
         * Returns the current routing profile.
         * @return The current routing profile. This can be defined in the rule list.
         */
        std::string getProfile() const;
        /**
         * Sets the current routing profile.
         * @param profile The new profile. This can be defined in the rule list.
         */
        void setProfile(const std::string& profile);

        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const;

    protected:
        void initialize(const Variant& config);

        static float CalculateTurnAngle(const std::vector<MapPos>& epsg3857Points, int pointIndex);
        
        static float CalculateAzimuth(const std::vector<MapPos>& epsg3857Points, int pointIndex);
        
        static bool TranslateInstructionCode(int instructionCode, RoutingAction::RoutingAction& action);

        picojson::value _featureData;
        std::string _profile;

        double _tesselationDistance;
        bool _pathStraightening;
        std::shared_ptr<sgre::RuleList> _ruleList;

        mutable std::shared_ptr<sgre::RouteFinder> _cachedRouteFinder;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
