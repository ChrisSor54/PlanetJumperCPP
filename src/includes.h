#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

// Constants
const double G = 1.0;
constexpr double PI = 3.14159265358979323846;


// Aliases

using Vector2d = sf::Vector2<double>;
using Vector2f = sf::Vector2f;


// Structs

enum class InputAction {
    Up,
    Down,
    Left,
    Right,
    Jump,
    Walk,
    RotateR,
    RotateL,
    ZoomIn,
    ZoomOut,
    SpeedUp,
    SpeedDown,
    ResetTimeScale,
    ToggleRotation,
    ToggleFreecam,
    DEBUG,
    DEBUG_ShowVelocities,
    DEBUG_ToggleReference,
    COUNT
};

struct InputState {
    bool pressed = false;
    bool released = false;
};

struct InputManager {
    std::unordered_map<InputAction, sf::Keyboard::Key> bindings = {
        {InputAction::Up, sf::Keyboard::Key::W},
        {InputAction::Down, sf::Keyboard::Key::S},
        {InputAction::Left, sf::Keyboard::Key::A},
        {InputAction::Right, sf::Keyboard::Key::D},
        {InputAction::RotateR, sf::Keyboard::Key::E},
        {InputAction::RotateL, sf::Keyboard::Key::Q},
        {InputAction::Jump, sf::Keyboard::Key::Space},
        {InputAction::Walk, sf::Keyboard::Key::LShift},
        {InputAction::ZoomIn, sf::Keyboard::Key::PageDown},
        {InputAction::ZoomOut, sf::Keyboard::Key::PageUp},
        {InputAction::SpeedUp, sf::Keyboard::Key::Period},
        {InputAction::SpeedDown, sf::Keyboard::Key::Comma},
        {InputAction::ResetTimeScale, sf::Keyboard::Key::Slash},
        {InputAction::ToggleRotation, sf::Keyboard::Key::R},
        {InputAction::ToggleFreecam, sf::Keyboard::Key::F},
        {InputAction::DEBUG, sf::Keyboard::Key::LControl},
        {InputAction::DEBUG_ShowVelocities, sf::Keyboard::Key::S},
        {InputAction::DEBUG_ToggleReference, sf::Keyboard::Key::R},
    };

    sf::Vector2f directionalInput = sf::Vector2f(0,0);
    std::unordered_map<InputAction, InputState> inputStates = {};
};
