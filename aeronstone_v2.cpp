#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>
#include <conio.h> // For _kbhit
#include <thread>  // For std::this_thread::sleep_for
#include <chrono>  // For std::chrono::milliseconds

// Define colors for use with SetConsoleTextAttribute
#define COL_FOREGROUND_BLUE      0x0001
#define COL_FOREGROUND_GREEN     0x0002
#define COL_FOREGROUND_RED       0x0004
#define COL_FOREGROUND_INTENSITY 0x0008
#define COL_BACKGROUND_BLUE      0x0010
#define COL_BACKGROUND_GREEN     0x0020
#define COL_BACKGROUND_RED       0x0040
#define COL_BACKGROUND_INTENSITY 0x0080

// Game constants
const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 25;
const int GAME_UPDATE_MS = 16; // Roughly 60 frames per second

// --- Utility Functions ---

// Global handle to the console for drawing
HANDLE hConsole;

// Sets the cursor position in the console window
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Sets the text color in the console
void setColor(int color) {
    SetConsoleTextAttribute(hConsole, color);
}

// Hides the blinking cursor
void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// --- Game Object Classes ---

// Forward declaration of the Game class
class Game; 

// Base class for all objects in the game (players, bullets, etc.)
class GameObject {
public:
    float x, y;
    char symbol;
    int color;
    bool isAlive;
    Game* game; // Pointer to the main game class to interact with it

    GameObject(Game* g, float x, float y, char s, int c) 
        : game(g), x(x), y(y), symbol(s), color(c), isAlive(true) {}

    virtual ~GameObject() {}

    // Pure virtual functions that must be implemented by derived classes
    virtual void update() = 0; 
    virtual void draw() {
        if (!isAlive) return;
        gotoxy(static_cast<int>(x), static_cast<int>(y));
        setColor(color);
        std::cout << symbol;
    }
};

class Bullet : public GameObject {
public:
    float velocityX;

    Bullet(Game* g, float x, float y, float velX, char s, int c)
        : GameObject(g, x, y, s, c), velocityX(velX) {}

    void update() override {
        if (!isAlive) return;
        x += velocityX;

        // Check for out of bounds
        if (x < 0 || x >= SCREEN_WIDTH) {
            isAlive = false;
        }
    }
};

class Player : public GameObject {
public:
    int score;
    int upKey, downKey, shootKey;
    float shootCooldown;
    const float SHOOT_DELAY = 0.3f; // Seconds between shots

    Player(Game* g, float x, float y, char s, int c, int up, int down, int shoot)
        : GameObject(g, x, y, s, c), score(0), upKey(up), downKey(down), shootKey(shoot), shootCooldown(0) {}

    void update() override; // Implemented after Game class is defined

    void addScore(int points) {
        score += points;
    }
};

// --- Main Game Class ---

class Game {
private:
    bool isRunning;
    std::vector<GameObject*> gameObjects;
    Player *player1, *player2;
    HANDLE hBuffer; // Handle for the back buffer
    CHAR_INFO* screenBuffer; // The character info buffer for double buffering

public:
    Game() : isRunning(false), player1(nullptr), player2(nullptr) {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        hBuffer = CreateConsoleScreenBuffer(
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            CONSOLE_TEXTMODE_BUFFER,
            NULL
        );
        screenBuffer = new CHAR_INFO[SCREEN_WIDTH * SCREEN_HEIGHT];
        SetConsoleActiveScreenBuffer(hBuffer);
        hideCursor();
    }

    ~Game() {
        // Clean up memory
        for (auto& obj : gameObjects) {
            delete obj;
        }
        gameObjects.clear();
        delete[] screenBuffer;
        CloseHandle(hBuffer);
        // Restore original console buffer
        SetConsoleActiveScreenBuffer(hConsole);
    }

    void addGameObject(GameObject* obj) {
        gameObjects.push_back(obj);
    }

