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
        inputManager.globalInputs.inputStates[(InputAction) i] = InputState{false, false};
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
        for (auto& player : players) {
            obj1->fixOverlap(*player);
            player->fixOverlap(*obj1);
        }
    }
    for (auto& player : players) {
        if (player->getState() != State::DEAD) {
            updateRelativeVelocities(player->velocity);
            updateRelativePositions(player->position);
            //rotateCamera(player->id, -player->rotation);
            break;
        }
    }
    updateViews();
    // if (defaultHomePlanet == nullptr) {
    //     defaultHomePlanet = static_cast<Body*>(objects[0].get());
    // }

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

void Game::setDefaultHomePlanet(Body* homePlanet) {
    for (auto& body : objects) {
        if (body.get() == homePlanet) {
            defaultHomePlanet = homePlanet;
            return;
        }
    }
}

#pragma endregion

// Spawning ----------------------------------------------------------------------------------------
#pragma region Spawning


Player* Game::addPlayer(sf::Color playerColor) {
    int playerID = playerCount;
    playerCount += 1;
    cameras[playerID] = Camera{
        sf::View(),
        sf::View(),
        Vector2d(0,0),
        Vector2d(0,0)
    };
    players.push_back(std::make_unique<Player>(playerID, playerColor));
    updateViews();
    return static_cast<Player*>(players.back().get());
}

Player* Game::addPlayer(sf::Color playerColor, Body* homePlanet) {
    Player* player = addPlayer(playerColor);
    teleportPlayerTo(player, homePlanet);
    return player;
}

void Game::removePlayer(int playerID) {
    if (playerCount <= 1) {
        return;
    }
    for (int i=0; i < players.size(); i++) {
        int id = players[i]->playerID;
        if (id == playerID) {
            players.erase(players.begin() + i);
            cameras.erase(playerID);
            playerCount -= 1;
            updateViews();
            return;
        }
    }
}

Body* Game::addBody(
    Vector2d position, Vector2d velocity,
    double mass, double radius, 
    sf::Angle rotationalVelocity, double surfaceFriction, MatterState state,
    sf::Color color) {
    for (auto& player : players) {
        if ((position - player->position).lengthSquared() < 1.0) {
            player->position.y -= radius;
            player->velocity = velocity;
        }
    }
    objects.push_back(std::make_unique<Body>(position, velocity, mass, radius, rotationalVelocity, surfaceFriction, state, color));
    return static_cast<Body*>(objects.back().get());
}

