/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_TRANSFORM_H_
#define _CARTO_MAPNIKVT_TRANSFORM_H_

#include <cmath>

#include <cglib/vec.h>
#include <cglib/mat.h>

namespace carto { namespace mvt {
    class Transform {
    public:
        virtual ~Transform() = default;

        virtual cglib::mat3x3<float> getMatrix() const = 0;

    protected:
        constexpr static float DEG_TO_RAD = 3.14159265f / 180.0f;
    };

    class MatrixTransform : public Transform {
    public:
        explicit MatrixTransform(const cglib::mat3x3<float>& matrix) : _matrix(matrix) { }

        virtual cglib::mat3x3<float> getMatrix() const override {
            return _matrix;
        }

    protected:
        cglib::mat3x3<float> _matrix;
    };

    class TranslateTransform : public Transform {
    public:
        explicit TranslateTransform(const cglib::vec2<float>& pos) : _pos(pos) { }

        const cglib::vec2<float>& getPos() const { return _pos; }

        virtual cglib::mat3x3<float> getMatrix() const override {
            return cglib::translate3_matrix(cglib::expand(_pos, 1.0f));
        }

    protected:
        cglib::vec2<float> _pos;
    };

    class RotateTransform : public Transform {
    public:
        explicit RotateTransform(const cglib::vec2<float>& pos, float angle) : _pos(pos), _angle(angle) { }

        const cglib::vec2<float>& getPos() const { return _pos; }
        float getAngle() const { return _angle; }

        virtual cglib::mat3x3<float> getMatrix() const override {
            return cglib::translate3_matrix(cglib::expand(_pos, 1.0f)) * cglib::rotate3_matrix(cglib::vec3<float>(0, 0, 1), _angle * DEG_TO_RAD) * cglib::translate3_matrix(cglib::expand(-_pos, 1.0f));
        }

    protected:
        cglib::vec2<float> _pos;
        float _angle;
    };

    class ScaleTransform : public Transform {
    public:
        explicit ScaleTransform(const cglib::vec2<float>& scale) : _scale(scale) { }

        const cglib::vec2<float>& getScale() const { return _scale; }

        virtual cglib::mat3x3<float> getMatrix() const override {
            return cglib::scale3_matrix(cglib::expand(_scale, 1.0f));
        }

    protected:
        cglib::vec2<float> _scale;
    };

    class SkewXTransform : public Transform {
    public:
        explicit SkewXTransform(float angle) : _angle(angle) { }

        float getAngle() const { return _angle; }

        virtual cglib::mat3x3<float> getMatrix() const override {
            cglib::mat3x3<float> m = cglib::mat3x3<float>::identity();
            m(0, 1) = std::tan(_angle * DEG_TO_RAD);
            return m;
        }

    protected:
        float _angle;
    };

    class SkewYTransform : public Transform {
    public:
        explicit SkewYTransform(float angle) : _angle(angle) { }

        float getAngle() const { return _angle; }

        virtual cglib::mat3x3<float> getMatrix() const override {
            cglib::mat3x3<float> m = cglib::mat3x3<float>::identity();
            m(1, 0) = std::tan(_angle * DEG_TO_RAD);
            return m;
        }

    protected:
        float _angle;
    };
} }

#endif
