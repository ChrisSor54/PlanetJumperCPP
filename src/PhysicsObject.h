#pragma once
#include "includes.h"

const float DEFAULT_FRICTION_COEFFICIENT = 0.6;
const float GROUNDED_MARGIN = 0.0;
const double COLLISION_MARGIN = 0.5;



class PhysicsObject {
    static constexpr float ELASTICITY = 0.8;
    
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
        PhysicsObject(Vector2d position, Vector2d velocity, double mass, double radius, float frictionCoefficient);
        void updateForces(PhysicsObject& other, double dt);
        void updatePosition(double dt);
        void fixOverlap(PhysicsObject& other);
        void fixOverlap(PhysicsObject& other, bool force);
        void applyImpulse(Vector2d impulseVector);
        double getSurfaceDistance(PhysicsObject& other);



        virtual void draw(sf::RenderWindow& window, float distanceScale) = 0;
        virtual ~PhysicsObject() {};

    protected:
        Vector2d velocityBuffer;
        float frictionCoefficient; // Must be between 0 and 1

        void updateVirtualForces(PhysicsObject& other, double dt);
        Vector2d getIntegratedForces(PhysicsObject& other, double dt, double resolution);
        bool checkCollision(PhysicsObject& other, double dt);
        bool checkCollision(PhysicsObject& other, double dt, double resolution);
        Vector2d getCollisionImpulse(PhysicsObject& other);
        Vector2d getCollisionImpulse(PhysicsObject& other, double elasticity);
        void setPosition(Vector2d newPos);
        Vector2d getGravityVector(PhysicsObject& other);
        
};

class Body : public PhysicsObject {

    public:
        // Properties
        sf::Color color;

        // Methods
        Body(Vector2d position, Vector2d velocity, double mass, double radius, float frictionCoefficient, sf::Color color);
        
        void draw(sf::RenderWindow& window, float distanceScale) override;

    private:
        sf::CircleShape shape;

        
};

using PhysObj = PhysicsObject;
