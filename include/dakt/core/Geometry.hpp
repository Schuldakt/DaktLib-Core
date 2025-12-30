// ============================================================================
// DaktLib Core Module - Geometry Types
// ============================================================================
// Fundamental geometry and color types that can be used independently
// without pulling in the full GUI module.
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace dakt::core
{

// ============================================================================
// Vector Types
// ============================================================================

struct Vec2
{
    f32 x = 0.0f;
    f32 y = 0.0f;

    constexpr Vec2() = default;
    constexpr Vec2(f32 x_, f32 y_) : x(x_), y(y_) {}
    constexpr explicit Vec2(f32 v) : x(v), y(v) {}

    [[nodiscard]] constexpr Vec2 operator+(Vec2 other) const { return {x + other.x, y + other.y}; }
    [[nodiscard]] constexpr Vec2 operator-(Vec2 other) const { return {x - other.x, y - other.y}; }
    [[nodiscard]] constexpr Vec2 operator*(f32 s) const { return {x * s, y * s}; }
    [[nodiscard]] constexpr Vec2 operator/(f32 s) const { return {x / s, y / s}; }
    [[nodiscard]] constexpr Vec2 operator*(Vec2 other) const { return {x * other.x, y * other.y}; }
    [[nodiscard]] constexpr Vec2 operator/(Vec2 other) const { return {x / other.x, y / other.y}; }

    constexpr Vec2& operator+=(Vec2 other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    constexpr Vec2& operator-=(Vec2 other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    constexpr Vec2& operator*=(f32 s)
    {
        x *= s;
        y *= s;
        return *this;
    }
    constexpr Vec2& operator/=(f32 s)
    {
        x /= s;
        y /= s;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(Vec2 other) const { return x == other.x && y == other.y; }
    [[nodiscard]] constexpr bool operator!=(Vec2 other) const { return !(*this == other); }

    [[nodiscard]] constexpr Vec2 operator-() const { return {-x, -y}; }

    [[nodiscard]] f32 length() const { return std::sqrt(x * x + y * y); }
    [[nodiscard]] constexpr f32 lengthSq() const { return x * x + y * y; }
    [[nodiscard]] constexpr f32 dot(Vec2 other) const { return x * other.x + y * other.y; }
    [[nodiscard]] constexpr f32 cross(Vec2 other) const { return x * other.y - y * other.x; }

    [[nodiscard]] Vec2 normalized() const
    {
        f32 len = length();
        return len > 0.0f ? *this / len : Vec2{};
    }

    [[nodiscard]] constexpr Vec2 perpendicular() const { return {-y, x}; }
    [[nodiscard]] constexpr Vec2 abs() const { return {std::abs(x), std::abs(y)}; }
    [[nodiscard]] constexpr Vec2 floor() const { return {std::floor(x), std::floor(y)}; }
    [[nodiscard]] constexpr Vec2 ceil() const { return {std::ceil(x), std::ceil(y)}; }
    [[nodiscard]] constexpr Vec2 round() const { return {std::round(x), std::round(y)}; }

    [[nodiscard]] static constexpr Vec2 min(Vec2 a, Vec2 b) { return {std::min(a.x, b.x), std::min(a.y, b.y)}; }
    [[nodiscard]] static constexpr Vec2 max(Vec2 a, Vec2 b) { return {std::max(a.x, b.x), std::max(a.y, b.y)}; }
    [[nodiscard]] static constexpr Vec2 lerp(Vec2 a, Vec2 b, f32 t) { return a + (b - a) * t; }
};

struct Vec4
{
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;
    f32 w = 0.0f;

    constexpr Vec4() = default;
    constexpr Vec4(f32 x_, f32 y_, f32 z_, f32 w_) : x(x_), y(y_), z(z_), w(w_) {}
    constexpr explicit Vec4(f32 v) : x(v), y(v), z(v), w(v) {}

    [[nodiscard]] constexpr Vec4 operator+(Vec4 other) const
    {
        return {x + other.x, y + other.y, z + other.z, w + other.w};
    }
    [[nodiscard]] constexpr Vec4 operator*(f32 s) const { return {x * s, y * s, z * s, w * s}; }
};

// ============================================================================
// Color Types
// ============================================================================

struct Color
{
    u8 r = 255;
    u8 g = 255;
    u8 b = 255;
    u8 a = 255;

    constexpr Color() = default;
    constexpr Color(u8 r_, u8 g_, u8 b_, u8 a_ = 255) : r(r_), g(g_), b(b_), a(a_) {}

    [[nodiscard]] static constexpr Color fromFloat(f32 r, f32 g, f32 b, f32 a = 1.0f)
    {
        return Color{
            static_cast<u8>(std::clamp(r, 0.0f, 1.0f) * 255.0f), static_cast<u8>(std::clamp(g, 0.0f, 1.0f) * 255.0f),
            static_cast<u8>(std::clamp(b, 0.0f, 1.0f) * 255.0f), static_cast<u8>(std::clamp(a, 0.0f, 1.0f) * 255.0f)};
    }

    [[nodiscard]] static constexpr Color fromU32(u32 rgba)
    {
        return Color{static_cast<u8>((rgba >> 24) & 0xFF), static_cast<u8>((rgba >> 16) & 0xFF),
                     static_cast<u8>((rgba >> 8) & 0xFF), static_cast<u8>(rgba & 0xFF)};
    }

    [[nodiscard]] static Color fromHSV(f32 h, f32 s, f32 v, f32 a = 1.0f)
    {
        // HSV to RGB conversion
        f32 c = v * s;
        f32 x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
        f32 m = v - c;

        f32 r1, g1, b1;
        if (h < 60.0f)
        {
            r1 = c;
            g1 = x;
            b1 = 0.0f;
        }
        else if (h < 120.0f)
        {
            r1 = x;
            g1 = c;
            b1 = 0.0f;
        }
        else if (h < 180.0f)
        {
            r1 = 0.0f;
            g1 = c;
            b1 = x;
        }
        else if (h < 240.0f)
        {
            r1 = 0.0f;
            g1 = x;
            b1 = c;
        }
        else if (h < 300.0f)
        {
            r1 = x;
            g1 = 0.0f;
            b1 = c;
        }
        else
        {
            r1 = c;
            g1 = 0.0f;
            b1 = x;
        }

        return Color::fromFloat(r1 + m, g1 + m, b1 + m, a);
    }

    [[nodiscard]] constexpr u32 toU32() const
    {
        return (static_cast<u32>(r) << 24) | (static_cast<u32>(g) << 16) | (static_cast<u32>(b) << 8) |
               static_cast<u32>(a);
    }

    [[nodiscard]] constexpr u32 toABGR() const
    {
        return (static_cast<u32>(a) << 24) | (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) |
               static_cast<u32>(r);
    }

    [[nodiscard]] constexpr Vec4 toFloat() const { return Vec4{r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f}; }

    [[nodiscard]] constexpr Color withAlpha(u8 alpha) const { return Color{r, g, b, alpha}; }

    [[nodiscard]] constexpr Color withAlpha(f32 alpha) const
    {
        return Color{r, g, b, static_cast<u8>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f)};
    }

    [[nodiscard]] static constexpr Color lerp(Color a, Color b, f32 t)
    {
        return Color{static_cast<u8>(a.r + (b.r - a.r) * t), static_cast<u8>(a.g + (b.g - a.g) * t),
                     static_cast<u8>(a.b + (b.b - a.b) * t), static_cast<u8>(a.a + (b.a - a.a) * t)};
    }

    [[nodiscard]] constexpr bool operator==(Color other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    // Predefined colors
    static constexpr Color white() { return {255, 255, 255, 255}; }
    static constexpr Color black() { return {0, 0, 0, 255}; }
    static constexpr Color red() { return {255, 0, 0, 255}; }
    static constexpr Color green() { return {0, 255, 0, 255}; }
    static constexpr Color blue() { return {0, 0, 255, 255}; }
    static constexpr Color yellow() { return {255, 255, 0, 255}; }
    static constexpr Color cyan() { return {0, 255, 255, 255}; }
    static constexpr Color magenta() { return {255, 0, 255, 255}; }
    static constexpr Color transparent() { return {0, 0, 0, 0}; }
    static constexpr Color gray(u8 v = 128) { return {v, v, v, 255}; }
};

// ============================================================================
// Rectangle Types
// ============================================================================

struct Rect
{
    Vec2 min;  // Top-left corner
    Vec2 max;  // Bottom-right corner

    constexpr Rect() = default;
    constexpr Rect(Vec2 min_, Vec2 max_) : min(min_), max(max_) {}
    constexpr Rect(f32 x, f32 y, f32 w, f32 h) : min(x, y), max(x + w, y + h) {}

    [[nodiscard]] static constexpr Rect fromPosSize(Vec2 pos, Vec2 size) { return Rect{pos, pos + size}; }

    [[nodiscard]] static constexpr Rect fromCenter(Vec2 center, Vec2 halfSize)
    {
        return Rect{center - halfSize, center + halfSize};
    }

    [[nodiscard]] constexpr f32 x() const { return min.x; }
    [[nodiscard]] constexpr f32 y() const { return min.y; }
    [[nodiscard]] constexpr f32 width() const { return max.x - min.x; }
    [[nodiscard]] constexpr f32 height() const { return max.y - min.y; }
    [[nodiscard]] constexpr Vec2 size() const { return max - min; }
    [[nodiscard]] constexpr Vec2 center() const { return (min + max) * 0.5f; }

    [[nodiscard]] constexpr Vec2 topLeft() const { return min; }
    [[nodiscard]] constexpr Vec2 topRight() const { return {max.x, min.y}; }
    [[nodiscard]] constexpr Vec2 bottomLeft() const { return {min.x, max.y}; }
    [[nodiscard]] constexpr Vec2 bottomRight() const { return max; }

    [[nodiscard]] constexpr f32 left() const { return min.x; }
    [[nodiscard]] constexpr f32 right() const { return max.x; }
    [[nodiscard]] constexpr f32 top() const { return min.y; }
    [[nodiscard]] constexpr f32 bottom() const { return max.y; }

    [[nodiscard]] constexpr bool contains(Vec2 p) const
    {
        return p.x >= min.x && p.x < max.x && p.y >= min.y && p.y < max.y;
    }

    [[nodiscard]] constexpr bool contains(Rect other) const
    {
        return other.min.x >= min.x && other.max.x <= max.x && other.min.y >= min.y && other.max.y <= max.y;
    }

    [[nodiscard]] constexpr bool overlaps(Rect other) const
    {
        return min.x < other.max.x && max.x > other.min.x && min.y < other.max.y && max.y > other.min.y;
    }

    [[nodiscard]] constexpr Rect intersection(Rect other) const
    {
        return Rect{Vec2::max(min, other.min), Vec2::min(max, other.max)};
    }

    [[nodiscard]] constexpr Rect united(Rect other) const
    {
        return Rect{Vec2::min(min, other.min), Vec2::max(max, other.max)};
    }

    [[nodiscard]] constexpr Rect expanded(f32 amount) const { return Rect{min - Vec2(amount), max + Vec2(amount)}; }

    [[nodiscard]] constexpr Rect expanded(Vec2 amount) const { return Rect{min - amount, max + amount}; }

    [[nodiscard]] constexpr Rect contracted(f32 amount) const { return expanded(-amount); }

    [[nodiscard]] constexpr Rect translated(Vec2 offset) const { return Rect{min + offset, max + offset}; }

    [[nodiscard]] constexpr Rect withPadding(f32 left, f32 top, f32 right, f32 bottom) const
    {
        return Rect{min + Vec2{left, top}, max - Vec2{right, bottom}};
    }

    [[nodiscard]] constexpr bool isValid() const { return max.x > min.x && max.y > min.y; }

    [[nodiscard]] constexpr f32 area() const { return width() * height(); }
};

// ============================================================================
// Shape Types
// ============================================================================

struct Circle
{
    Vec2 center;
    f32 radius = 0.0f;

    constexpr Circle() = default;
    constexpr Circle(Vec2 c, f32 r) : center(c), radius(r) {}
    constexpr Circle(f32 x, f32 y, f32 r) : center(x, y), radius(r) {}

    [[nodiscard]] constexpr bool contains(Vec2 p) const { return (p - center).lengthSq() <= radius * radius; }

    [[nodiscard]] constexpr Rect bounds() const { return Rect::fromCenter(center, Vec2(radius)); }
};

}  // namespace dakt::core
