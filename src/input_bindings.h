#pragma once


// Input


using Vector2d = sf::Vector2<double>;
using Vector2f = sf::Vector2f;
using sfKey = sf::Keyboard::Scan;


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
    ResetTimescale,
    ToggleRotation,
    ToggleFreecam,
    AddPlayer,
    RemovePlayer,
    DEBUG,
    DEBUG_ShowVelocities,
    DEBUG_ToggleReference,
    COUNT
};

struct InputState {
    bool pressed = false;
    bool released = false;
};

struct InputMap {
    std::unordered_map<InputAction, std::vector<sfKey>> bindings;
    std::unordered_map<InputAction, InputState> inputStates;
    Vector2f directionalInput = Vector2f(0,0);
};



struct InputManager {
    std::unordered_map<int, InputMap> playerInputs = {
        {0, {{
            {InputAction::Up,                       {sfKey::W}},
            {InputAction::Down,                     {sfKey::S}},
            {InputAction::Left,                     {sfKey::A}},
            {InputAction::Right,                    {sfKey::D}},
            {InputAction::Jump,                     {sfKey::Space}},
            {InputAction::Walk,                     {sfKey::LShift}},
            {InputAction::RotateR,                  {}},
            {InputAction::RotateL,                  {}},
            {InputAction::ZoomIn,                   {sfKey::E}},
            {InputAction::ZoomOut,                  {sfKey::Q}},
            }, {}, Vector2f(0,0)
        }},
        {1, {{
            {InputAction::Up,                       {sfKey::Up}},
            {InputAction::Down,                     {sfKey::Down}},
            {InputAction::Left,                     {sfKey::Left}},
            {InputAction::Right,                    {sfKey::Right}},
            {InputAction::Jump,                     {sfKey::RControl}},
            {InputAction::Walk,                     {sfKey::RShift}},
            {InputAction::RotateR,                  {}},
            {InputAction::RotateL,                  {}},
            {InputAction::ZoomIn,                   {sfKey::PageDown}},
            {InputAction::ZoomOut,                  {sfKey::PageUp}},
            }, {}, Vector2f(0,0)
        }},
        {2, {{
            {InputAction::Up,                       {sfKey::W}},
            {InputAction::Down,                     {sfKey::S}},
            {InputAction::Left,                     {sfKey::A}},
            {InputAction::Right,                    {sfKey::D}},
            {InputAction::Jump,                     {sfKey::Space}},
            {InputAction::Walk,                     {sfKey::LShift}},
            {InputAction::RotateR,                  {sfKey::E}},
            {InputAction::RotateL,                  {sfKey::Q}},
            {InputAction::ZoomIn,                   {sfKey::PageDown}},
            {InputAction::ZoomOut,                  {sfKey::PageUp}},
            }, {}, Vector2f(0,0)
        }},
        {3, {{
            {InputAction::Up,                       {sfKey::W}},
            {InputAction::Down,                     {sfKey::S}},
            {InputAction::Left,                     {sfKey::A}},
            {InputAction::Right,                    {sfKey::D}},
            {InputAction::Jump,                     {sfKey::Space}},
            {InputAction::Walk,                     {sfKey::LShift}},
            {InputAction::RotateR,                  {sfKey::E}},
            {InputAction::RotateL,                  {sfKey::Q}},
            {InputAction::ZoomIn,                   {sfKey::PageDown}},
            {InputAction::ZoomOut,                  {sfKey::PageUp}},
            }, {}, Vector2f(0,0)
        }}
    };

    InputMap globalInputs = {{
        {InputAction::SpeedUp,                  {sfKey::Period}},
        {InputAction::SpeedDown,                {sfKey::Comma}},
        {InputAction::ResetTimescale,           {sfKey::Slash}},
        {InputAction::ToggleRotation,           {sfKey::R}},
        {InputAction::ToggleFreecam,            {sfKey::F}},
        {InputAction::AddPlayer,                {sfKey::Equal}},
        {InputAction::RemovePlayer,             {sfKey::Hyphen}},
        {InputAction::DEBUG,                    {sfKey::LControl}},
        {InputAction::DEBUG_ShowVelocities,     {sfKey::S}},
        {InputAction::DEBUG_ToggleReference,    {sfKey::R}},
    }, {}, Vector2f(0, 0)
    };
};

const std::unordered_map<std::string, InputAction> inputActionNames = {
    {"Up",                   InputAction::Up},
    {"Down",                 InputAction::Down},
    {"Left",                 InputAction::Left},
    {"Right",                InputAction::Right},
    {"Jump",                 InputAction::Jump},
    {"Walk",                 InputAction::Walk},
    {"RotateR",              InputAction::RotateR},
    {"RotateL",              InputAction::RotateL},
    {"ZoomIn",               InputAction::ZoomIn},
    {"ZoomOut",              InputAction::ZoomOut},
    {"SpeedUp",              InputAction::SpeedUp},
    {"SpeedDown",            InputAction::SpeedDown},
    {"ResetTimescale",       InputAction::ResetTimescale},
    {"ToggleRotation",       InputAction::ToggleRotation},
    {"ToggleFreecam",        InputAction::ToggleFreecam},
    {"AddPlayer",            InputAction::AddPlayer},
    {"RemovePlayer",         InputAction::RemovePlayer},
    {"DEBUG",                InputAction::DEBUG},
    {"DEBUG_ShowVelocities", InputAction::DEBUG_ShowVelocities},
    {"DEBUG_ToggleReference", InputAction::DEBUG_ToggleReference},
};
