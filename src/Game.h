#pragma once
#include "includes.h"

#include "PhysicsObject.h"
#include "Player.h"



struct Camera {
    sf::View view;
    sf::View uiView;
    Vector2d position = Vector2d(0,0);
    Vector2d velocity = Vector2d(0,0);
    Vector2f bgOffset = Vector2f(0,0);
    float zoomScale = 1.0;
};



//--------------------------------------------------------------------------------------------------
// CONSTANTS

const float CAMERA_SPEED = 200.0;
const float CAMERA_LERP_SPEED = 1.0;
const float CAMERA_ROTATE_SPEED = 10.0;
const int COLLISION_RESOLUTION = 8;
const double ZOOM_SPEED = 1.5;
const float MIN_ZOOM_SCALE = 1/(3*ZOOM_SPEED);
const double BACKGROUND_SCROLL_SPEED = 0.1;

const bool DEBUG_VELOCITY_SCALE = 1.0;
const double VISUAL_ORBIT_DURATION = 10;
const int VISUAL_ORBIT_RESOLUTION_SCALE = 600;



class Game {
    public:
        // Members
        sf::RenderWindow window;

        // Methods
        Game(unsigned int window_w, unsigned int window_h);
        void run();

        Player* addPlayer(sf::Color color);
        void removePlayer(int playerID);

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

<<<<<<< Updated upstream
        void teleportPlayerTo(Player* player, PhysicsObject* target);
=======
        void teleportPlayerTo(int playerID, PhysicsObject* target);

        void drawString(sf::RenderTarget& target, sf::String string, sf::Vector2f pos);
>>>>>>> Stashed changes
        
    private:
        // Members
        InputManager inputManager;
        std::vector<std::unique_ptr<PhysicsObject>> objects;
        Vector2d globalOrigin = Vector2d(0, 0);
        Vector2d globalVelocity = Vector2d(0, 0);
        sf::Angle globalRotation = sf::Angle::Zero;
        float zoomScale = 1.0;
        float timeScale = 1.0;
        float dt;

        bool queueAddPlayer = false;
        bool queueRemovePlayer = false;


        std::vector<std::unique_ptr<Player>> players;
        unsigned int playerCount = 0;

        bool freecamEnabled = false;
        bool copyRotation = true;

        //std::vector<Camera> cameras;
        std::unordered_map<int, Camera> cameras;

        sf::Vector2f bgSize;
        sf::Texture bgTexture;
        sf::Sprite bgSprite;

        static sf::Texture fontSpritesheet;
        static bool fontSpritesheetLoaded;
        std::vector<sf::Sprite> stringSprites;
        

        // Debug flags
        bool drawVelocities = false;
        bool useParentAsReference = true;

        // Methods
        void updateInputStates();
        void handleInput(double dt);
        bool inputPressed(InputAction input);
        bool inputPressed(int playerID, InputAction input);
        bool inputReleased(InputAction input);
        bool inputReleased(int playerID, InputAction input);
        void update(double dt);

        // Updates
        void updateRelativeVelocities(Vector2d newReferenceFrame);
        void updateRelativePositions(Vector2d newOrigin);
        void updateRelativeRotations(int cameraID, sf::Angle newRotation);
        void updateCameras(double dt);
        
        void drawVisualOrbits(double duration, int resolution);

        // Cameras and Views
        void updateViews();
        void centerCamera(int playerID);
        void centerCamera(int playerID, float lerpScale);
        void moveCamera(int cameraID, Vector2d offset);
        void moveCamera(int cameraID, Vector2d offset, Vector2f backGroundOffset);
        void zoomCamera(int cameraID, float zoomValue);
        void rotateCamera(int cameraID, sf::Angle targetAngle);
        void rotateCamera(int cameraID, sf::Angle targetAngle, float lerpScale);

        // Drawing
        void draw(sf::RenderWindow& window);
        void drawBackground(int cameraID, sf::RenderWindow& window, double dt);
        void drawUI(sf::RenderWindow& window);
};
