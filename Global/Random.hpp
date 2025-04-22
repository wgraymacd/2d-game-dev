#pragma once

#include <chrono>
#include <random>

// This header-only Random namespace implements a self-seeding Mersenne Twister.
// Requires C++17 or newer.
// It can be #included into as many code files as needed (The inline keyword avoids ODR violations)
namespace Random
{
    // Returns a seeded Mersenne Twister
    // Note: we'd prefer to return a std::seed_seq (to initialize a std::mt19937), but std::seed can't be copied, so it can't be returned by value.
    // Instead, we'll create a std::mt19937, seed it, and then return the std::mt19937 (which can be copied).
    inline std::mt19937 generate()
    {
        std::random_device rd { };

        // Create seed_seq with clock and 7 random numbers from std::random_device
        std::seed_seq ss {
            static_cast<std::seed_seq::result_type>(
                std::chrono::steady_clock::now().time_since_epoch().count()
            ),
            rd(), rd(), rd(), rd(), rd(), rd(), rd()
        };

        return std::mt19937 { ss };
    }

    // Generates a seeded std::mt19937 and copies it into our global object
    inline std::mt19937 mt { generate() };

    /// @brief Generate a random integral type between [min, max] (all same type)
    template <typename T>
    T getIntegral(T min, T max)
    {
        return std::uniform_int_distribution<T>{min, max}(mt);
    }

    /// @brief Generate a random floating point type between [min, max] (all same type)
    template <typename T>
    T getFloatingPoint(T min, T max)
    {
        return std::uniform_real_distribution<T>{min, max}(mt);
    }

    // Generate a random value between [min, max] (inclusive)
    // * min and max can have different types
        // * return type must be explicitly specified as a template argument
    // * min and max will be converted to the return type
    // Sample call: Random::get<std::size_t>(0, 6);  // returns std::size_t
    // Sample call: Random::get<std::size_t>(0, 6u); // returns std::size_t
    // Sample call: Random::get<std::int>(0, 6u);    // returns int
    // template <typename R, typename S, typename T>
    // R get(S min, T max)
    // {
    //     return get<R>(static_cast<R>(min), static_cast<R>(max));
    // }
}
