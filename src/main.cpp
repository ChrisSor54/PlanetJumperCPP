#include "main.h"




//--------------------------------------------------------------------------------------------------
// Game

// Initialization ----------------------------------------------------------------------------------
#pragma region Initialization

Game::Game(unsigned int window_w, unsigned int window_h) 
    : bgSprite(bgTexture) {
    window.create(sf::VideoMode({window_w, window_h}), "Planet Jumper");
    if (!bgTexture.loadFromFile("assets/background.png")) {
        throw std::invalid_argument("Bad background texture");
    }
    bgTexture.setRepeated(true);
    bgSprite.setTexture(bgTexture);

    for (int i=0; i<(int)InputAction::COUNT; i++) {
        inputManager.inputStates[(InputAction) i] = InputState{false, false};
    }
}

void Game::run() {
    sf::Clock clock;
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    // Planetary System Initialization

    //addBody(Vector2d(100.f, 0.f), Vector2d(0, 0),   14000000.0, 100.0, sf::Color(255, 255, 255));  
    //addBody(Vector2d(-500.f, 0.f), Vector2d(0, 120), 1000000.0, 20.0, sf::Color(255, 255, 255));


    // Moon C1
    // Body* moonC1 = addBody(
    //     Vector2d(-34000, -87000.0),
    //     Vector2d(487.95 + 244.95, 0),
    //     20000,
    //     300.0,
    //     0.7,
    //     sf::Color(64, 63, 62));

    // // Moon C2
    // Body* moonC2 = addBody(
    //     Vector2d(-34000, -82000.0),
    //     Vector2d(487.95 - 300, 0),
    //     5000,
    //     40.0,
    //     0.2,
    //     sf::Color(255, 157, 59));
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
        dt = std::min(dt, 0.3f);
        //std::cout << dt << std::endl;
        //dt = 0.016;
        //dt = 1.0;
        updateInputStates();
        handleInput(dt);
        update(dt);
        draw(window);
        
    }
}

#pragma enregion

// Spawning ----------------------------------------------------------------------------------------
#pragma region Spawning

Body* Game::addBody(Vector2d position,Vector2d velocity, double mass, double radius, sf::Angle rotationalVelocity, double surfaceFriction, sf::Color color, bool drawTexture) {
    if ((position - player.position).lengthSquared() < 1.0) {
        player.position.y -= radius;
        player.velocity = velocity;
    }
    objects.push_back(std::make_unique<Body>(position, velocity, mass, radius, rotationalVelocity, surfaceFriction, color, drawTexture));
    return static_cast<Body*>(objects.back().get());
}

Body* Game::addSatellite(
    PhysicsObject* parent,
    sf::Angle angle,
    double semiMajorAxis,
    double eccentricity,
    bool antiClockwise,
    double mass,
    double radius,
    sf::Angle rotationalVelocity,
    double surfaceFriction,
    sf::Color color) {

    if (eccentricity < 0 || eccentricity > 1.0) {
        throw std::invalid_argument("Error, Eccentricity must be between 0 and 1");
    }
    
    double M = parent->mass;
    if (mass > M) {
        std::cout << "Warning, mass is greater than parent mass!" << std::endl;
    }
    double r = semiMajorAxis*(1 + eccentricity);
    double periapsis = semiMajorAxis*(1 - eccentricity);
    //double semiMinorAxis = semiMajorAxis*sqrt(1 - pow(eccentricity, 2));
    double combinedRadii = radius + parent->radius;
    if (r < combinedRadii) {
        throw std::invalid_argument("Error, SemiMajorAxis is too small");
    }
    if (periapsis < combinedRadii) {
        std::cout << "Warning, periapsis collides with parent body radius!" << std::endl;
    }
    double targetVelocity = sqrt(G*M*(2/r - 1/semiMajorAxis));
    if (antiClockwise) targetVelocity *= -1.0;
    Vector2d position = parent->position + Vector2d(r, 0).rotatedBy(angle);
    Vector2d velocity = parent->velocity + Vector2d(0, targetVelocity).rotatedBy(angle);
    return addBody(position, velocity, mass, radius, rotationalVelocity, surfaceFriction, color, true);
}

#pragma endregion

// Input -------------------------------------------------------------------------------------------
#pragma region Input

