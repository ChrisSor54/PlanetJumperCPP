#pragma once
#include "includes.h"


class PhysicsObject {
    static constexpr float ELASTICITY = 0.8;
    static constexpr float COLLISION_MARGIN = 0.5;
    
    public:
        int id;
        // Properties
        Vector2d position;
        Vector2d velocity;
        double mass;
        double radius;    

        // Methods
        PhysicsObject(Vector2d position, Vector2d velocity, double mass, double radius);
        void updateForces(PhysicsObject& other, double dt);
        void updatePosition(double dt);

        virtual void draw(sf::RenderWindow& window, float distanceScale) = 0;
        virtual ~PhysicsObject() {};

    protected:
        bool checkCollision(PhysicsObject& other, double dt);
        Vector2d getCollisionImpulse(PhysicsObject& other);
        double getSurfaceDistance(PhysicsObject& other);
        void setPosition(Vector2d newPos);
        void applyImpulse(Vector2d impulseVector);
        Vector2d getGravityVector(PhysicsObject& other);
};

class Body : public PhysicsObject {

    public:
        // Properties
        sf::Color color;

        // Methods
        Body(Vector2d position, Vector2d velocity, double mass, double radius, sf::Color);
        
        void draw(sf::RenderWindow& window, float distanceScale) override;

    private:
        sf::CircleShape shape;
        sf::Text text;
        
};

using PhysObj = PhysicsObject;
