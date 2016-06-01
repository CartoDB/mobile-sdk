/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPPOS_H_
#define _CARTO_MAPPOS_H_

#include <string>
#include <utility>
#include <functional>

namespace carto {
    class MapVec;
    
    /**
     * A double precision map position defined using three coordinates. X and y coordinates denote positions on the map,
     * while z coordinate is height from the ground plane. Actual units for x, y and z depend on map projection.
     * For example, in EPSG:4326 x is used for latitude, y for longitude and z for height in meters.
     */
    class MapPos {
    public:
        /**
         * Constructs a MapPos object. All coordinates will be 0.
         */
        MapPos();
        /**
         * Constructs a MapPos object from 2 coordinates. The z coordinate will be 0.
         * @param x The x coordinate.
         * @param y The y coordinate.
         */
        MapPos(double x, double y);
        /**
         * Constructs a MapPos object from 3 coordinates.
         * @param x The x coordinate.
         * @param y The y coordinate.
         * @param z The z coordinate.
         */
        MapPos(double x, double y, double z);

        /**
         * Returns the x coordinate of this map position.
         * @return The x coordinate of this map position.
         */
        double getX() const;
        /**
         * Sets the x coordinate of this map position.
         * @param x The new x coordinate of this map position.
         */
        void setX(double x);
        /**
         * Returns the y coordinate of this map position.
         * @return The y coordinate of this map position.
         */
        double getY() const;
        /**
         * Sets the y coordinate of this map position.
         * @param y The new y coordinate of this map position.
         */
        void setY(double y);
        /**
         * Returns the z coordinate of this map position.
         * @return The z coordinate of this map position.
         */
        double getZ() const;
        /**
         * Sets the z coordinate of this map position.
         * @param z The new z coordinate of this map position.
         */
        void setZ(double z);
        
        /**
         * Returns the n-th coordinate of this map position.
         * @param n The index of the requested coordinate.
         * @return The n-th coordinate of this map position.
         */
        double operator[](std::size_t n) const;
        /**
         * Returns a modifiable n-th coordinate of this map position.
         * @param n The index of the requested coordinate.
         * @return The modifiable n-th coordinate of this map position.
         */
        double& operator[](std::size_t n);
        
        /**
         * Sets the x and y coordinates of this map position.
         * @param x The new x coordinate of this map position.
         * @param y The new y coordinate of this map position.
         */
        void setCoords(double x, double y);
        /**
         * Sets the x, y and z coordinates of this map position.
         * @param x The new x coordinate of this map position.
         * @param y The new y coordinate of this map position.
         * @param z The new z coordinate of this map position.
         */
        void setCoords(double x, double y, double z);
        
        /**
         * Adds map vector to this map position.
         * @param v The map vector to be added.
         * @return This map position after the addition.
         */
        MapPos& operator+=(const MapVec& v);
        /**
         * Subtracts map vector from this map position.
         * @param v The map vector to be subtracted.
         * @return This map position after the subtraction.
         */
        MapPos& operator-=(const MapVec& v);
    
        /**
         * Creates a new map position by adding a map vector to this map position.
         * @param v The map vector to be added.
         * @return The new map position.
         */
        MapPos operator+(const MapVec& v) const;
        /**
         * Creates a new map position by subtracting a map vector from this map position.
         * @param v The map vector to be subtracted.
         * @return The new map position.
         */
        MapPos operator-(const MapVec& v) const;
    
        /**
         * Creates a new map vector by subtracting a map position from this map position.
         * @param p The map position to be subtracted.
         * @return The new map vector.
         */
        MapVec operator-(const MapPos& p) const;
    
        /**
         * Checks for equality between this and another map position.
         * @param p The other map position.
         * @return True if equal.
         */
        bool operator==(const MapPos& p) const;
        /**
         * Checks for inequality between this and another map position.
         * @param p The other map position.
         * @return True if not equal.
         */
        bool operator!=(const MapPos& p) const;
    
        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;

        /**
         * Creates a string representation of this map position, useful for logging.
         * @return The string representation of this map position.
         */
        std::string toString() const;
        
    private:
        double _x;
        double _y;
        double _z;
    };
    
}

namespace std {

    template <>
    struct hash<carto::MapPos> {
        size_t operator() (const carto::MapPos& pos) const {
            return static_cast<size_t>(pos.hash());
        }
    };

}

#endif
