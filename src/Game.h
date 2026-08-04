#pragma once
#include "includes.h"

#include "PhysicsObject.h"
#include "Player.h"


//--------------------------------------------------------------------------------------------------
// CONSTANTS

const float CAMERA_SPEED = 200.0;
const float CAMERA_LERP_SPEED = 10.0;
const int COLLISION_RESOLUTION = 8;
const float MIN_ZOOM_SCALE = 0.125;
const double BACKGROUND_SCROLL_SPEED = 0.01;

const bool DEBUG_VELOCITY_SCALE = 1.0;
const double VISUAL_ORBIT_DURATION = 10;
const int VISUAL_ORBIT_RESOLUTION_SCALE = 30; 


class Game {
    public:
        // Members
        sf::RenderWindow window;

        // Methods
        Game(unsigned int window_w, unsigned int window_h);
        void run();

        Body* addBody(
            Vector2d position,
            Vector2d velocity,
            double mass,
            double radius,
            sf::Angle rotationalVelocity,
            double frictionCoefficient,
            MatterState state,
            sf::Color color
        );

        Body* addBody(
            Vector2d position,
            Vector2d velocity,
            double mass,
            double radius,
            sf::Angle rotationalVelocity,
            double frictionCoefficient,
            MatterState state,
            sf::Color color,
            BodyTexture bodyTexture
        );
        
        Body* addSatellite(
            PhysicsObject* parent,
            sf::Angle angle,
            double semiMajorAxis,
            double eccentricity,
            bool antiClockwise,
            double mass,
            double radius,
            sf::Angle rotationalVelocity,
            double frictionCoefficient,
            MatterState state,
            sf::Color color
        );

        Body* addSatellite(
            PhysicsObject* parent,
            sf::Angle angle,
            double semiMajorAxis,
            double eccentricity,
            bool antiClockwise,
            double mass,
            double radius,
            sf::Angle rotationalVelocity,
            double frictionCoefficient,
            MatterState state,
            sf::Color color,
            BodyTexture satTexture
        );

        void teleportPlayerTo(PhysicsObject* target);
        
    private:
        // Members
        InputManager inputManager;
        std::vector<std::unique_ptr<PhysicsObject>> objects;
        Vector2d globalOrigin = Vector2d(0, 0);
        Vector2d globalVelocity = Vector2d(0, 0);
        sf::Angle globalRotation = sf::Angle::Zero;
        sf::Vector2f bgOffset = sf::Vector2f(0,0);
        float zoomScale = 1.0;
        float timeScale = 1.0;
        Player player;
        float dt;

        bool freecamEnabled = false;
        bool copyRotation = true;

        sf::Vector2f bgSize;
        sf::Texture bgTexture;
        sf::Sprite bgSprite;

        // Debug flags
        bool drawVelocities = false;
        bool useParentAsReference = true;
        bool enlargePlanets = false;

        // Methods
        void updateInputStates();
        void handleInput(double dt);
        void update(double dt);
        void draw(sf::RenderWindow& window);
        void drawBackground(sf::RenderWindow& window, double dt);

        // Updates
        void updateRelativeVelocities(Vector2d newReferenceFrame);
        void updateRelativePositions(Vector2d newOrigin);
        void updateRelativePositions(Vector2d newOrigin, float lerpScale);
        void updateRelativeRotations(sf::Angle newRotation, float lerpScale);

        
        void drawVisualeOrbits(double duration, int resolution);

        // Camera
        void moveCamera(sf::Vector2f offset);
        void moveCamera(sf::Vector2f offset, Vector2f backGroundOffset);
        void zoomCamera(float zoomValue);
        void rotateCamera(sf::Angle targetAngle);
};