Body* Game::addBody(
    Vector2d position, Vector2d velocity,
    double mass, double radius, 
    sf::Angle rotationalVelocity, double surfaceFriction, MatterState state,
    sf::Color color, BodyTexture bodyTexture) {
    for (auto& player : players) {
        if ((position - player->position).lengthSquared() < 1.0) {
            player->position.y -= radius;
            player->velocity = velocity;
        }
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


void Game::teleportPlayerTo(Player* player, PhysObj* target) {
    Vector2d offset(0, -(target->radius+GROUNDED_MARGIN));
    player->position = target->position + offset.rotatedBy(target->rotation);
    player->velocity = target->velocity;
    player->rotation = offset.angle() + sf::degrees(90);
    //player->fixOverlap(*target);
    cameras[player->playerID].position = player->position;
    updateViews();
}

#pragma endregion

// Input -------------------------------------------------------------------------------------------
#pragma region Input

using IA = InputAction;

void Game::updateInputStates() {
    // Reset input states
    for (auto& [input, state] : inputManager.globalInputs.inputStates) {
        state = InputState{false, false};
    }
    inputManager.globalInputs.directionalInput = Vector2f(0,0);
    for (auto& player : players) {
        int i = player->playerID;
        for (auto& [input, state] : inputManager.playerInputs[i].inputStates) {
            state = InputState{false, false};
        }
        inputManager.playerInputs[i].directionalInput = Vector2f(0,0);
    }

    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            updateViews();
        }

        if (window.hasFocus()) {
            if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {   
                for (auto& [inputAction, keybinds] : inputManager.globalInputs.bindings) {
                    for (auto& key : keybinds)
                        if (key == keyReleased->scancode)
                            inputManager.globalInputs.inputStates[inputAction].released = true;
                        
                }
                for (auto& player : players) {
                    int i = player->playerID;
                    for (auto& [inputAction, keybinds] : inputManager.playerInputs[i].bindings) {
                        for (auto& key : keybinds)
                            if (key == keyReleased->scancode)
                                inputManager.playerInputs[i].inputStates[inputAction].released = true;
                            
                    }
                }
            } 
        }
    }
    if (!window.hasFocus()) return;
    for (auto& [inputAction, state] : inputManager.globalInputs.inputStates) {
        for (auto& key : inputManager.globalInputs.bindings[inputAction])
            if (sf::Keyboard::isKeyPressed(key)) state.pressed = true;
    }
    for (auto& player : players) {
        if (player == nullptr) {continue;}
        int i = player->playerID;
        InputMap& playerInput = inputManager.playerInputs[i];
        for (auto& [inputAction, state] : playerInput.inputStates) {
            for (auto& key : playerInput.bindings[inputAction])
                if (sf::Keyboard::isKeyPressed(key)) state.pressed = true;
        }
        if (playerInput.inputStates[InputAction::Up].pressed) {
            playerInput.directionalInput.y -= 1.0;
        }
        if (playerInput.inputStates[InputAction::Down].pressed) {
            playerInput.directionalInput.y += 1.0;
        }
        if (playerInput.inputStates[InputAction::Left].pressed) {
            playerInput.directionalInput.x -= 1.0;
        }
        if (playerInput.inputStates[InputAction::Right].pressed) {
            playerInput.directionalInput.x += 1.0;
        }
        if (playerInput.directionalInput.lengthSquared() > 0.0) {
            sf::Angle cameraRotation = cameras[i].view.getRotation();
            playerInput.directionalInput = (playerInput.directionalInput.normalized().rotatedBy(cameraRotation));
        }
    }
    
}

void Game::handleInput(double dt) {
    if (!inputPressed(IA::DEBUG)) {
        for (auto& player : players) {
            if (inputReleased(player->playerID, IA::ZoomIn)) {
                zoomCamera(player->playerID, ZOOM_SPEED);
            } else if (inputReleased(player->playerID, IA::ZoomOut)) {
                zoomCamera(player->playerID, 1/ZOOM_SPEED);
            }
        }


        if (inputReleased(IA::SpeedUp)) {
            timeScale *= 2.0;
        } else if (inputReleased(IA::SpeedDown)) {
            timeScale *= 0.5;
        }
        if (inputReleased(IA::ResetTimescale)) {
            timeScale = 1.0;
        }
        if (inputReleased(IA::ToggleRotation)) {
            copyRotation = !copyRotation;
        }
        if (inputReleased(IA::AddPlayer)) {
            if (playerCount < 4) {
                int r = (rand()%156) + 100;
                int g = (rand()%156) + 100;
                int b = (rand()%156) + 100;
                if (defaultHomePlanet == nullptr) {
                    Player* player = addPlayer(sf::Color(r,g,b));
                    player->position = players[0]->position;
                } else {
                    Player* player = addPlayer(sf::Color(r,g,b), defaultHomePlanet);
                }
            }
        }
        if (inputReleased(IA::RemovePlayer)) {
            int playerID = players.back()->playerID;
            if (playerID != 0) {
                removePlayer(playerID);
            }
        }
        bool playerAlive = false;
        for (auto& player : players) {
            if (player->getState() != State::DEAD) {
                playerAlive = true;
                // int rotationInput = inputPressed(player->id, IA::RotateR) - inputPressed(player->id, IA::RotateL);
                // if (player->getState() != State::GROUNDED && rotationInput != 0) {
                //     sf::Angle currentRotation = cameraViews[player->id].getRotation();
                //     rotateCamera(player->id, currentRotation + sf::degrees(2)*rotationInput*dt*CAMERA_ROTATE_SPEED);
                // }
            }
        }
        // if (!playerAlive) {

        // } 
        // else if (freecamEnabled) {
        //     if (inputManager.directionalInput.lengthSquared() != 0) {
        //         Vector2f cameraOffset = inputManager.directionalInput.rotatedBy(globalRotation)*(float)(CAMERA_SPEED*dt);
        //         moveCamera(0, cameraOffset*zoomScale, cameraOffset*(float)BACKGROUND_SCROLL_SPEED);
        //         inputManager.directionalInput = Vector2f(0,0);
        //     }
        // }
  
    } else { // Debug
        if (inputReleased(IA::DEBUG_ShowVelocities)) {
            drawVelocities = !drawVelocities;
        }
    }
}