void Game::updateInputStates() {
    // Reset input states
    for (auto& [input, state] : inputManager.inputStates) {
        state = InputState{false, false};
    }
    inputManager.directionalInput = sf::Vector2f(0,0);

    if (!window.hasFocus()) return;

    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::View view(sf::FloatRect(
                {0.f, 0.f},
                {static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)}
            ));
            view.zoom(zoomScale);
            window.setView(view);
        } 

        if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {   
            for (auto& [inputAction, key] : inputManager.bindings) {
                if (key == keyReleased->code) {
                    inputManager.inputStates[inputAction].released = true;
                }
            } 
        }     
    }
    for (auto& [inputAction, state] : inputManager.inputStates) {
        state.pressed = sf::Keyboard::isKeyPressed(inputManager.bindings[inputAction]);
    }
    if (inputManager.inputStates[InputAction::Up].pressed) {
        inputManager.directionalInput.y -= 1.0;
    }
    if (inputManager.inputStates[InputAction::Down].pressed) {
        inputManager.directionalInput.y += 1.0;
    }
    if (inputManager.inputStates[InputAction::Left].pressed) {
        inputManager.directionalInput.x -= 1.0;
    }
    if (inputManager.inputStates[InputAction::Right].pressed) {
        inputManager.directionalInput.x += 1.0;
    }
    if (inputManager.directionalInput.lengthSquared() != 0)
        inputManager.directionalInput = inputManager.directionalInput.normalized();
}

void Game::handleInput(double dt) {
    if (!inputManager.inputStates[InputAction::DEBUG].pressed) {
        if (inputManager.inputStates[InputAction::ZoomIn].released) {
            zoomCamera(2.0);
        } else if (inputManager.inputStates[InputAction::ZoomOut].released) {
            zoomCamera(0.5);
        }
    
        if (inputManager.inputStates[InputAction::ToggleFreecam].released) {
            freecamEnabled = !freecamEnabled;
            sf::View view = window.getView();
            float windowWidth = window.getSize().x;
            float windowHeight = window.getSize().y;
            view.setCenter({windowWidth/2.f, windowHeight/2.f});
            window.setView(view);
        }
        if (player.getState() == State::DEAD) {
            if (inputManager.directionalInput.lengthSquared() != 0) {
                moveGlobalPositions(static_cast<Vector2d>(inputManager.directionalInput) * (double)(CAMERA_SPEED*dt*zoomScale));
            }
        } else if (freecamEnabled) {
            if (inputManager.directionalInput.lengthSquared() != 0) {
                moveCamera(inputManager.directionalInput.rotatedBy(globalRotation) * (float)(CAMERA_SPEED*dt*zoomScale));
                inputManager.directionalInput = sf::Vector2f(0,0);
            }
        }
        if (inputManager.inputStates[InputAction::SpeedUp].released) {
            timeScale *= 2.0;
        } else if (inputManager.inputStates[InputAction::SpeedDown].released) {
            timeScale *= 0.5;
        }
        if (inputManager.inputStates[InputAction::ResetTimeScale].released) {
            timeScale = 1.0;
        }

        if (inputManager.inputStates[InputAction::ToggleRotation].released) {
            copyRotation = !copyRotation;
        }
        
    } else { // Debug
        if (inputManager.inputStates[InputAction::DEBUG_ShowVelocities].released) {
            drawVelocities = !drawVelocities;
        }
        if (inputManager.inputStates[InputAction::DEBUG_ToggleReference].released) {
            useParentAsReference = !useParentAsReference;
        }
    }
    
}

#pragma endregion

// Update ------------------------------------------------------------------------------------------
#pragma region Update

