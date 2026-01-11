#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iostream>
#include <fstream>  // For file handling to save high score

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

constexpr float GRAVITY = 900.f;
constexpr float JUMP = -350.f;
constexpr float PIPE_SPEED = 300.f;
constexpr float PIPE_GAP = 220.f;

enum GameState {
    SPLASH,
    MENU,
    GAME,
    GAME_OVER,
    HIGHSCORE
};

struct Pipe {
    sf::Sprite top;
    sf::Sprite bottom;
    bool passed = false; // Flag to check if the bird has passed this pipe
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "FlapDash");
    window.setFramerateLimit(60);

    //LOAD TEXTURES
    sf::Texture birdTex, pipeTex, bgTex, splashTex, titleTex, startBtnTex, highScoreBtnTex;
    if (!birdTex.loadFromFile("bird.png")) return -1;
    if (!pipeTex.loadFromFile("pipe.png")) return -1;
    if (!bgTex.loadFromFile("background.png")) return -1;
    if (!splashTex.loadFromFile("splash.png")) return -1;
    if (!titleTex.loadFromFile("title 2.png")) return -1; // Load title image
    if (!startBtnTex.loadFromFile("start_button.png")) return -1;
    if (!highScoreBtnTex.loadFromFile("highscore_button.png")) return -1;

    //FONT
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) return -1; // Ensure you have a font file

    //SPLASH SCREEN
    sf::Sprite splash(splashTex);
    splash.setScale(
        float(WIDTH) / splashTex.getSize().x,
        float(HEIGHT) / splashTex.getSize().y
    );

    //BACKGROUND
    sf::Sprite background(bgTex);
    background.setScale(
        float(WIDTH) / bgTex.getSize().x,
        float(HEIGHT) / bgTex.getSize().y
    );

    //TITLE IMAGE
    sf::Sprite title(titleTex);
    title.setPosition(WIDTH / 2.f - title.getLocalBounds().width / 2.f, HEIGHT / 50.f); // Position slightly lower than top

    //BIRD
    sf::Sprite bird(birdTex);
    bird.setPosition(200.f, HEIGHT / 2.f);
    bird.setScale(0.6f, 0.6f); // Increased bird size

    float velocity = 0.f;
    bool gameOver = false;
    bool gameStarted = false;
    bool showSplash = true;

    //BUTTONS
    sf::RectangleShape startButton(sf::Vector2f(200.f, 60.f));
    startButton.setPosition(WIDTH / 4.f - startButton.getSize().x / 2.f, HEIGHT / 2.f + 100.f);
    startButton.setTexture(&startBtnTex);

    sf::RectangleShape highScoreButton(sf::Vector2f(200.f, 75.f));
    highScoreButton.setPosition(WIDTH * 3.f / 4.f - highScoreButton.getSize().x / 2.f, HEIGHT / 2.f + 92.f);
    highScoreButton.setTexture(&highScoreBtnTex);

    int currentScore = 0;
    int highScore = 0;

    // Load high score from file, if file doesn't exist, create it with a default value
    std::ifstream highScoreFile("highscore.txt");
    if (highScoreFile.is_open()) {
        highScoreFile >> highScore;
        highScoreFile.close();
    } else {
        std::ofstream createFile("highscore.txt");
        if (createFile.is_open()) {
            createFile << 0; // Default starting high score
            createFile.close();
        } else {
            std::cerr << "Error: Unable to create highscore.txt file." << std::endl;
            return -1; // Exit if the file can't be created
        }
    }

    std::vector<Pipe> pipes;
    sf::Clock pipeClock;
    sf::Clock deltaClock;
    sf::Clock splashClock;

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    GameState state = SPLASH;

    while (window.isOpen()) {
        float dt = deltaClock.restart().asSeconds();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Handle clicks on the start and high score buttons
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (startButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    gameStarted = true;
                    showSplash = false;
                    currentScore = 0;
                    state = GAME;  // Switch to the game state
                }
                if (highScoreButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    state = HIGHSCORE;  // Switch to high score screen
                }
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space && !gameOver && gameStarted)
                    velocity = JUMP;

                if (event.key.code == sf::Keyboard::R && gameOver) {
                    // Reset the game when 'R' is pressed after game over
                    gameStarted = false;
                    gameOver = false;
                    pipes.clear();
                    bird.setPosition(200.f, HEIGHT / 2.f);
                    velocity = 0.f;
                    currentScore = 0; // Reset the score
                    state = MENU;  // Go back to the menu
                }

                // Back to Menu when 'M' is pressed
                if (event.key.code == sf::Keyboard::M && state == HIGHSCORE) {
                    state = MENU;  // Switch back to the menu screen
                }
            }
        }

        //SPLASH TIMER
        if (showSplash && splashClock.getElapsedTime().asSeconds() > 2.f) {
            showSplash = false;
            state = MENU;  // Transition to MENU after splash screen
        }

        //GAME LOGIC
