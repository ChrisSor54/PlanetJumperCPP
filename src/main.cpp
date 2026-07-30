#include "main.h"


//--------------------------------------------------------------------------------------------------
// CONSTANTS

const float CAMERA_SPEED = 100.0;
const float CAMERA_LERP_SPEED = 10.0;



//--------------------------------------------------------------------------------------------------
// VARIABLES



//--------------------------------------------------------------------------------------------------
// Game

Game::Game(unsigned int window_w, unsigned int window_h) {
    window.create(sf::VideoMode({window_w, window_h}), "Planet Jumper");
}

void Game::run() {
    sf::Clock clock;
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    // Planetary System Initialization

    addBody(Vector2d(100.f, 0.f), Vector2d(0, 0), 100000000.0, 30.0, sf::Color(255, 255, 255));    
    //addBody(Vector2d(200.f, 0.f), Vector2d(0, 0), 1000000000.0, 30.0, sf::Color(255, 255, 255));

    for (auto& obj1 : objects) {
        for (auto& obj2 : objects) {
            if (&obj1 == &obj2) {
                continue;
            }
            obj1->fixOverlap(*obj2);
        }
        obj1->fixOverlap(player);
        player.fixOverlap(*obj1);
    }

    while (window.isOpen()) {
        
        dt = clock.restart().asSeconds(); // get deltatime
        dt = std::min(dt, 1.f);
        //std::cout << dt << std::endl;
        // dt = 0.016;
        //dt = 1.0;
        handleInput(dt);
        update(dt);
        draw(window);
        
    }
}

void Game::handleInput(double dt) {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        else if (const auto* keyPressed = event->getIf<sf::Event::KeyReleased>())
        {
            switch(keyPressed->code) {
                case sf::Keyboard::Key::PageUp:
                    distanceScale = setZoomScale(zoomScale + 1);
                    break;
                case sf::Keyboard::Key::PageDown:
                    distanceScale = setZoomScale(zoomScale - 1);
                    break;
                case sf::Keyboard::Key::B:
                    player.applyImpulse(Vector2d(0, -100));
                    break;
            }
        }
    }
    if (player.getState() == State::DEAD) {
        Vector2d dirInput = Vector2d(0, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            dirInput.x -= 1.0;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            dirInput.x += 1.0;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            dirInput.y -= 1.0;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            dirInput.y += 1.0;
        }
        if (dirInput.lengthSquared() != 0) {
            moveCamera(dirInput * static_cast<double>(CAMERA_SPEED*dt*distanceScale));
        }

    }
    
}

void Game::update(double dt) {
    bool playerActive = player.getState() != State::DEAD;

    double resolution = 1;

    player.parentObject = nullptr; // Reset parentObject to be determined on update
    player.hasCollided = false;

    double delta = dt/resolution;
    for (int i=0; i<resolution; i++) {
        // Loop through each body and update
        for (auto& obj1 : objects) {
            obj1->parentObject = nullptr;
            obj1->hasCollided = false;
            // Update forces between bodies and the player
            for (auto& obj2 : objects) {
                if (&obj1 == &obj2) {
                    continue;
                }
                obj1->updateForces(*obj2, delta);
            }
            if (playerActive) {
                obj1->updateForces(player, delta);
                player.updateForces(*obj1, delta);
            }        
        }

        if (playerActive) player.handleInput(window, dt);

        // Update positions based on velocities
        for (auto& obj : objects) {
            obj->updatePosition(delta);
        }
        if (playerActive) player.updatePosition(delta);

        // If collisions occured, check and fix overlap issues
        for (auto& obj1 : objects) {
            if (obj1->hasCollided) {
                for (auto& obj2 : objects) {
                    if (&obj1 == &obj2) {
                        continue;
                    }
                    obj1->fixOverlap(*obj2);
                }
                if (playerActive) {
                    obj1->fixOverlap(player);
                    player.fixOverlap(*obj1);
                }
                
            }
        }

        if (playerActive) {
            player.update(delta);
            updateRelativeVelocities(player);
            centerCamera(CAMERA_LERP_SPEED);
        }
    }
}

void Game::draw(sf::RenderWindow& window) {
    window.clear();
    for (auto& obj : objects) {
        obj->draw(window, distanceScale);
    }
    if (player.getState() != State::DEAD) {
        player.draw(window, distanceScale);

    }
    window.display();
}

// Initialization ----------------------------------------------------------------------------------


void Game::addBody(Vector2d position, Vector2d velocity, double mass, double radius, sf::Color color) {
    objects.push_back(std::make_unique<Body>(position, velocity, mass, radius, DEFAULT_FRICTION_COEFFICIENT, color));
}


// Updates -----------------------------------------------------------------------------------------

void Game::updateRelativeVelocities(PhysObj& referenceObject) {
    Vector2d refVelocity = referenceObject.velocity;
    for (auto& obj : objects) {
        obj->velocity -= refVelocity;
    }
    player.velocity -= refVelocity;
    globalVelocity -= refVelocity;
}


// Camera ------------------------------------------------------------------------------------------

void Game::centerCamera(float lerpScale) {
    Vector2d targetPos = player.position;
    float t = lerpScale*dt;
    t = std::clamp(t, 0.f, 1.f);
    float lerpX = std::lerp(0.f, targetPos.x, t);
    float lerpY = std::lerp(0.f, targetPos.y, t);
    Vector2d lerpOffset(lerpX, lerpY);
    moveCamera(lerpOffset);    
}

void Game::moveCamera(Vector2d offset) {
    for (auto& obj : objects) {
        obj->position -= offset;
    }
    player.position -= offset;
    globalOrigin += offset;
}

float Game::setZoomScale(int newZoomScale) {
    zoomScale = newZoomScale;
    float newDistanceScale = static_cast<float>(pow(2, zoomScale));
    return newDistanceScale;
}

// MAIN --------------------------------------------------------------------------------------------

int main() {
    Game game(800, 600);
    game.run();
    
    return 0;
}