void Game::update(double dt) {

    // player.setState(State::DEAD);
    bool playerActive = player.getState() != State::DEAD;

    player.parentObject = nullptr; // Reset parentObject to be determined on update
    player.hasCollided = false;
    player.isGrounded = false;
    double delta = dt;
    int resolution = 1;

    if (timeScale < 1) {
        delta *= timeScale;
    } else if (timeScale > 1) {
        double intPart;
        delta += (int) std::modf(timeScale, &intPart);
        resolution = (int) intPart;
    }
    //std::cout << std::to_string(resolution) << " | " << std::to_string(delta) << std::endl;
    for (int i=0; i<resolution; i++) {
        // Loop through each body and update
        for (auto& obj1 : objects) {
            obj1->parentObject = nullptr;
            obj1->hasCollided = false;
            obj1->isGrounded = false;
            // Update gravitational forces between bodies and the player
            for (auto& obj2 : objects) {
                if (&obj1 == &obj2) {
                    continue;
                }
                obj1->updateGravity(*obj2, delta);
            }
            if (playerActive) {
                obj1->updateGravity(player, delta);
                player.updateGravity(*obj1, delta);
            }        
        }

        for (int i=0; i<COLLISION_RESOLUTION; i++) {
            // Loop through each body and update
            for (auto& obj1 : objects) {
                // Update collisions between bodies and the player
                for (auto& obj2 : objects) {
                    if (&obj1 == &obj2) {
                        continue;
                    }
                    obj1->updateCollision(*obj2, delta);
                }
                if (playerActive) {
                    obj1->updateCollision(player, delta);
                    player.updateCollision(*obj1, delta);
                }        
            }
            for (auto& obj : objects) {
                obj->updateVelocity();
            }
            player.updateVelocity();
        }

        globalOrigin += globalVelocity*dt;

        if (playerActive) {
            player.update(delta);
            player.handleInput(inputManager, window, dt);
        }

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
        player.updateSmoke(dt);
        //std::cout << std::to_string((player.velocity - player.parentObject->velocity).length()) << std::endl;
        if (playerActive) {
            updateRelativeVelocities(player.velocity);
            updateRelativePositions(CAMERA_LERP_SPEED, player.position);
            if (player.getState() == State::GROUNDED && copyRotation && !freecamEnabled) {
                updateRelativeRotations(CAMERA_LERP_SPEED, player.rotation);
            }
            //centerCamera(CAMERA_LERP_SPEED, player, false);
            if (freecamEnabled) {
                moveCamera(static_cast<sf::Vector2f>(globalVelocity*dt));
            }
        }
    }
}

void Game::updateRelativePositions(float lerpScale, Vector2d newCenter) {
    float t = lerpScale*dt*zoomScale;
    t = std::clamp(t, 0.f, 1.f);
    float lerpX = std::lerp(0.f, newCenter.x, t);
    float lerpY = std::lerp(0.f, newCenter.y, t);
    Vector2d lerpOffset(lerpX, lerpY);
    moveGlobalPositions(lerpOffset);
}

void Game::updateRelativeRotations(float lerpScale, sf::Angle newRotation) {
    float t = lerpScale*dt*zoomScale;
    t = std::clamp(t, 0.f, 1.f);
    sf::View view = window.getView();
    float targetAngle = newRotation.asDegrees();
    float currentAngle = view.getRotation().asDegrees();
    float diff = std::fmod(targetAngle - currentAngle + 180.f, 360.f);
    if (diff < 0) diff += 360.f;
        diff -= 180.f;
    float lerpA = currentAngle + (diff+90) * t;
    rotateCamera(sf::degrees(lerpA));
}

void Game::moveGlobalPositions(Vector2d offset) {
    for (auto& obj : objects) {
        obj->position -= offset;
    }
    player.position -= offset;
    for (auto& particle : player.smokeArray) {
        if (particle.lifespan <= 0) continue;
        particle.pos -= (sf::Vector2f) offset;
    }
    globalOrigin -= offset;
}

void Game::updateRelativeVelocities(Vector2d newCenterVelocity) {
    for (auto& obj : objects) {
        obj->velocity -= newCenterVelocity;
    }
    player.velocity -= newCenterVelocity;
    for (auto& particle : player.smokeArray) {
        if (particle.lifespan <= 0) continue;
        particle.vel -= (sf::Vector2f) newCenterVelocity;
    }
    globalVelocity -= newCenterVelocity;
}


#pragma endregion

// Drawing -----------------------------------------------------------------------------------------
#pragma region Drawing

