#pragma once
#include <iostream>

#include "config.h"

enum class Colour : std::uint8_t { RED, GREEN, PURPLE, DEFAULT };
inline void set_colour(std::ostream& os,
                       const Colour colour = Colour::DEFAULT) {
    if constexpr (use_colours) {
        switch (colour) {
            case Colour::RED:
                os << "\033[31m";
                break;
            case Colour::GREEN:
                os << "\033[32m";
                break;
            case Colour::PURPLE:
                os << "\033[35m";
                break;
            default:
                os << "\033[0m";
                break;
        }
    }
}

inline void output_with_colour(std::ostream& os, const Colour colour,
                               const std::string_view message) {
    set_colour(os, colour);
    os << message;
    set_colour(os);
}