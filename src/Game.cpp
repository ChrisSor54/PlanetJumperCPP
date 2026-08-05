#include "Game.h"


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

    if (player.getState() != State::DEAD) {
        updateRelativeVelocities(player.velocity);
        updateRelativePositions(player.position);
        rotateCamera(-player.rotation);
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

#pragma endregion

// Spawning ----------------------------------------------------------------------------------------
#pragma region Spawning

Body* Game::addBody(
    Vector2d position, Vector2d velocity,
    double mass, double radius, 
    sf::Angle rotationalVelocity, double surfaceFriction, MatterState state,
    sf::Color color) {
    if ((position - player.position).lengthSquared() < 1.0) {
        player.position.y -= radius;
        player.velocity = velocity;
    }
    objects.push_back(std::make_unique<Body>(position, velocity, mass, radius, rotationalVelocity, surfaceFriction, state, color));
    return static_cast<Body*>(objects.back().get());
}

Body* Game::addBody(
    Vector2d position, Vector2d velocity,
    double mass, double radius, 
    sf::Angle rotationalVelocity, double surfaceFriction, MatterState state,
    sf::Color color, BodyTexture bodyTexture) {
    if ((position - player.position).lengthSquared() < 1.0) {
        player.position.y -= radius;
        player.velocity = velocity;
    }
    objects.push_back(std::make_unique<Body>(position, velocity, mass, radius, rotationalVelocity, surfaceFriction, state, color, bodyTexture));
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
    MatterState state,
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
    return addBody(position, velocity, mass, radius, rotationalVelocity, surfaceFriction, state, color);
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
    MatterState state,
    sf::Color color, BodyTexture satTexture) {

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
    return addBody(position, velocity, mass, radius, rotationalVelocity, surfaceFriction, state, color, satTexture);
}


void Game::teleportPlayerTo(PhysObj* target) {
    Vector2d offset(0, -target->radius-GROUNDED_MARGIN);
    player.position = target->position + offset;
    player.velocity = target->velocity;
    player.rotation = offset.angle() + sf::degrees(90);
    player.fixOverlap(*target);
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

        if (window.hasFocus()) {
            if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {   
                for (auto& [inputAction, keybinds] : inputManager.bindings) {
                    for (auto& key : keybinds)
                        if (key == keyReleased->scancode)
                            inputManager.inputStates[inputAction].released = true;
                        
                }
            }
        }
    }
    if (!window.hasFocus()) return;
    for (auto& [inputAction, state] : inputManager.inputStates) {
        for (auto& key : inputManager.bindings[inputAction])
            if (sf::Keyboard::isKeyPressed(key)) state.pressed = true;
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
                updateRelativePositions(static_cast<Vector2d>(inputManager.directionalInput)*(double)(CAMERA_SPEED*dt*zoomScale));
            }
        } else if (freecamEnabled) {
            if (inputManager.directionalInput.lengthSquared() != 0) {
                Vector2f cameraOffset = inputManager.directionalInput.rotatedBy(globalRotation)*(float)(CAMERA_SPEED*dt);
                moveCamera(cameraOffset*zoomScale, cameraOffset*(float)BACKGROUND_SCROLL_SPEED);
                inputManager.directionalInput = Vector2f(0,0);
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
        if (inputManager.inputStates[InputAction::DEBUG_EnlargePlanets].released) {
            enlargePlanets = !enlargePlanets;
        }
    }
}

#pragma endregion

// Update ------------------------------------------------------------------------------------------
#pragma region Update

void Game::update(double dt) {
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
            updateRelativePositions(player.position, CAMERA_LERP_SPEED);
            if (player.state == State::GROUNDED && copyRotation && !freecamEnabled) {
                updateRelativeRotations(player.rotation, CAMERA_LERP_SPEED);
            }
            //centerCamera(CAMERA_LERP_SPEED, player, false);
            if (freecamEnabled) {
                moveCamera(static_cast<sf::Vector2f>(globalVelocity*dt));
            }
        }
    }
}

void Game::updateRelativePositions(Vector2d newOrigin) {
    for (auto& obj : objects) {
        obj->position -= newOrigin;
    }
    player.position -= newOrigin;
    for (auto& particle : player.smokeArray) {
        if (particle.lifespan <= 0) continue;
        particle.pos -= (sf::Vector2f) newOrigin;
    }
    globalOrigin -= newOrigin;
}

void Game::updateRelativePositions(Vector2d newOrigin, float lerpScale) {
    float t = lerpScale*dt*zoomScale;
    t = std::clamp(t, 0.f, 1.f);
    float lerpX = std::lerp(0.f, newOrigin.x, t);
    float lerpY = std::lerp(0.f, newOrigin.y, t);
    Vector2d lerpOffset(lerpX, lerpY);
    updateRelativePositions(lerpOffset);
}

void Game::updateRelativeRotations(sf::Angle newRotation, float lerpScale) {
    float t = lerpScale*dt*zoomScale;
    t = std::clamp(t, 0.f, 1.f);
    sf::View view = window.getView();
    float targetAngle = newRotation.asDegrees();
    float currentAngle = view.getRotation().asDegrees();
    float diff = std::fmod(targetAngle - currentAngle + 180.f, 360.f);
    if (diff < 0) diff += 360.f;
        diff -= 180.f;
    float lerpA = currentAngle + (diff+90.f) * t;
    rotateCamera(sf::degrees(lerpA));
}

void Game::updateRelativeVelocities(Vector2d newReferenceFrame) {
    for (auto& obj : objects) {
        obj->velocity -= newReferenceFrame;
    }
    player.velocity -= newReferenceFrame;
    for (auto& particle : player.smokeArray) {
        if (particle.lifespan <= 0) continue;
        particle.vel -= (sf::Vector2f) newReferenceFrame;
    }
    globalVelocity -= newReferenceFrame;
}


#pragma endregion

// Drawing -----------------------------------------------------------------------------------------
#pragma region Drawing

void Game::draw(sf::RenderWindow& window) {
    window.clear();
    drawBackground(window, dt);
    player.drawSmoke(window);

    if (drawVelocities) {
        for (auto& obj : objects) {
            if (obj->parentObject) {
                obj->drawOrbitalPath(window, VISUAL_ORBIT_RESOLUTION_SCALE);
            }
        }
    }
    
    for (auto& obj : objects) {
        if (enlargePlanets) {
            obj->draw(window, Vector2f(1.f, 1.f)*(1.f + (float)(500.f/obj->radius)));
        } else {
            obj->draw(window);
        }
        
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
        bgOffset += timeScale*static_cast<sf::Vector2f>(globalVelocity*BACKGROUND_SCROLL_SPEED*dt)/zoomScale;
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

void Game::moveCamera(sf::Vector2f offset, Vector2f backGroundOffset) {
    sf::View view = window.getView();
    view.move(offset);
    window.setView(view);
    bgOffset -= backGroundOffset;
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
