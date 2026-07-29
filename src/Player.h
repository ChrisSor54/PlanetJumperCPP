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
};

const double MOVE_SPEED = 100;
const double PLAYER_MASS = 7.0;
const double COLLISION_RADIUS = 12.0;
const float SPRITE_WIDTH = 32.0;


class Player : public PhysObj {
    public:
        // Properties

        enum State state;
        double speed;
        sf::Texture spriteTexture;
        sf::Sprite sprite;
        float animSpeed;
        
        // Methods
        Player();

        void handleInput(sf::Event inputEvent, double dt);
        void update(double dt);
        void draw(sf::RenderWindow& window, float distanceScale) override;

        
    private:
    sf::Clock animationTimer;
    bool flipSprite;
    std::unordered_map<Anim, Animation> animations = {
        { Anim::IDLE, {
        .row = 0,
        .column = 0,
        .numSprites = 1
        }}
    };

    void playAnimation(Anim anim);

};

// Animations