    void showStartScreen() {
        clearScreen();
        setColor(COL_FOREGROUND_RED | COL_FOREGROUND_GREEN | COL_FOREGROUND_INTENSITY); // Bright Yellow
        gotoxy(28, 6); std::cout << "A E R O N S T O N E";
        setColor(COL_FOREGROUND_RED | COL_FOREGROUND_GREEN | COL_FOREGROUND_BLUE); // White
        gotoxy(22, 10); std::cout << "Player 1 (Left): 'W' Up, 'S' Down, 'D' Shoot";
        gotoxy(22, 12); std::cout << "Player 2 (Right): 'O' Up, 'L' Down, 'K' Shoot";
        gotoxy(28, 16); std::cout << "First to 5 points wins!";
        gotoxy(25, 20); std::cout << "Press any key to start...";
        present();
        _getch();
    }

    void showEndScreen() {
        clearScreen();
        std::string winnerText = (player1->score > player2->score) ? "Player 1 Wins!" : "Player 2 Wins!";
        int winnerColor = (player1->score > player2->score) ? (COL_FOREGROUND_BLUE | COL_FOREGROUND_INTENSITY) : (COL_FOREGROUND_RED | COL_FOREGROUND_INTENSITY);
        
        setColor(winnerColor);
        gotoxy(32, 10); std::cout << winnerText;
        
        setColor(COL_FOREGROUND_RED | COL_FOREGROUND_GREEN | COL_FOREGROUND_BLUE); // White
        gotoxy(28, 14); std::cout << "Press any key to exit.";

        present();
        _getch();
    }

    void initialize() {
        player1 = new Player(this, 2, SCREEN_HEIGHT / 2, '&', COL_FOREGROUND_BLUE | COL_FOREGROUND_INTENSITY, 'W', 'S', 'D');
        player2 = new Player(this, SCREEN_WIDTH - 3, SCREEN_HEIGHT / 2, '$', COL_FOREGROUND_RED | COL_FOREGROUND_INTENSITY, 'O', 'L', 'K');
        addGameObject(player1);
        addGameObject(player2);
    }

    void run() {
        initialize();
        showStartScreen();
        isRunning = true;

        auto lastTime = std::chrono::high_resolution_clock::now();

        while (isRunning) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            processInput();
            update(deltaTime.count());
            render();

            if (player1->score >= 5 || player2->score >= 5) {
                isRunning = false;
            }

            // Sleep to maintain a consistent frame rate
            std::this_thread::sleep_for(std::chrono::milliseconds(GAME_UPDATE_MS));
        }
        showEndScreen();
    }

