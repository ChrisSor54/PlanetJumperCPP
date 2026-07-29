#pragma once
#include "includes.h"
#include "PhysicsObject.h"

enum class State {
    GROUNDED,
    FLYING,
    DEAD
};

enum class Anim {
    IDLE,
    WALKING,
    CHARGING,
    FLYING
};

struct Animation {
    int row;
    int column;
    int numSprites;
    float animSpeed;
};

const double MOVE_SPEED = 5;
const double PLAYER_MASS = 7.0;
const double COLLISION_RADIUS = 12.0;
const float SPRITE_WIDTH = 32.0;


class Player : public PhysObj {
    public:
        // Properties

        enum State state;
        double speed;
        sf::RenderTexture spriteTexture;
        sf::Sprite sprite;
        float animSpeed;
        
        // Methods
        Player();

        State getState();
        void setState(State newState);
        void handleInput(sf::RenderWindow& window, double dt);
        void update(double dt);
        void draw(sf::RenderWindow& window, float distanceScale) override;

        
    private:
    float animationTimer;
    Anim currentAnimation;
    Anim animationBuffer;
    State stateBuffer;
    bool flipSprite;


    std::unordered_map<Anim, Animation> animations = {
        { Anim::IDLE, {
        .row = 0,
        .column = 0,
        .numSprites = 1,
        .animSpeed = 0
        }},
        { Anim::WALKING, {
            .row = 0,
            .column = 0,
            .numSprites = 4,
            .animSpeed = 4
        }}
    };

    void playAnimation(Anim anim);
    void updateState();
    void updateAnimation(float dt);

};

// Animations

