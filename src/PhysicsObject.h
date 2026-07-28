#pragma once
#include "includes.h"

class PhysicsObject {
    public:
        // Properties
        Vector2d position;
        Vector2d velocity;
        double mass;
        double radius;    

        // Methods
        PhysicsObject(Vector2d position, Vector2d velocity, double mass, double radius);
        void update(double dt);
        virtual void draw(sf::RenderWindow& window, float distanceScale) = 0;
        virtual ~PhysicsObject() {};

    protected:
        void setPosition(Vector2d newPos);
        void applyImpulse(Vector2d impulseVector);
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
};

using PhysObj = PhysicsObject;