private:
    void processInput() {
        if (_kbhit()) {
            char key = _getch();
            if (key == 27) { // ESC key
                isRunning = false;
            }
        }
    }

    void update(float deltaTime) {
        // Update all game objects
        for (auto& obj : gameObjects) {
            obj->update();
        }

        // Collision detection
        std::vector<Bullet*> bullets;
        for (auto& obj : gameObjects) {
            if (dynamic_cast<Bullet*>(obj) != nullptr && obj->isAlive) {
                bullets.push_back(static_cast<Bullet*>(obj));
            }
        }

        for (auto& bullet : bullets) {
            // Check collision with player 1
            if (static_cast<int>(bullet->x) == static_cast<int>(player1->x) && static_cast<int>(bullet->y) == static_cast<int>(player1->y)) {
                if (bullet->velocityX > 0) { // Only bullets from player 2 can hit player 1
                    player2->addScore(1);
                    bullet->isAlive = false;
                    player1->isAlive = false; // "Kill" player for a moment
                    Beep(200, 150);
                }
            }
            // Check collision with player 2
            else if (static_cast<int>(bullet->x) == static_cast<int>(player2->x) && static_cast<int>(bullet->y) == static_cast<int>(player2->y)) {
                if (bullet->velocityX < 0) { // Only bullets from player 1 can hit player 2
                    player1->addScore(1);
                    bullet->isAlive = false;
                    player2->isAlive = false;
                    Beep(200, 150);
                }
            }
        }
        
        // "Respawn" players if they were hit
        if (!player1->isAlive) {
            player1->x = 2;
            player1->y = SCREEN_HEIGHT / 2;
            player1->isAlive = true;
        }
        if (!player2->isAlive) {
            player2->x = SCREEN_WIDTH - 3;
            player2->y = SCREEN_HEIGHT / 2;
            player2->isAlive = true;
        }

        // Remove dead objects
        gameObjects.erase(
            std::remove_if(gameObjects.begin(), gameObjects.end(), 
                [](GameObject* obj) {
                    if (!obj->isAlive) {
                        // Don't delete players, just bullets
                        if (dynamic_cast<Player*>(obj) == nullptr) {
                            delete obj;
                            return true;
                        }
                    }
                    return false;
                }), 
            gameObjects.end()
        );
    }
    
    // Renders the entire scene to the back buffer
    void render() {
        clearScreen();
        drawUI();
        for (const auto& obj : gameObjects) {
            obj->draw();
        }
        present();
    }
    
    // Clears the back buffer
    void clearScreen() {
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
            screenBuffer[i].Char.AsciiChar = ' ';
            screenBuffer[i].Attributes = COL_FOREGROUND_RED | COL_FOREGROUND_GREEN | COL_FOREGROUND_BLUE;
        }
    }

    // Draws character to back buffer. Overwrites the global gotoxy/cout for buffering
    void drawToBuffer(int x, int y, char symbol, int color) {
        if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
            screenBuffer[x + y * SCREEN_WIDTH].Char.AsciiChar = symbol;
            screenBuffer[x + y * SCREEN_WIDTH].Attributes = color;
        }
    }

    // Swaps the back buffer to the active console screen
    void present() {
        COORD bufferSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
        COORD bufferCoord = { 0, 0 };
        SMALL_RECT writeRegion = { 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 };
        WriteConsoleOutputA(hBuffer, screenBuffer, bufferSize, bufferCoord, &writeRegion);
    }
    
    // Draws the User Interface (scores, etc)
    void drawUI() {
        std::string p1_score = "Player 1: " + std::to_string(player1->score);
        std::string p2_score = "Player 2: " + std::to_string(player2->score);

        setColor(COL_FOREGROUND_BLUE | COL_FOREGROUND_GREEN | COL_FOREGROUND_INTENSITY);
        for(size_t i = 0; i < p1_score.length(); ++i) {
            drawToBuffer(2 + i, 1, p1_score[i], COL_FOREGROUND_BLUE | COL_FOREGROUND_INTENSITY);
        }
        for (size_t i = 0; i < p2_score.length(); ++i) {
            drawToBuffer(SCREEN_WIDTH - p2_score.length() - 2 + i, 1, p2_score[i], COL_FOREGROUND_RED | COL_FOREGROUND_INTENSITY);
        }

        // Draw a dividing line
        for (int i = 0; i < SCREEN_WIDTH; ++i) {
            drawToBuffer(i, 2, 205, COL_FOREGROUND_RED | COL_FOREGROUND_GREEN | COL_FOREGROUND_BLUE); // 205 is '═'
        }
    }
};

// --- Player::update Implementation ---
// We must implement this after the Game class is fully defined
void Player::update() {
    if (!isAlive) return;

    // Movement
    if (GetAsyncKeyState(upKey) & 0x8000) {
        y--;
    }
    if (GetAsyncKeyState(downKey) & 0x8000) {
        y++;
    }

    // Keep player in bounds (below the UI line)
    if (y < 3) y = 3;
    if (y >= SCREEN_HEIGHT) y = SCREEN_HEIGHT - 1;

    // Shooting
    if (shootCooldown > 0) {
        shootCooldown -= (float)GAME_UPDATE_MS / 1000.0f; // Decrease cooldown
    }
    if (GetAsyncKeyState(shootKey) & 0x8000 && shootCooldown <= 0) {
        shootCooldown = SHOOT_DELAY;
        float bulletVelocity = (x < SCREEN_WIDTH / 2) ? 1.0f : -1.0f;
        int bulletColor = (x < SCREEN_WIDTH / 2) ? (COL_FOREGROUND_GREEN | COL_FOREGROUND_INTENSITY) : (COL_FOREGROUND_RED | COL_FOREGROUND_GREEN | COL_FOREGROUND_INTENSITY);
        char bulletChar = (x < SCREEN_WIDTH / 2) ? '-' : '=';
        
        // Spawn bullet in front of player
        Bullet* b = new Bullet(game, x + bulletVelocity, y, bulletVelocity, bulletChar, bulletColor); 
        game->addGameObject(b);
        Beep(500, 50); // Shoot sound
    }
}

// --- Main Function ---

int main() {
    // Set console window size and title
    system("mode con: cols=80 lines=25");
    system("title Aeronstone Refactored");
    
    Game game;
    game.run();

    return 0;
}
