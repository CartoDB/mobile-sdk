/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPVEC_H_
#define _CARTO_MAPVEC_H_

#include <string>

namespace carto {

    /**
     * A double precision map vector defined by 3 coordinates.
     */
    class MapVec {
    public:
        /**
         * Constructs a MapVec object. All coordinates will be 0.
         */
        MapVec();
        /**
         * Constructs a MapVec object from 2 coordinates. The z coordinate will be 0.
         * @param x The x coordinate.
         * @param y The y coordinate.
         */
        MapVec(double x, double y);
        /**
         * Constructs a MapVec object from 3 coordinates.
         * @param x The x coordinate.
         * @param y The y coordinate.
         * @param z The z coordinate.
         */
        MapVec(double x, double y, double z);
    
        /**
         * Returns the x coordinate of this map vector.
         * @return The x coordinate of this map vector.
         */
        double getX() const;
        /**
         * Sets the x coordinate of this map vector.
         * @param x The new x coordinate of this map vector.
         */
        void setX(double x);
        /**
         * Returns the y coordinate of this map vector.
         * @return The y coordinate of this map vector.
         */
        double getY() const;
        /**
         * Sets the y coordinate of this map vector.
         * @param y The new y coordinate of this map vector.
         */
        void setY(double y);
        /**
         * Returns the z coordinate of this map vector.
         * @return The z coordinate of this map vector.
         */
        double getZ() const;
        /**
         * Sets the z coordinate of this map vector.
         * @param z The new z coordinate of this map vector.
         */
        void setZ(double z);
        
        /**
         * Returns the n-th coordinate of this map vector.
         * @param n The index of the requested coordinate.
         * @return The n-th coordinate of this map vector.
         */
        double operator[](std::size_t n) const;
        /**
         * Returns a modifiable n-th coordinate of this map vector.
         * @param n The index of the requested coordinate.
         * @return The modifiable n-th coordinate of this map vector.
         */
        double& operator[](std::size_t n);
        
        /**
         * Sets the x and y coordinates of this map vector.
         * @param x The new x coordinate of this map vector.
         * @param y The new y coordinate of this map vector.
         */
        void setCoords(double x, double y);
        /**
         * Sets the x, y and z coordinates of this map vector.
         * @param x The new x coordinate of this map vector.
         * @param y The new y coordinate of this map vector.
         * @param z The new z coordinate of this map vector.
         */
        void setCoords(double x, double y, double z);
        
        /**
         * Adds another map vector to this map vector.
         * @param v The map vector to be added.
         * @return This map vector after the addition.
         */
        MapVec& operator+=(const MapVec& v);
        /**
         * Subtracts another map vector from this map vector.
         * @param v The map vector to be added.
         * @return This map vector after the subtraction.
         */
        MapVec& operator-=(const MapVec& v);
        /**
         * Multiplies this map vector by a multiplier.
         * @param multiplier The multiplier.
         * @return This map vector after the multiplication.
         */
        MapVec& operator*=(double multiplier);
        /**
         * Divides this map vector by a divider.
         * @param divider The divider.
         * @return This map vector after the division.
         */
        MapVec& operator/=(double divider);
    
        /**
         * Creates a new map vector by adding a map vector to this map vector.
         * @param v The map vector to be added.
         * @return The new map vector.
         */
        MapVec operator+(const MapVec& v) const;
        /**
         * Creates a new map vector by subtracting a map vector from this map vector.
         * @param v The map vector to be subtracted.
         * @return The new map vector.
         */
        MapVec operator-(const MapVec& v) const;
        /**
         * Creates a new map vector by multiplying this map vector with a multiplier.
         * @param multiplier The multiplier.
         * @return The new map vector.
         */
        MapVec operator*(double multiplier) const;
        /**
         * Creates a new map vector by dividing this map vector with a divider.
         * @param divider The divider.
         * @return The new map vector.
         */
        MapVec operator/(double divider) const;
    
        /**
         * Checks for equality between this and another map vector.
         * @param v The other map vector.
         * @return True if equal.
         */
        bool operator==(const MapVec& v) const;
        /**
         * Checks for inequality between this and another map vector.
         * @param v The other map vector.
         * @return True if not equal.
         */
        bool operator!=(const MapVec& v) const;
    
        /**
         * Calculates the length of this map vector.
         * @return The length of this map vector.
         */
        double length() const;
        /**
         * Calculates the square length of this map vector.
         * @return The square length of this map vector.
         */
        double lengthSqr() const;
    
        /**
         * Normalizes this map vector.
         * @return This map vector after the normalization.
         */
        MapVec& normalize();
        /**
         * Creates a new map vector by normalizing this map vector.
         * @return The new normalized map vector.
         */
        MapVec getNormalized() const;
    
        /**
         * Rotates this map vector around it's back end and around z axis.
         * @param sin The sine value of the rotation angle.
         * @param cos The cosine value of the rotation angle.
         * @return This map vector after the rotation.
         */
        MapVec& rotate2D(double sin, double cos);
        /**
         * Creates a new map vector by rotating this map vector around it's back end and around z axis.
         * @param sin The sine value of the rotation angle.
         * @param cos The cosine value of the rotation angle.
         * @return The new rotated map vector.
         */
        MapVec getRotated2D(double sin, double cos) const;
    
        /**
         * Calculates the 2D cross product between this and another map vector. Defined as x1 * y2 - y1 * x2.
         * @param v The other map vector.
         * @return The 2D cross product between this and another map vector.
         */
        double crossProduct2D(const MapVec& v) const;
        /**
         * Creates a new map vector that's perpendicular to the plane defined by this and another map vector.
         * @param v The other map vector.
         * @return The new perpendicular map vector.
         */
        MapVec crossProduct3D(const MapVec& v) const;
        
        /**
         * Calculates the dot product between this and another map vector.
         * @param v The other map vector.
         * @return The dot product between this and another map vector.
         */
        double dotProduct(const MapVec& v) const;
    
        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;

        /**
         * Creates a string representation of this map vector, useful for logging.
         * @return The string representation of this map vector.
         */
        std::string toString() const;
        
    private:
        double _x;
        double _y;
        double _z;
    };
    
}

#endif
