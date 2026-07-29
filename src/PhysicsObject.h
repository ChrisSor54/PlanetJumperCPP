#pragma once
#include "includes.h"


class PhysicsObject {
    static constexpr float ELASTICITY = 0.8;
    static constexpr float COLLISION_DAMP_MARGIN = 1.0;
    
    public:
        int id;
        // Properties
        Vector2d position;
        Vector2d velocity;
        double mass;
        double radius;
        sf::Angle rotation;
        float elasticity;
        PhysicsObject* parentObject = nullptr;

        bool hasCollided;

        // Methods
        PhysicsObject(double mass, double radius);
        PhysicsObject(Vector2d position, Vector2d velocity, double mass, double radius);
        void updateForces(PhysicsObject& other, double dt);
        void updatePosition(double dt);
        void fixOverlap(PhysicsObject& other, double dt);

        virtual void draw(sf::RenderWindow& window, float distanceScale) = 0;
        virtual ~PhysicsObject() {};

    protected:

        bool checkCollision(PhysicsObject& other, double dt);
        Vector2d getCollisionImpulse(PhysicsObject& other);
        Vector2d getCollisionImpulse(PhysicsObject& other, double elasticity);
        double getSurfaceDistance(PhysicsObject& other);
        void setPosition(Vector2d newPos);
        void applyImpulse(Vector2d impulseVector);
        Vector2d getGravityVector(PhysicsObject& other);
        

    private:
        Vector2d velocityBuffer;
};

class Body : public PhysicsObject {

    public:
        // Properties
        sf::Color color;

        // Methods
        Body(Vector2d position, Vector2d velocity, double mass, double radius, sf::Color color);
        
        void draw(sf::RenderWindow& window, float distanceScale) override;

    private:
        sf::CircleShape shape;

        
};

using PhysObj = PhysicsObject;