bool Game::inputPressed(InputAction input) {
    return inputManager.globalInputs.inputStates[input].pressed;
}

bool Game::inputPressed(int playerID, InputAction input) {
    return inputManager.playerInputs[playerID].inputStates[input].pressed;
}

bool Game::inputReleased(InputAction input) {
    return inputManager.globalInputs.inputStates[input].released;
}

bool Game::inputReleased(int playerID, InputAction input) {
    return inputManager.playerInputs[playerID].inputStates[input].released;
}

#pragma endregion

// Update ------------------------------------------------------------------------------------------
#pragma region Update

void Game::update(double dt) {
    //bool playerActive = player.getState() != State::DEAD;
    for (auto& player: players) {
        player->parentObject = nullptr; // Reset parentObject to be determined on update
        player->hasCollided = false;
        player->isGrounded = false;
    }
    
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
            for (auto& player : players) {
                if (player->getState() == State::DEAD) continue;
                obj1->updateGravity(*player, delta);
                player->updateGravity(*obj1, delta);     
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
                for (auto& player : players) {
                    if (player->getState() == State::DEAD) continue;
                    obj1->updateCollision(*player, delta);
                    player->updateCollision(*obj1, delta);     
                }
            }
            for (auto& obj : objects) {
                obj->updateVelocity();
            }
            for (auto& player : players) {
                player->updateVelocity();
            }
        }

        globalOrigin += globalVelocity*delta;

        for (auto& player : players) {
            if (player->getState() == State::DEAD) continue;
            player->update(delta);
            player->handleInput(inputManager.playerInputs[player->playerID], delta);     
        }

        // Update positions based on velocities
        for (auto& obj : objects) {
            obj->updatePosition(delta);
        }
        for (auto& player : players) {
            if (player->getState() == State::DEAD) continue;
            player->updatePosition(delta);  
        }

        // If collisions occured, check and fix overlap issues
        for (auto& obj1 : objects) {
            if (obj1->hasCollided) {
                for (auto& obj2 : objects) {
                    if (&obj1 == &obj2) {
                        continue;
                    }
                    obj1->fixOverlap(*obj2);
                }
                for (auto& player : players) {
                    if (player->getState() == State::DEAD) continue;
                    obj1->fixOverlap(*player);
                    player->fixOverlap(*obj1);  
                }
            }
        }
        for (auto& player : players) {
            player->updateSmoke(delta);  
        }
        //std::cout << std::to_string((player.velocity - player.parentObject->velocity).length()) << std::endl;
        Vector2d relativeOrigin(0,0);
        Vector2d relativeVelocityOrigin(0,0);
        int alivePlayerCount = 0;
        for (auto& player : players) {
            if (player->getState() != State::DEAD) {
                relativeOrigin += player->position;
                relativeVelocityOrigin += player->velocity;
                alivePlayerCount += 1;
            }
                // if (player->getState() == State::GROUNDED && copyRotation && !freecamEnabled) {
                //     updateRelativeRotations(player->id, player->rotation);
                // } else if (freecamEnabled) {
                //     if (freecamEnabled) {
                //         moveCamera(player->id, globalVelocity*dt);
                //     }
                // }
        }
        relativeOrigin /= (double)alivePlayerCount;
        relativeVelocityOrigin /= (double)alivePlayerCount;
        updateRelativePositions(relativeOrigin);
        updateRelativeVelocities(relativeVelocityOrigin);
    }
    for (auto& player : players) {
        Camera& camera = cameras[player->playerID];
        centerCamera(player->playerID);
        // Vector2d relVel = camera.velocity - player->velocity;
    }
    if (drawVelocities) {
        for (auto& obj : objects) {
            obj->updateOrbitalPath(VISUAL_ORBIT_RESOLUTION_SCALE);
        }
        for (auto& player : players) {
            player->updateOrbitalPath(VISUAL_ORBIT_RESOLUTION_SCALE);
        }
    }
}

