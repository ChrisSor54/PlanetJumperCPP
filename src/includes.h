#pragma once
#include <SFML/Graphics.hpp>
#include "input_bindings.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdlib>
#define FMT_HEADER_ONLY
#include <fmt/format.h>

// Includes are used by multiple files


// Constants
const double G = 1.0;
constexpr double PI = 3.14159265358979323846;


// Aliases

using Vector2d = sf::Vector2<double>;
using Vector2f = sf::Vector2f;

using sfKey = sf::Keyboard::Scan;


struct BodyTexture {
    int xOrigin = 0;
    int yOrigin = 0;
    int width = 32;
    int height = 32;
    bool repeat = true;
};