if (gameStarted && !gameOver && state == GAME) {
    // Bird physics
    velocity += GRAVITY * dt;
    bird.move(0.f, velocity * dt);

    // Check if bird hits the top of the screen
    if (bird.getPosition().y < 0) {
        gameOver = true;
        state = GAME_OVER;  // Switch to GAME_OVER state
    }

    // Check if bird hits the bottom of the screen
    if (bird.getPosition().y + bird.getGlobalBounds().height > HEIGHT) {
        gameOver = true;
        state = GAME_OVER;  // Switch to GAME_OVER state
    }

    // Spawn pipes
    if (pipeClock.getElapsedTime().asSeconds() > 1.0f) {
        pipeClock.restart();
        float gapY = 100 + std::rand() % (HEIGHT - 300);

        Pipe pipe;

        pipe.top.setTexture(pipeTex);
        pipe.bottom.setTexture(pipeTex);

        pipe.top.setScale(0.90f, -0.90f);  // Set the correct pipe scaling
        pipe.bottom.setScale(0.90f, 0.90f);

        pipe.top.setPosition(WIDTH, gapY);  // Top pipe position
        pipe.bottom.setPosition(WIDTH, gapY + PIPE_GAP);  // Bottom pipe position

        pipes.push_back(pipe);
    }

    // Move pipes and check for collisions
    for (auto& p : pipes) {
        p.top.move(-PIPE_SPEED * dt, 0.f);
        p.bottom.move(-PIPE_SPEED * dt, 0.f);

        if (bird.getGlobalBounds().intersects(p.top.getGlobalBounds()) ||
            bird.getGlobalBounds().intersects(p.bottom.getGlobalBounds())) {
            gameOver = true;
            state = GAME_OVER;  // Transition to GAME_OVER state
            if (currentScore > highScore)
                highScore = currentScore;
        }

        // Increment score when the bird passes the pipe
        if (!p.passed && p.top.getPosition().x + p.top.getGlobalBounds().width < bird.getPosition().x) {
            currentScore++;
            p.passed = true; // Mark this pipe as passed
        }
    }

    // Remove off-screen pipes
    pipes.erase(
        std::remove_if(pipes.begin(), pipes.end(),
            [](Pipe& p) {
                return p.top.getPosition().x + 200 < 0;
            }),
        pipes.end()
    );
}


        //SAVE HIGH SCORE
        if (gameOver && currentScore > highScore) {
            highScore = currentScore;
            std::ofstream highScoreFile("highscore.txt");
            if (highScoreFile.is_open()) {
                highScoreFile << highScore;
                highScoreFile.close();
            }
        }

        //DRAW
        window.clear();

        if (showSplash) {
            window.draw(splash);
        } else if (state == MENU) {
            window.draw(background);
            window.draw(title); // Draw the title
            window.draw(startButton);
            window.draw(highScoreButton);
        } else if (state == GAME) {
            window.draw(background);
            for (auto& p : pipes) {
                window.draw(p.top);
                window.draw(p.bottom);
            }
            window.draw(bird);

            // Draw score and high score in top-left corner
            sf::Text scoreText("Score: " + std::to_string(currentScore), font, 30);
            scoreText.setPosition(10.f, 10.f);
            scoreText.setFillColor(sf::Color::White);
            window.draw(scoreText);

            sf::Text highScoreText("High Score: " + std::to_string(highScore), font, 30);
            highScoreText.setPosition(10.f, 40.f);
            highScoreText.setFillColor(sf::Color::White);
            window.draw(highScoreText);
        } else if (state == GAME_OVER) {
            // Display Game Over screen
            window.draw(background);
            sf::Text gameOverText("Game Over", font, 50);
            gameOverText.setPosition(WIDTH / 2.f - gameOverText.getLocalBounds().width / 2.f, HEIGHT / 4.f);
            gameOverText.setFillColor(sf::Color::Red);

            sf::Text scoreText("Score: " + std::to_string(currentScore), font, 30);
            scoreText.setPosition(WIDTH / 2.f - scoreText.getLocalBounds().width / 2.f, HEIGHT / 2.f - 50.f);
            scoreText.setFillColor(sf::Color::White);

            sf::Text highScoreText("High Score: " + std::to_string(highScore), font, 30);
            highScoreText.setPosition(WIDTH / 2.f - highScoreText.getLocalBounds().width / 2.f, HEIGHT / 2.f);
            highScoreText.setFillColor(sf::Color::White);

            sf::Text replayText("Press 'R' to Replay", font, 24);
            replayText.setPosition(WIDTH / 2.f - replayText.getLocalBounds().width / 2.f, HEIGHT / 2.f + 50.f);
            replayText.setFillColor(sf::Color::Yellow);

            window.draw(gameOverText);
            window.draw(scoreText);
            window.draw(highScoreText);
            window.draw(replayText);
        } else if (state == HIGHSCORE) {
            // High score screen
            window.draw(background);

            sf::Text highScoreTitle("High Score", font, 50);
            highScoreTitle.setPosition(WIDTH / 2.f - highScoreTitle.getLocalBounds().width / 2.f, HEIGHT / 4.f);
            highScoreTitle.setFillColor(sf::Color::White);

            sf::Text highScoreText("High Score: " + std::to_string(highScore), font, 40);
            highScoreText.setPosition(WIDTH / 2.f - highScoreText.getLocalBounds().width / 2.f, HEIGHT / 2.f);
            highScoreText.setFillColor(sf::Color::Yellow);

            sf::Text backText("Press 'M' to Return to Menu", font, 24);
            backText.setPosition(WIDTH / 2.f - backText.getLocalBounds().width / 2.f, HEIGHT * 3.f / 4.f);
            backText.setFillColor(sf::Color::Red);

            window.draw(highScoreTitle);
            window.draw(highScoreText);
            window.draw(backText);
        }

        window.display();
    }

    return 0;
}
