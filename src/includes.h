#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>


// Includes are used by multiple files


// Constants
const double G = 1.0;
constexpr double PI = 3.14159265358979323846;


// Aliases

using Vector2d = sf::Vector2<double>;
using Vector2f = sf::Vector2f;

using sfKey = sf::Keyboard::Scan;


// Input

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
    DEBUG_EnlargePlanets,
    COUNT
};

struct InputState {
    bool pressed = false;
    bool released = false;
};

struct InputManager {
    std::unordered_map<InputAction, std::vector<sfKey>> bindings = {
        {InputAction::Up,                       {sfKey::W, sfKey::Up}},
        {InputAction::Down,                     {sfKey::S, sfKey::Down}},
        {InputAction::Left,                     {sfKey::A, sfKey::Left}},
        {InputAction::Right,                    {sfKey::D, sfKey::Right}},
        {InputAction::RotateR,                  {sfKey::E}},
        {InputAction::RotateL,                  {sfKey::Q}},
        {InputAction::Jump,                     {sfKey::Space}},
        {InputAction::Walk,                     {sfKey::LShift}},
        {InputAction::ZoomIn,                   {sfKey::PageDown}},
        {InputAction::ZoomOut,                  {sfKey::PageUp}},
        {InputAction::SpeedUp,                  {sfKey::Period}},
        {InputAction::SpeedDown,                {sfKey::Comma}},
        {InputAction::ResetTimeScale,           {sfKey::Slash}},
        {InputAction::ToggleRotation,           {sfKey::R}},
        {InputAction::ToggleFreecam,            {sfKey::F}},
        {InputAction::DEBUG,                    {sfKey::LControl}},
        {InputAction::DEBUG_ShowVelocities,     {sfKey::S}},
        {InputAction::DEBUG_ToggleReference,    {sfKey::R}},
        {InputAction::DEBUG_EnlargePlanets,     {sfKey::E}},
    };

    Vector2f directionalInput = Vector2f(0,0);
    std::unordered_map<InputAction, InputState> inputStates = {};
};


struct BodyTexture {
    int xOrigin = 0;
    int yOrigin = 0;
    int width = 32;
    int height = 32;
    bool repeat = true;
};

