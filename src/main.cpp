#include "main.h"


//-----------------------------------------------------------------------
// CONSTANTS

const float CAMERA_SPEED = 100.0;


//-----------------------------------------------------------------------
// VARIABLES



//-----------------------------------------------------------------------
// Game

Game::Game(unsigned int window_w, unsigned int window_h) {
    window.create(sf::VideoMode({window_w, window_h}), "Planet Jumper");
};

void Game::run() {
    sf::Clock clock;
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    addBody(Vector2d(50.f,  10.f), Vector2d(5.0, 0.0),  1000000.0, 10.0, sf::Color(255, 255, 255));
    addBody(Vector2d(100.f, 10.f), Vector2d(-5.0, 0.0), 1000000.0, 10.0, sf::Color(255, 255, 255));

    while (window.isOpen()) {
        
        float dt = clock.restart().asSeconds(); // get deltatime
        dt = 0.016;
        update(dt);
        handleInput(dt);
        draw(window);
        
    }
};

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
            }
        }
    }
    Vector2d dirInput = Vector2d(0.0, 0.0);
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
    moveCamera(dirInput * (CAMERA_SPEED * distanceScale * dt));

};

void Game::update(double dt) {
    bool collisionOccurred = false;
    for (auto& obj1 : objects) {
        for (auto& obj2 : objects) {
            if (&obj1 == &obj2) {
                continue;
            }
            if (obj1->updateForces(*obj2, dt)) {
                collisionOccurred = true;
            }
        }
    }

    for (auto& obj : objects) {
        obj->updatePosition(dt);
    }

    if (collisionOccurred) {
        for (auto& obj1 : objects) {
            for (auto& obj2 : objects) {
                if (&obj1 == &obj2) {
                    continue;
                }
                obj1->fixOverlap(*obj2, dt);
            }
        }
    }
};

void Game::draw(sf::RenderWindow& window) {
    window.clear();
    for (auto& obj : objects) {
        obj->draw(window, distanceScale);
    }

    window.display();
}


void Game::addBody(Vector2d position, Vector2d velocity, double mass, double radius, sf::Color color) {
    objects.push_back(std::make_unique<Body>(position, velocity, mass, radius, color));
}

void Game::moveCamera(Vector2d offset) {
    for (auto& obj : objects) {
        obj->position -= offset;
    }
}

float Game::setZoomScale(int newZoomScale) {
    zoomScale = newZoomScale;
    float newDistanceScale = static_cast<float>(pow(2, zoomScale));
    return newDistanceScale;
}


int main() {
    Game game(800, 600);
    game.run();
    
    return 0;
}