void Game::draw(sf::RenderWindow& window) {
    window.clear();
    drawBackground(window, dt);
    player.drawSmoke(window);
    for (auto& obj : objects) {
        if (drawVelocities && obj->parentObject) obj->drawOrbitalPath(window, VISUAL_ORBIT_RESOLUTION_SCALE);
        obj->draw(window);
    }
    if (player.getState() != State::DEAD) {
        if (drawVelocities && player.parentObject) player.drawOrbitalPath(window, VISUAL_ORBIT_RESOLUTION_SCALE);
        player.draw(window);
    }
    if (drawVelocities) {
        Vector2d referenceVelocity = globalVelocity;
        if (player.getState() != State::DEAD) referenceVelocity = player.velocity;
        for (auto& obj : objects) {
            Vector2d thisReferenceVelocity = referenceVelocity;
            if (useParentAsReference && obj->parentObject) thisReferenceVelocity = obj->parentObject->velocity;
            // obj->drawVelocity(window, thisReferenceVelocity, DEBUG_VELOCITY_SCALE);
        }
        if (player.getState() != State::DEAD) {
            Vector2d thisReferenceVelocity = referenceVelocity;
            if (useParentAsReference && player.parentObject) thisReferenceVelocity = player.parentObject->velocity;
            player.drawVelocity(window, thisReferenceVelocity, DEBUG_VELOCITY_SCALE);
            
        }
    }

    window.display();
}

void Game::drawBackground(sf::RenderWindow& window, double dt) {
    sf::View view = window.getView();
    if (!freecamEnabled) {
        bgOffset += static_cast<sf::Vector2f>(globalVelocity*BACKGROUND_SCROLL_SPEED*dt)/zoomScale;
    }
    int windowX = static_cast<int>(window.getSize().x);
    int windowY = static_cast<int>(window.getSize().y);
    int windowM = std::max(windowX, windowY);
    int offsetX = -bgOffset.x;
    int offsetY = -bgOffset.y;
    bgSprite.setTextureRect(sf::IntRect({offsetX, offsetY}, {2*windowX, 2*windowY}));
    
    bgSprite.setOrigin(sf::Vector2f(windowX, windowY));
    bgSprite.setPosition(view.getCenter());
    bgSprite.setScale(sf::Vector2f(1.0, 1.0) * zoomScale);
    window.draw(bgSprite);
}

#pragma endregion

// Camera ------------------------------------------------------------------------------------------
#pragma region Camera

void Game::moveCamera(sf::Vector2f offset) {
    sf::View view = window.getView();
    view.move(offset);
    window.setView(view);
}

void Game::rotateCamera(sf::Angle targetAngle) {
    sf::View view = window.getView();
    view.setRotation(targetAngle);
    globalRotation = targetAngle;
    window.setView(view);
}

void Game::zoomCamera(float zoomValue) {
    if (zoomScale*zoomValue < MIN_ZOOM_SCALE) {
        return;
    }
    zoomScale *= zoomValue;
    sf::View view = window.getView();
    view.zoom(zoomValue);
    window.setView(view);
}

#pragma endregion


// MAIN --------------------------------------------------------------------------------------------
#pragma region Main

int main() {
    Game game(800, 600);

    // Star A
    Body* starA = game.addBody(
        Vector2d(-50000.f, 0.f), // Position
        Vector2d(0, 0), // Velocity
        5*pow(10, 11), // Mass
        20000.0, // Radius
        sf::degrees(0), // Rotational Velocity
        0.1, // Surface Friction
        sf::Color(251, 255, 148), // Color
        false // Draw body texture
    ); 

    // Planet A
    Body* planetA = game.addSatellite(
        starA, // Parent Body
        sf::degrees(0), // Angle
        50000, // SemiMajorAxis
        0.0, // Eccentricity
        false, // CounterClockwise orbit
        8*pow(10, 7), // Mass
        450, // Radius
        sf::degrees(2),
        0.8, // Surface Friction
        sf::Color(168, 168, 162) // Color
    ); 

    // Moon A1
    Body* moonA1 = game.addSatellite(
        planetA,
        sf::degrees(180),
        700,
        0.0,
        false,
        5*pow(10, 5),
        40.0,
        sf::degrees(10),
        0.2,
        sf::Color(100, 130, 88)
    );

    // Planet B
    Body* planetB = game.addSatellite(
        starA,
        sf::degrees(0),
        35000,
        0,
        false,
        65*pow(10, 5),
        100.0,
        sf::radians(0.108),
        0.8,
        sf::Color(21, 24, 79)
    );

    // Planet C
    Body* planetC = game.addSatellite(
        starA,
        sf::degrees(-90),
        84000,
        0,
        false,
        18*pow(10, 7),
        1000.0,
        sf::degrees(0),
        0.1,
        sf::Color(150, 140, 126)
    );


    game.run();
    
    return 0;
}
#pragma endregion
