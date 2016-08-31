/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PACKAGEINFO_H_
#define _CARTO_PACKAGEINFO_H_

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace carto {
    class PackageMetaInfo;
    class PackageTileMask;
    
    namespace PackageType {
        /**
         * Package types.
         */
        enum PackageType {
            /**
             * Map package.
             */
            PACKAGE_TYPE_MAP = 0,
            /**
             * Routing package.
             */
            PACKAGE_TYPE_ROUTING = 1
        };
    }

    /**
     * Information about map package. This includes id, type, version, name, description and size.
     */
    class PackageInfo {
    public:
        /**
         * Constructs a new package info instance.
         * @param packageId The unique id of the package
         * @param packageType The type of the package
         * @param version The increasing version number of the package
         * @param size Size of the package in bytes
         * @param serverURL Location of the package
         * @param tileMask The tile mask of the package
         * @param metaInfo Package meta info
         */
        PackageInfo(const std::string& packageId, PackageType::PackageType packageType, int version, std::uint64_t size, const std::string& serverURL, const std::shared_ptr<PackageTileMask>& tileMask, const std::shared_ptr<PackageMetaInfo>& metaInfo) : _packageId(packageId), _packageType(packageType), _version(version), _size(size), _serverURL(serverURL), _tileMask(tileMask), _metaInfo(metaInfo) { }

        /**
         * Returns the internal package id. This should not be displayed to the user.
         * @return The unique package id
         */
        const std::string& getPackageId() const {
            return _packageId;
        }

        /**
         * Returns the package type.
         * @return The package type (map, routing, etc)
         */
        PackageType::PackageType getPackageType() const {
            return _packageType;
        }
        
        /**
         * Returns the package version. This should not be displayed to the user.
         * @return The increasing package version number
         */
        int getVersion() const {
            return _version;
        }

        /**
         * Returns the default name (short description) of the package. It is better to use getNames method instead, as each package may contain multiple names.
         * The name returned is generic name or if that is not available, then English name.
         * @return The list of names for the package in the specified language (if not available, generic/English names are returned)
         */
        std::string getName() const;

        /**
         * Returns the names (short description) of the package. This can be displayed to the user. Each package may have multiple names, if the package has multiple classifications.
         * Note: this information is dervied from package meta info, "name_XXX" fields.
         * @param lang The language for the name
         * @return The list of names for the package in the specified language (if not available, generic/English names are returned)
         */
        std::vector<std::string> getNames(const std::string& lang) const;
        
        /**
         * Returns the size of the package in bytes. This can be displayed to the user.
         * @return The size of the package in bytes
         */
        std::uint64_t getSize() const {
            return _size;
        }

        /**
         * Returns the location of the package. This should not be displayed to the user.
         * @return The location of the package
         */
        const std::string& getServerURL() const {
            return _serverURL;
        }

        /**
         * Returns the encoded tile mask of the package. This is available for map packages but not for routing packages.
         * This should not be displayed to the user.
         * @return The tile mask of the package
         */
        const std::shared_ptr<PackageTileMask>& getTileMask() const {
            return _tileMask;
        }

        /**
         * Returns package meta info. If package contains no meta info, null is returned.
         * @return The package meta info.
         */
        const std::shared_ptr<PackageMetaInfo>& getMetaInfo() const {
            return _metaInfo;
        }

    private:
        std::string _packageId;
        PackageType::PackageType _packageType;
        int _version;
        std::uint64_t _size;
        std::string _serverURL;
        std::shared_ptr<PackageTileMask> _tileMask;
        std::shared_ptr<PackageMetaInfo> _metaInfo;
    };

}

#endif

#endif
