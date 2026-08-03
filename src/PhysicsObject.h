#pragma once
#include "includes.h"

const float DEFAULT_FRICTION_COEFFICIENT = 0.6;
const float GROUNDED_MARGIN = 0.1;
const double COLLISION_REST_COEFFICIENT = 0.16;



class PhysicsObject {
    static constexpr float ELASTICITY = 0.5;
    
    public:
        int id;
        // Properties
        Vector2d position;
        Vector2d velocity;
        double mass;
        double radius;
        sf::Angle rotation;
        sf::Angle rotationalVelocity;
        float elasticity;
        PhysicsObject* parentObject = nullptr;

        bool hasCollided = false;
        bool isGrounded = false;

        // Methods
        PhysicsObject(double mass, double radius);
        PhysicsObject(Vector2d position, Vector2d velocity, double mass, double radius, sf::Angle rotationalVelocity, float surfaceFriction);
        void updateGravity(PhysicsObject& other, double dt);
        void updateCollision(PhysicsObject& other, double dt);
        void updateVelocity();
        void updatePosition(double dt);
        void fixOverlap(PhysicsObject& other);
        void fixOverlap(PhysicsObject& other, bool force);
        void applyImpulse(Vector2d impulseVector);
        double getSurfaceDistance(PhysicsObject& other);



        virtual void draw(sf::RenderWindow& window) = 0;
        virtual void drawVelocity(sf::RenderWindow& window, Vector2d referenceVelocity, double scale) = 0;
        void drawOrbitalPath(sf::RenderWindow& window, int resolutionScale);
        void drawOrbitalPath(sf::RenderWindow& window, double duration, int resolutionScale);
        virtual ~PhysicsObject() {};

    protected:
        Vector2d velocityBuffer;
        float surfaceFriction; // Must be between 0 and 1        

        // void updateVirtualForces(PhysicsObject& other, double dt);
        // Vector2d getIntegratedForces(PhysicsObject& other, double dt, double resolution);
        bool checkCollision(PhysicsObject& other, double dt);
        bool checkCollision(PhysicsObject& other, double dt, double resolution);
        Vector2d getCollisionImpulse(PhysicsObject& other);
        Vector2d getCollisionImpulse(PhysicsObject& other, double sqrRestThreshold);
        Vector2d getGravityVector(PhysicsObject& other);
        Vector2d getGravityVector(PhysicsObject& other, int distancePower);
        Vector2d getGravityVector(Vector2d position1, double mass1, Vector2d position2, double mass2);
        double getSurfaceVelocity();
        double getOrbitalPeriod(PhysicsObject& referenceObject);
        double getSemiMajorAxis(PhysicsObject& referenceObject); 
        std::vector<Vector2d> getOrbitalPath(double duration, int resolutionScale);
};

class Body : public PhysicsObject {

    public:
        // Properties
        sf::Color color;

        // Methods
        Body(
            Vector2d position,
            Vector2d velocity,
            double mass,
            double radius,
            sf::Angle rotationalVelocity,
            float surfaceFriction,
            sf::Color color
        );
        Body(
            Vector2d position,
            Vector2d velocity,
            double mass,
            double radius,
            sf::Angle rotationalVelocity,
            float surfaceFriction,
            sf::Color color,
            bool drawTexture
        );
        
        void draw(sf::RenderWindow& window) override;
        void drawVelocity(sf::RenderWindow& window, Vector2d referenceVelocity, double scale) override;

    private:
        sf::CircleShape shape;
        static sf::Texture texture;
        static bool textureLoaded;

        
};

using PhysObj = PhysicsObject;