void Game::updateRelativePositions(Vector2d newOrigin) {
    for (auto& obj : objects) {
        obj->position -= newOrigin;
    }
    for (auto& player : players) {
        player->position -= newOrigin;
        
        for (auto& particle : player->smokeArray) {
            if (particle.lifespan <= 0) continue;
            particle.pos -= (sf::Vector2f) newOrigin;
        }
    }
    for (auto& [cameraID, camera] : cameras) {
        camera.position -= newOrigin;
    }
    globalOrigin -= newOrigin;
}

void Game::updateRelativeRotations(int cameraID, sf::Angle newRotation) {
    //rotateCamera(cameraID, newRotation);
}

void Game::updateRelativeVelocities(Vector2d newReferenceFrame) {
    for (auto& obj : objects) {
        obj->velocity -= newReferenceFrame;
    }
    for (auto& player : players) {
        player->velocity -= newReferenceFrame;
        for (auto& particle : player->smokeArray) {
            if (particle.lifespan <= 0) continue;
            particle.vel -= (sf::Vector2f) newReferenceFrame;
        }
    }
    for (auto& [cameraID, camera] : cameras) {
        camera.velocity -= newReferenceFrame;
    }
    globalVelocity -= newReferenceFrame;
}


#pragma endregion

// Drawing -----------------------------------------------------------------------------------------
#pragma region Drawing

void Game::draw(sf::RenderWindow& window) {
    window.clear();
    for (auto& player : players) {
        int i = player->playerID;
        window.setView(cameras[i].view);
        drawBackground(i, window, dt);
        //cameraViews[player->id].setCenter(static_cast<Vector2f>(player->position));
        for (auto& player : players) {
            player->drawSmoke(window);
        }

        if (drawVelocities) {
            for (auto& obj : objects) {
                if (obj->parentObject) {
                    obj->drawOrbitalPath(window);
                }
            }
            //player->drawOrbitalPath(window);
            // for (auto& player : players) {
            //     player->drawOrbitalPath(window);
            // }
        }
        
        for (auto& obj : objects) {
            obj->draw(window);
        }
        for (auto& player : players) {
            if (player->playerID == i) continue;
            if (player->getState() != State::DEAD) {
                player->draw(window);
            }
        }
        players[i]->draw(window);
        drawUI(window, i);
    }
    if (playerCount < 4) {
        window.setView(globalUIView);
        Vector2f promptPos;
        switch(playerCount) {
            case 1:
            case 2:
                promptPos = {static_cast<float>((globalUIView.getSize().x-(18.0*16.0))), 0.f};
                break;
            case 3:
                promptPos = {static_cast<float>((globalUIView.getSize().x-(18.0*8.0))*.75), static_cast<float>((globalUIView.getSize().y-8.0)*.75)};
                break;
        }
        drawString(window, "Add Player:    +\nRemove Player: -", promptPos, {2.0, 2.0}, 0.25, false);
    }
    window.display();
}

void Game::drawBackground(int cameraID, sf::RenderWindow& window, double dt) {
    Camera& camera = cameras[cameraID];
    camera.bgOffset += timeScale*static_cast<sf::Vector2f>((players[cameraID]->velocity - globalVelocity)*BACKGROUND_SCROLL_SPEED*dt)/camera.zoomScale;
    sf::Vector2u winSize = window.getSize();
    sf::FloatRect viewport = camera.view.getViewport();
    int windowX = 2*static_cast<int>(viewport.size.x * winSize.x);
    int windowY = 2*static_cast<int>(viewport.size.y * winSize.y);
    int windowM = std::max(windowX, windowY);
    int offsetX = camera.bgOffset.x;
    int offsetY = camera.bgOffset.y;
    bgSprite.setTextureRect(sf::IntRect({offsetX, offsetY}, {windowM, windowM}));
    
    bgSprite.setOrigin(sf::Vector2f(windowM/2, windowM/2));
    bgSprite.setPosition(static_cast<sf::Vector2f>(camera.position));
    bgSprite.setScale(sf::Vector2f(1.0, 1.0)*camera.zoomScale);
    window.draw(bgSprite);
}


