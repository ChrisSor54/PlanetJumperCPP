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
    JUMPING,
    FLYING
};

struct Animation {
    int row;
    int column;
    int numSprites;
    float animSpeed;
};

struct SmokeParticle {
    sf::Vector2f pos = sf::Vector2f(0, 0);
    sf::Vector2f vel = sf::Vector2f(0, 0);
    float lifespan = 0;
    float totalLifespan = 0;
};

const double MOVE_SPEED = 150.0;
const sf::Angle ROTATION_SPEED = sf::degrees(30);
const double THRUSTER_STRENGTH = 100.0;
const double PLAYER_MASS = 5.0;
const double COLLISION_RADIUS = 12.0;
const double PLAYER_ELASTICITY = 0.0;
const double PLAYER_FRICTION = 1.0;

const double MIN_JUMP_CHARGE = 100.0;
const double MAX_JUMP_CHARGE = 2000.0;
const double JUMP_CHARGE_SPEED = 0.75;

// SMOKE

const double SMOKE_RADIUS = 1.0;
const double MIN_SMOKE_VELOCITY = 0.5;
const double MAX_SMOKE_VELOCITY = 5.0;
const double SMOKE_ANGLE_OFFSET = 5;
const double SMOKE_LIFESPAN = 0.5;
const float SMOKE_SPAWN_COOLDOWN = 0.016;
const int MAX_SMOKE = 100;

const float CHARGE_SPRITE_OFFSET = 1.0;
const float SPRITE_WIDTH = 32.0;
const sf::Vector2f SPRITE_ORIGIN = sf::Vector2f(SPRITE_WIDTH/2.f, SPRITE_WIDTH/2.f);


class Player : public PhysObj {
    public:
        // Properties

        enum State state;
        int playerID;
        double speed = MOVE_SPEED;
        sf::RenderTexture spriteTexture;
        sf::Sprite sprite;

        int smokeIndex = 0;
        std::array<SmokeParticle, MAX_SMOKE> smokeArray;
        
        // Methods
        Player(int playerID, sf::Color playerColor);

        State getState();
        void updateGravity(PhysObj& other, double dt);
        void updateCollision(PhysicsObject& other, double dt);
        void setState(State newState);
        void handleInput(InputMap& input, double dt);
        void update(double dt);
        void updateSmoke(double dt);
        void draw(sf::RenderWindow& window) override;
        void draw(sf::RenderWindow& window, Vector2f scale) override;
        void drawSmoke(sf::RenderWindow& window);

        
    private:
        float jumpCharge = 0.0;
        bool flipSprite = false;
        float smokeSpawnCooldown = 0.0;
                
        float animationTimer = 0.0;
        Anim currentAnimation;
        float animSpeed = 0.0;
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
                .animSpeed = 0.05f
            }},
            { Anim::FLYING, {
                .row = 1,
                .column = 1,
                .numSprites = 1,
                .animSpeed = 0
            }},
            { Anim::CHARGING, {
                .row = 1,
                .column = 0,
                .numSprites = 1,
                .animSpeed = 0
            }},
            { Anim::JUMPING, {
                .row = 1,
                .column = 1,
                .numSprites = 1,
                .animSpeed = 0
            }}
        };

        void chargeJump(double dt);
        void jump();
        void fly(Vector2d direction, double dt);
        void rotate(sf::Angle rotationSpeed, double dt);
        void spawnSmokeParticle(double smokeVelocity, sf::Angle angleOffset, double lifespan);
        void playAnimation(Anim anim);
        void playAnimation(Anim anim, bool force);
        void playAnimation(Anim anim, bool force, float customSpeed);
        Vector2d getCollisionImpulse(PhysicsObject& other);
        Vector2d getCollisionImpulse(PhysicsObject& other, double sqrRestThreshold);
        void updateAnimation(float dt);

        static sf::Texture spritesheet;
        static sf::Texture spriteMask;
        static bool spritesheetLoaded;

};
