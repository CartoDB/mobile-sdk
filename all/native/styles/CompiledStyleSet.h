/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_COMPILEDSTYLESET_H
#define _CARTO_COMPILEDSTYLESET_H

#include <memory>
#include <string>
#include <vector>

namespace carto {
    class AssetPackage;

    /**
     * Compiled style set definition for vector tiles. Compiled style sets should contains either Mapnik XML stylesheets
     * with all required assets or CartoCSS JSON-based project files.
     */
    class CompiledStyleSet {
    public:
        /**
         * Constructs a style set from the given asset package.
         * If the asset package contains multiple styles (XML or JSON files), the first one (based on lexicographical order) from the root folder is chosen as the current style.
         * @param assetPackage The asset package containing compiled style set data and style definition.
         */
        explicit CompiledStyleSet(const std::shared_ptr<AssetPackage>& assetPackage);
        /**
         * Constructs a style from the given asset package and explicit style name.
         * @param assetPackage The asset package containing compiled style set data and style definition.
         * @param styleName The name of the style to use.
         */
        CompiledStyleSet(const std::shared_ptr<AssetPackage>& assetPackage, const std::string& styleName);
        virtual ~CompiledStyleSet();

        /**
         * Returns the current style name.
         * @return The current style name.
         */
        const std::string& getStyleName() const;

        /**
         * Returns the asset name defining the current style name.
         * @return The CartoCSS string used for the style.
         * @return The asset name defining the current style name.
         */
        const std::string& getStyleAssetName() const;

        /**
         * Returns the style asset package.
         * @return The style asset package.
         */
        const std::shared_ptr<AssetPackage>& getAssetPackage() const;

    private:
        std::string _styleName;
        std::string _styleAssetName;
        std::shared_ptr<AssetPackage> _assetPackage;
    };

}

#endif
