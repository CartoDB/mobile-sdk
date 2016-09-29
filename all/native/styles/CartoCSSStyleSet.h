/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSSSTYLESET_H_
#define _CARTO_CARTOCSSSTYLESET_H_

#include <memory>
#include <string>
#include <vector>

namespace carto {
    class AssetPackage;

    /**
     * CartoCSS style definition for vector tiles. Style sets contain optional asset package,
     * fonts and other resources can be loaded from the asset package.
     */
    class CartoCSSStyleSet {
    public:
        /**
         * Constructs a style from CartoCSS string.
         * @param cartoCSS The CartoCSS string defining point styling.
         */
        explicit CartoCSSStyleSet(const std::string& cartoCSS);
        /**
         * Constructs a style from CartoCSS string and asset data.
         * @param cartoCSS The CartoCSS string defining point styling.
         * @param assetPackage The optional asset package for the style. Can be null if CartoCSS does not use any local assets.
         */
        CartoCSSStyleSet(const std::string& cartoCSS, const std::shared_ptr<AssetPackage>& assetPackage);
        virtual ~CartoCSSStyleSet();

        /**
         * Returns the CartoCSS string used for the style.
         * @return The CartoCSS string used for the style.
         */
        const std::string& getCartoCSS() const;

        /**
         * Returns the style asset package.
         * @return The style asset package.
         */
        const std::shared_ptr<AssetPackage>& getAssetPackage() const;

    private:
        std::string _cartoCSS;
        std::shared_ptr<AssetPackage> _assetPackage;
    };

}

#endif