void Game::drawUI(sf::RenderWindow& window, int playerID) {
    window.setView(cameras[playerID].uiView);
    auto& bindings = inputManager.playerInputs[playerID].bindings;

    auto keyName = [&](IA action) -> std::string {
        auto it = bindings.find(action);
        if (it == bindings.end() || it->second.empty()) return "?";
        std::string s = sf::Keyboard::getDescription(it->second[0]).toAnsiString();
        for (char& c : s) c = std::toupper(static_cast<unsigned char>(c));
        return s;
    };

    std::string moveInputs = fmt::format("{}/{}/{}/{}",
        keyName(IA::Up), keyName(IA::Left), keyName(IA::Down), keyName(IA::Right));
    std::string helpText = fmt::format("MOVE: {}\nCHARGE JUMP: {}\nDECELERATE/WALK: {}\nZOOM IN/OUT: {}/{}",
        moveInputs, keyName(IA::Jump), keyName(IA::Walk), keyName(IA::ZoomIn), keyName(IA::ZoomOut));

    float xScale = (playerCount > 2) ? 0.75 : 1.0;
    // int rows = (playerCount == 2) ? 2 : (playerCount+1)/2;
    // float yScale = (rows >= 2) ? (1.f/(float)rows) : 1.f;
    float yScale = xScale;
    drawString(window, helpText, {0.f, 0.f}, {static_cast<float>(2.0*xScale), static_cast<float>(2.0*yScale)}, 0.25, false);
}

sf::Texture Game::bigFontSpritesheet;
sf::Texture Game::smallFontSpritesheet;
bool Game::fontSpritesheetLoaded = false;

void Game::drawString(sf::RenderTarget& target, sf::String string, sf::Vector2f position, sf::Vector2f scale, float linePadding, bool useBigFont) {
    if (!fontSpritesheetLoaded) {
        if (!bigFontSpritesheet.loadFromFile("assets/SpaceFont_big.png")) {
            throw std::invalid_argument("Bad large font texture");
        }
        if (!smallFontSpritesheet.loadFromFile("assets/SpaceFont_small.png")) {
            throw std::invalid_argument("Bad small font texture");
        }
        fontSpritesheetLoaded = true;
    }
    sf::Texture fontTexture = (useBigFont) ? bigFontSpritesheet : smallFontSpritesheet;
    sf::Vector2i cellSize = (useBigFont) ? sf::Vector2i(16,16) : sf::Vector2i(8,8);

    sf::Sprite charSprite = sf::Sprite(fontTexture);
    float x = position.x;
    // charSprite.setPosition(position);
    // target.draw(charSprite);

    for (unsigned char c : string) {
        if (c == '\n') { x = position.x; position.y += cellSize.y*scale.y + linePadding; continue; }

        int i = std::toupper(c)-32;  // atlas starts at ASCII 32 (space)
        charSprite.setTextureRect(sf::IntRect({(i % 13) * cellSize.x, (i / 13) * cellSize.y}, cellSize));
        charSprite.setPosition({x, position.y});
        charSprite.setScale(scale);
        target.draw(charSprite);

        x += cellSize.x*scale.x;
    }
}

#pragma endregion

// Camera ------------------------------------------------------------------------------------------
#pragma region Cameras and Views


void Game::updateViews() {
    sf::Vector2f winSize(window.getSize());
    int cols = (playerCount > 2) ? 2 : 1;
    int rows = (playerCount == 2) ? 2 : (playerCount+1)/2;

    sf::Vector2f viewSize(winSize.x / cols, winSize.y / rows);
    sf::Vector2f viewportSize(1.f / cols, 1.f / rows);

    for (auto& player : players) {
        int i = player->playerID;
        sf::FloatRect viewport(
            {(i % cols) * viewportSize.x, (i / cols) * viewportSize.y},
            viewportSize);

        cameras[i].view.setSize(viewSize);
        cameras[i].view.zoom(cameras[i].zoomScale);
        cameras[i].view.setViewport(viewport);        

        cameras[i].uiView.setSize(viewSize);
        cameras[i].uiView.setCenter(viewSize / 2.f);
        cameras[i].uiView.setViewport(viewport);
    }
    globalUIView.setSize(winSize);
    globalUIView.setCenter({winSize.x/2.0, winSize.y/2.0});
}

