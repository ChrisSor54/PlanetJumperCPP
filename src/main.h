#pragma once
#include "includes.h"

#include "PhysicsObject.h"
#include "Player.h"


//--------------------------------------------------------------------------------------------------
// CONSTANTS

const float CAMERA_SPEED = 100.0;
const float CAMERA_LERP_SPEED = 25.0;
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
            sf::Color color,
            bool drawBody
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
            sf::Color color
        );
        
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

        // Methods
        void updateInputStates();
        void handleInput(double dt);
        void update(double dt);
        void draw(sf::RenderWindow& window);
        void drawBackground(sf::RenderWindow& window, double dt);

        // Updates
        void updateRelativeVelocities(Vector2d newCenterVelocity);
        void updateRelativePositions(float lerpScale, Vector2d newCenter);
        void updateRelativeRotations(float lerpScale, sf::Angle newRotation);

        void moveGlobalPositions(Vector2d offset);
        void drawVisualeOrbits(double duration, int resolution);

        // Camera
        void moveCamera(sf::Vector2f offset);
        void zoomCamera(float zoomValue);
        void rotateCamera(sf::Angle targetAngle);
};
