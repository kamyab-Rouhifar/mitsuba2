#pragma once

#include <mitsuba/core/ray.h>

NAMESPACE_BEGIN(mitsuba)

/// Generic n-dimensional bounding sphere data structure
template <typename Point_> struct BoundingSphere {
    static constexpr size_t Size = Point_::Size;

    using Point  = Point_;
    using Value = value_t<Point>;
    using Vector = typename Point::Vector;
    using Mask = mask_t<Value>;

    Point center;
    Value radius;

    /// Construct bounding sphere(s) at the origin having radius zero
    BoundingSphere() : center(0.f), radius(0.f) { }

    /// Create bounding sphere(s) from given center point(s) with given size(s)
    BoundingSphere(const Point &center, const Value &radius)
    : center(center), radius(radius) { }

    /// Equality test against another bounding sphere
    bool operator==(const BoundingSphere &bsphere) const {
        return all_nested(all(eq(center, bsphere.center)) & eq(radius, bsphere.radius));
    }

    /// Inequality test against another bounding sphere
    bool operator!=(const BoundingSphere &bsphere) const {
        return any_nested(any(neq(center, bsphere.center)) | neq(radius, bsphere.radius));
    }

    /// Return whether this bounding sphere has a radius of zero or less.
    Mask empty() const {
        return radius <= 0.f;
    }

    /// Expand the bounding sphere radius to contain another point.
    void expand(const Point &p) {
        radius = enoki::max(radius, norm(p - center));
    }

    /**
     * \brief Check whether a point lies \a on or \a inside the bounding sphere
     *
     * \param p The point to be tested
     *
     * \tparam Strict Set this parameter to \c true if the bounding
     *                sphere boundary should be excluded in the test
     *
     * \remark In the Python bindings, the 'Strict' argument is a normal
     *         function parameter with default value \c False.
     */
    template <bool Strict = false>
    Mask contains(const Point &p) const {
        if (Strict)
            return squared_norm(p - center) < radius*radius;
        else
            return squared_norm(p - center) <= radius*radius;
    }

    /// Check if a ray intersects a bounding box
    template <typename Point>
    MTS_INLINE auto ray_intersect(const Ray<Point> &ray) const {
        auto o = ray.o - center;

        return math::solve_quadratic(
            squared_norm(ray.d),
            2.f * dot(o, ray.d),
            squared_norm(o) - radius * radius
        );
    }

    ENOKI_ALIGNED_OPERATOR_NEW()
};

/// Print a string representation of the bounding sphere
template <typename Point>
std::ostream &operator<<(std::ostream &os, const BoundingSphere<Point> &bsphere) {
    os << "BoundingSphere" << type_suffix<Point>();
    if (bsphere.empty())
        os << "[empty]";
    else
        os << "[center = " << bsphere.center << ", radius = " << bsphere.radius << "]";
    return os;
}

NAMESPACE_END(mitsuba)