void Game::moveCamera(int cameraID, Vector2d offset) {
    cameras[cameraID].position -= offset;
}

void Game::moveCamera(int cameraID, Vector2d offset, Vector2f backGroundOffset) {
    moveCamera(cameraID, offset);
    cameras[cameraID].bgOffset -= backGroundOffset;
}

void Game::centerCamera(int playerID) {
    cameras[playerID].position = players[playerID]->position;
    cameras[playerID].view.setCenter((Vector2f)cameras[playerID].position);
    if (players[playerID]->getState() == State::GROUNDED) {
        rotateCamera(playerID, players[playerID]->rotation, CAMERA_ROTATE_SPEED);
    }
}

void Game::centerCamera(int playerID, float lerpScale) {
    float t = lerpScale*dt*timeScale;
    t = std::clamp(t, 0.f, 1.f);
    Vector2d playerVelocity = players[playerID]->velocity - globalVelocity;
    Vector2d cameraVelocity = cameras[playerID].velocity - globalVelocity;
    Vector2f relativeVelocity = static_cast<Vector2f>(cameraVelocity - playerVelocity);
    //Vector2d relativePosition = playerPosition - cameraPosition;
    float lerpX = std::lerp((float)cameraVelocity.x, (float)playerVelocity.x, t);
    float lerpY = std::lerp((float)cameraVelocity.y, (float)playerVelocity.y, t);
    Vector2d velocityLerpOffset(lerpX, lerpY);
    cameras[playerID].velocity = velocityLerpOffset;
    // Vector2f playerPosition = static_cast<Vector2f>(players[playerID]->position - globalOrigin);
    // Vector2f cameraPosition = static_cast<Vector2f>(cameras[playerID].position - globalOrigin);
    // //Vector2d relativePosition = playerPosition - cameraPosition;
    // lerpX = std::lerp(cameraPosition.x, playerPosition.x, t);
    // lerpY = std::lerp(cameraPosition.y, playerPosition.y, t);
    // Vector2d positionLerpOffset(lerpX, lerpY);
    cameras[playerID].position = players[playerID]->position - (cameras[playerID].velocity*(double)dt);
    playerVelocity = players[playerID]->velocity - globalVelocity;
    cameraVelocity = cameras[playerID].velocity - globalVelocity;
    //relativeVelocity = static_cast<Vector2f>(cameraVelocity - playerVelocity);
    cameras[playerID].view.setCenter((Vector2f)cameras[playerID].position);
    if (players[playerID]->getState() == State::GROUNDED) {
        rotateCamera(playerID, players[playerID]->rotation, CAMERA_ROTATE_SPEED);
    }
}

void Game::rotateCamera(int cameraID, sf::Angle targetAngle) {
    cameras[cameraID].view.setRotation(targetAngle + sf::degrees(90));
}

void Game::rotateCamera(int cameraID, sf::Angle targetAngle, float lerpScale) {
    float t = lerpScale*dt*zoomScale;
    t = std::clamp(t, 0.f, 1.f);
    float currentAngle = cameras[cameraID].view.getRotation().asDegrees();
    float diff = std::fmod(targetAngle.asDegrees() - currentAngle + 180.f, 360.f);
    if (diff < 0) diff += 360.f;
        diff -= 180.f;
    float lerpA = currentAngle + (diff+90.f) * t;
    cameras[cameraID].view.setRotation(sf::degrees(lerpA));
}

void Game::zoomCamera(int cameraID, float zoomValue) {
    if (cameras[cameraID].zoomScale*zoomValue < MIN_ZOOM_SCALE) {
        return;
    }
    cameras[cameraID].zoomScale *= zoomValue;
    cameras[cameraID].view.zoom(zoomValue);
}

#pragma endregion
