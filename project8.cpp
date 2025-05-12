#include <graphics.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>

using namespace std;

// Constants
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;
const int ROAD_WIDTH = 800;
const int ROAD_LEFT = (SCREEN_WIDTH - ROAD_WIDTH) / 2;

// Game states
enum GameState { MENU, PLAYING, GAME_OVER, INSTRUCTIONS, HIGH_SCORES, CAR_SELECTION, PLAYER_DETAILS, CREDITS };

// Player Details class
class PlayerDetails {
private:
    string name;
    int age;
    string playerID;
    int gamesPlayed;
    int totalScore;
    
public:
    PlayerDetails() : name(""), age(0), playerID(""), gamesPlayed(0), totalScore(0) {}
    
    void setName(const string& n) { name = n; }
    void setAge(int a) { age = a; }
    void setPlayerID(const string& id) { playerID = id; }
    void incrementGames() { gamesPlayed++; }
    void addScore(int score) { totalScore += score; }
    
    string getName() const { return name; }
    int getAge() const { return age; }
    string getPlayerID() const { return playerID; }
    int getGamesPlayed() const { return gamesPlayed; }
    int getTotalScore() const { return totalScore; }
    
    void inputDetails() {
        cleardevice();
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 3);
        setcolor(LIGHTCYAN);
        
        char input[100];
        
        outtextxy(SCREEN_WIDTH/2 - 250, 150, "Enter your name: ");
        cin.getline(input, sizeof(input));
        name = input;
        
        outtextxy(SCREEN_WIDTH/2 - 250, 250, "Enter your age: ");
        cin.getline(input, sizeof(input));
        age = atoi(input);
        
        // Generate simple player ID
        playerID = name.substr(0, 3) + to_string(age) + to_string(rand() % 1000);
        
        setcolor(LIGHTGREEN);
        outtextxy(SCREEN_WIDTH/2 - 250, 400, "Your Player ID: ");
        outtextxy(SCREEN_WIDTH/2 - 100, 450, const_cast<char*>(playerID.c_str()));
        
        setcolor(YELLOW);
        outtextxy(SCREEN_WIDTH/2 - 300, 550, "Press any key to continue...");
        getch();
    }
    
    void displayDetails(int x, int y) const {
        char buffer[200];
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 2);
        setcolor(LIGHTMAGENTA);
        
        sprintf(buffer, "Player: %s", name.c_str());
        outtextxy(x, y, buffer);
        
        sprintf(buffer, "Age: %d", age);
        outtextxy(x, y + 40, buffer);
        
        sprintf(buffer, "ID: %s", playerID.c_str());
        outtextxy(x, y + 80, buffer);
        
        sprintf(buffer, "Games Played: %d", gamesPlayed);
        outtextxy(x, y + 120, buffer);
        
        sprintf(buffer, "Total Score: %d", totalScore);
        outtextxy(x, y + 160, buffer);
    }
    
    void saveToFile() const {
        ofstream out("player_details.dat", ios::binary | ios::app);
        if (out) {
            out.write(name.c_str(), name.size() + 1);
            out.write(reinterpret_cast<const char*>(&age), sizeof(age));
            out.write(playerID.c_str(), playerID.size() + 1);
            out.write(reinterpret_cast<const char*>(&gamesPlayed), sizeof(gamesPlayed));
            out.write(reinterpret_cast<const char*>(&totalScore), sizeof(totalScore));
            out.close();
        }
    }
    
    void loadFromFile(const string& id) {
        ifstream in("player_details.dat", ios::binary);
        if (in) {
            while (in) {
                char nameBuf[100], idBuf[100];
                int age, games, score;
                
                in.getline(nameBuf, 100, '\0');
                in.read(reinterpret_cast<char*>(&age), sizeof(age));
                in.getline(idBuf, 100, '\0');
                in.read(reinterpret_cast<char*>(&games), sizeof(games));
                in.read(reinterpret_cast<char*>(&score), sizeof(score));
                
                if (!in) break;
                
                if (string(idBuf) == id) {
                    name = nameBuf;
                    this->age = age;
                    playerID = idBuf;
                    gamesPlayed = games;
                    totalScore = score;
                    break;
                }
            }
            in.close();
        }
    }
};

// Base GameObject class
class GameObject {
protected:
    int x, y;
    int width, height;
    bool active;
    int color;

public:
    GameObject(int w, int h, int col) : width(w), height(h), color(col), active(false), x(0), y(0) {}
    virtual ~GameObject() {}

    bool isActive() const { return active; }
    void setActive(bool a) { active = a; }
    
    virtual void spawn(int xPos, int yPos) {
        x = xPos;
        y = yPos;
        active = true;
    }
    
    virtual void update() = 0;
    virtual void draw() const = 0;
    
    bool checkCollision(int objX, int objY, int objW, int objH) const {
        return active &&
            objX < x + width &&
            objX + objW > x &&
            objY < y + height &&
            objY + objH > y;
    }
    
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

// Vehicle class (base for all vehicles)
class Vehicle : public GameObject {
protected:
    int speed;
    
public:
    Vehicle(int w, int h, int col, int spd) : GameObject(w, h, col), speed(spd) {}
    
    void setSpeed(int s) { speed = s; }
    int getSpeed() const { return speed; }
    
    void moveLeft() { if (x > ROAD_LEFT + 10) x -= speed; }
    void moveRight() { if (x < ROAD_LEFT + ROAD_WIDTH - width - 10) x += speed; }
    void moveUp() { if (y > 10) y -= speed; }
    void moveDown() { if (y < SCREEN_HEIGHT - height - 10) y += speed; }
};

// PlayerCar class
class PlayerCar : public Vehicle {
private:
    int carType;
    int lives;
    
public:
    PlayerCar(int type = 0) : Vehicle(50, 80, BLUE, 10), carType(type), lives(3) {
        // Set properties based on car type
        switch (type) {
            case 0: // Default car
                width = 50;
                height = 80;
                color = BLUE;
                speed = 10;
                break;
            case 1: // Sports car
                width = 45;
                height = 75;
                color = RED;
                speed = 15;
                break;
            case 2: // Truck
                width = 60;
                height = 90;
                color = GREEN;
                speed = 7;
                lives = 5; // Extra lives for truck
                break;
        }
    }
    
    void setCarType(int type) { carType = type; }
    int getCarType() const { return carType; }
    
    int getLives() const { return lives; }
    void loseLife() { lives--; }
    void gainLife() { if (lives < 5) lives++; }
    
    void draw() const override {
        if (!active) return;
        
        setfillstyle(SOLID_FILL, color);
        bar(x, y, x + width, y + height);
        
        // Add details based on car type
        switch (carType) {
            case 0: // Default car
                setcolor(WHITE);
                rectangle(x + 10, y + 15, x + width - 10, y + height - 15);
                break;
            case 1: // Sports car
                setcolor(YELLOW);
                line(x + 5, y + height/2, x + width - 5, y + height/2);
                break;
            case 2: // Truck
                setcolor(BLACK);
                rectangle(x + 5, y + 10, x + width - 5, y + height - 20);
                break;
        }
    }
    
    void update() override {} // Player car is controlled by input
};

// EnemyCar class
class EnemyCar : public Vehicle {
private:
    int type;
    
public:
    EnemyCar(int t = 0) : Vehicle(40, 70, RED, 3), type(t) {
        // Randomize enemy car types
        type = rand() % 3;
        
        switch (type) {
            case 0: // Normal car
                width = 40;
                height = 70;
                color = RED;
                speed = 3 + rand() % 5;
                break;
            case 1: // Fast car
                width = 35;
                height = 65;
                color = MAGENTA;
                speed = 6 + rand() % 7;
                break;
            case 2: // Big truck
                width = 60;
                height = 90;
                color = DARKGRAY;
                speed = 2 + rand() % 4;
                break;
        }
    }
    
    void spawn() {
        x = ROAD_LEFT + rand() % (ROAD_WIDTH - width);
        y = -height;
        active = true;
    }
    
    void update() override {
        if (active) {
            y += speed;
            if (y > SCREEN_HEIGHT) active = false;
        }
    }
    
    void draw() const override {
        if (!active) return;
        
        setfillstyle(SOLID_FILL, color);
        bar(x, y, x + width, y + height);
        
        // Add details based on type
        switch (type) {
            case 0: // Normal car
                setcolor(WHITE);
                rectangle(x + 5, y + 10, x + width - 5, y + height - 10);
                break;
            case 1: // Fast car
                setcolor(BLACK);
                line(x, y + height/2, x + width, y + height/2);
                break;
            case 2: // Big truck
                setcolor(RED);
                rectangle(x + 10, y + 5, x + width - 10, y + height - 30);
                break;
        }
    }
};

// Coin class
class Coin : public GameObject {
private:
    int value;
    bool isSpecial;
    int gameSpeed;
    
public:
    Coin(int speed = 3) : GameObject(20, 20, YELLOW), value(5), isSpecial(false), gameSpeed(speed) {
        // 20% chance to be special
        if (rand() % 5 == 0) {
            isSpecial = true;
            value = 10;
            color = LIGHTMAGENTA;
        }
    }
    
    void spawn() {
        x = ROAD_LEFT + rand() % (ROAD_WIDTH - width);
        y = -height;
        active = true;
    }
    
    void update() override {
        if (active) {
            y += gameSpeed - 2;
            if (y > SCREEN_HEIGHT) active = false;
        }
    }
    
    void draw() const override {
        if (!active) return;
        
        setfillstyle(SOLID_FILL, color);
        fillellipse(x + width/2, y + width/2, width/2, width/2);
        
        if (isSpecial) {
            setcolor(YELLOW);
            circle(x + width/2, y + width/2, width/2 + 2);
        }
    }
    
    int getValue() const { return value; }
    bool isCoinSpecial() const { return isSpecial; }
};

// Obstacle class
class Obstacle : public GameObject {
private:
    int gameSpeed;
    
public:
    Obstacle(int speed = 5) : GameObject(60, 30, BROWN), gameSpeed(speed) {}
    
    void spawn() {
        x = ROAD_LEFT + rand() % (ROAD_WIDTH - width);
        y = -height;
        active = true;
    }
    
    void update() override {
        if (active) {
            y += gameSpeed;
            if (y > SCREEN_HEIGHT) active = false;
        }
    }
    
    void draw() const override {
        if (!active) return;
        
        setfillstyle(SOLID_FILL, color);
        bar(x, y, x + width, y + height);
        
        setcolor(DARKGRAY);
        // Draw rocks or cracks
        for (int i = 0; i < 3; i++) {
            line(x + 10 + i*15, y + 5, x + 5 + i*15, y + height - 5);
        }
    }
};

// PowerUp class
class PowerUp : public GameObject {
private:
    int type; // 0 = speed boost, 1 = extra life, 2 = shield
    int gameSpeed;
    
public:
    PowerUp(int speed = 4) : GameObject(25, 25, GREEN), type(rand() % 3), gameSpeed(speed) {
        switch (type) {
            case 0: // Speed boost
                color = LIGHTCYAN;
                break;
            case 1: // Extra life
                color = LIGHTGREEN;
                break;
            case 2: // Shield
                color = LIGHTBLUE;
                break;
        }
    }
    
    int getType() const { return type; }
    
    void spawn() {
        x = ROAD_LEFT + rand() % (ROAD_WIDTH - width);
        y = -height;
        active = true;
    }
    
    void update() override {
        if (active) {
            y += gameSpeed - 1;
            if (y > SCREEN_HEIGHT) active = false;
        }
    }
    
    void draw() const override {
        if (!active) return;
        
        setfillstyle(SOLID_FILL, color);
        fillellipse(x + width/2, y + width/2, width/2, width/2);
        
        setcolor(WHITE);
        switch (type) {
            case 0: // Speed
                outtextxy(x + 5, y + 5, "S");
                break;
            case 1: // Life
                outtextxy(x + 5, y + 5, "L");
                break;
            case 2: // Shield
                outtextxy(x + 5, y + 5, "D");
                break;
        }
    }
};

// HighScoreEntry class
class HighScoreEntry {
private:
    string playerName;
    string playerID;
    int score;
    int carType;
    time_t date;
    
public:
    HighScoreEntry(const string& name = "", const string& id = "", int s = 0, int c = 0) 
        : playerName(name), playerID(id), score(s), carType(c), date(time(0)) {}
        
    string getPlayerName() const { return playerName; }
    string getPlayerID() const { return playerID; }
    int getScore() const { return score; }
    int getCarType() const { return carType; }
    time_t getDate() const { return date; }
    
    string getDateString() const {
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtime(&date));
        return string(buffer);
    }
    
    bool operator<(const HighScoreEntry& other) const {
        return score > other.score;
    }
};

// Game class
class Game {
private:
    GameState state;
    PlayerCar playerCar;
    PlayerDetails playerDetails;
    vector<EnemyCar> enemies;
    vector<Coin> coins;
    vector<Obstacle> obstacles;
    vector<PowerUp> powerUps;
    vector<HighScoreEntry> highScores;
    
    int score;
    int level;
    int gameSpeed;
    bool shieldActive;
    int shieldTimer;
    bool gamePaused;
    
    void drawRoad() const {
        // Road
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(ROAD_LEFT, 0, ROAD_LEFT + ROAD_WIDTH, SCREEN_HEIGHT);
        
        // Road borders
        setfillstyle(SOLID_FILL, WHITE);
        bar(ROAD_LEFT - 5, 0, ROAD_LEFT, SCREEN_HEIGHT);
        bar(ROAD_LEFT + ROAD_WIDTH, 0, ROAD_LEFT + ROAD_WIDTH + 5, SCREEN_HEIGHT);
        
        // Lane markers
        setfillstyle(SOLID_FILL, YELLOW);
        for (int y = -50; y < SCREEN_HEIGHT; y += 100) {
            bar(SCREEN_WIDTH / 2 - 5, y, SCREEN_WIDTH / 2 + 5, y + 50);
        }
    }
    
    void drawHUD() const {
        char buffer[100];
        
        // Score
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 3);
        setcolor(LIGHTGREEN);
        sprintf(buffer, "Score: %d", score);
        outtextxy(20, 20, buffer);
        
        // Level
        setcolor(LIGHTCYAN);
        sprintf(buffer, "Level: %d", level);
        outtextxy(20, 60, buffer);
        
        // Lives
        setcolor(LIGHTRED);
        sprintf(buffer, "Lives: %d", playerCar.getLives());
        outtextxy(20, 100, buffer);
        
        // Shield indicator
        if (shieldActive) {
            setcolor(LIGHTBLUE);
            outtextxy(20, 140, "SHIELD ACTIVE");
        }
        
        // Player info
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 2);
        playerDetails.displayDetails(SCREEN_WIDTH - 300, 20);
    }
    
    void spawnEnemies() {
        // Spawn enemies based on level
        int maxEnemies = 3 + level;
        if (enemies.size() < maxEnemies && rand() % 100 < 5 + level) {
            enemies.emplace_back();
            enemies.back().spawn();
        }
    }
    
    void spawnCoins() {
        if (coins.size() < 5 && rand() % 100 < 8) {
            coins.emplace_back(gameSpeed);
            coins.back().spawn();
        }
    }
    
    void spawnObstacles() {
        if (level > 2 && obstacles.size() < 2 && rand() % 100 < 3) {
            obstacles.emplace_back(gameSpeed);
            obstacles.back().spawn();
        }
    }
    
    void spawnPowerUps() {
        if (powerUps.size() < 1 && rand() % 100 < 2) {
            powerUps.emplace_back(gameSpeed);
            powerUps.back().spawn();
        }
    }
    
    void updateObjects() {
        // Update enemies
        for (auto& enemy : enemies) {
            enemy.update();
            
            if (enemy.checkCollision(playerCar.getX(), playerCar.getY(), 
                                    playerCar.getWidth(), playerCar.getHeight())) {
                if (shieldActive) {
                    shieldActive = false;
                } else {
                    playerCar.loseLife();
                    if (playerCar.getLives() <= 0) {
                        state = GAME_OVER;
                        playerDetails.addScore(score);
                        playerDetails.incrementGames();
                        playerDetails.saveToFile();
                        saveHighScore();
                    }
                }
                enemy.setActive(false);
            }
        }
        
        // Remove inactive enemies
        enemies.erase(remove_if(enemies.begin(), enemies.end(), 
                              [](const EnemyCar& e) { return !e.isActive(); }), 
                     enemies.end());
        
        // Update coins
        for (auto& coin : coins) {
            coin.update();
            
            if (coin.checkCollision(playerCar.getX(), playerCar.getY(), 
                                  playerCar.getWidth(), playerCar.getHeight())) {
                score += coin.getValue();
                coin.setActive(false);
            }
        }
        
        // Remove inactive coins
        coins.erase(remove_if(coins.begin(), coins.end(), 
                     [](const Coin& c) { return !c.isActive(); }), 
                  coins.end());
        
        // Update obstacles
        for (auto& obs : obstacles) {
            obs.update();
            
            if (obs.checkCollision(playerCar.getX(), playerCar.getY(), 
                                 playerCar.getWidth(), playerCar.getHeight())) {
                if (shieldActive) {
                    shieldActive = false;
                } else {
                    playerCar.loseLife();
                    if (playerCar.getLives() <= 0) {
                        state = GAME_OVER;
                        playerDetails.addScore(score);
                        playerDetails.incrementGames();
                        playerDetails.saveToFile();
                        saveHighScore();
                    }
                }
                obs.setActive(false);
            }
        }
        
        // Remove inactive obstacles
        obstacles.erase(remove_if(obstacles.begin(), obstacles.end(), 
                               [](const Obstacle& o) { return !o.isActive(); }), 
                      obstacles.end());
        
        // Update power-ups
        for (auto& pu : powerUps) {
            pu.update();
            
            if (pu.checkCollision(playerCar.getX(), playerCar.getY(), 
                                playerCar.getWidth(), playerCar.getHeight())) {
                applyPowerUp(pu.getType());
                pu.setActive(false);
            }
        }
        
        // Remove inactive power-ups
        powerUps.erase(remove_if(powerUps.begin(), powerUps.end(), 
                      [](const PowerUp& p) { return !p.isActive(); }), 
                   powerUps.end());
        
        // Update shield timer
        if (shieldActive) {
            shieldTimer--;
            if (shieldTimer <= 0) {
                shieldActive = false;
            }
        }
        
        // Level progression
        if (score >= level * 100) {
            level++;
            gameSpeed++;
        }
    }
    
    void applyPowerUp(int type) {
        switch (type) {
            case 0: // Speed boost
                playerCar.setSpeed(playerCar.getSpeed() + 2);
                break;
            case 1: // Extra life
                playerCar.gainLife();
                break;
            case 2: // Shield
                shieldActive = true;
                shieldTimer = 300; // 10 seconds at 30 FPS
                break;
        }
    }
    
    void loadHighScores() {
        highScores.clear();
        ifstream in("highscores.dat", ios::binary);
        if (in) {
            while (in) {
                int score, carType;
                time_t date;
                char name[100], id[100];
                
                in.read(reinterpret_cast<char*>(&score), sizeof(score));
                in.read(reinterpret_cast<char*>(&carType), sizeof(carType));
                in.read(reinterpret_cast<char*>(&date), sizeof(date));
                
                // Read name and ID
                in.getline(name, 100, '\0');
                in.getline(id, 100, '\0');
                
                if (!in) break;
                
                highScores.emplace_back(name, id, score, carType);
            }
            in.close();
        }
        sort(highScores.begin(), highScores.end());
    }
    
    void saveHighScore() {
        // Add current score to high scores
        highScores.emplace_back(playerDetails.getName(), playerDetails.getPlayerID(), 
                              score, playerCar.getCarType());
        sort(highScores.begin(), highScores.end());
        
        // Keep only top 10
        if (highScores.size() > 10) {
            highScores.resize(10);
        }
        
        // Save to file
        ofstream out("highscores.dat", ios::binary);
        if (out) {
            for (const auto& entry : highScores) {
                int score = entry.getScore();
                out.write(reinterpret_cast<const char*>(&score), sizeof(score));
                
                int carType = entry.getCarType();
                out.write(reinterpret_cast<const char*>(&carType), sizeof(carType));
                
                time_t date = entry.getDate();
                out.write(reinterpret_cast<const char*>(&date), sizeof(date));
                
                // Write name and ID as null-terminated strings
                out.write(entry.getPlayerName().c_str(), entry.getPlayerName().size() + 1);
                out.write(entry.getPlayerID().c_str(), entry.getPlayerID().size() + 1);
            }
            out.close();
        }
    }
    
    void drawMenu() {
        cleardevice();
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 5);
        setcolor(LIGHTRED);
        outtextxy(SCREEN_WIDTH/2 - 200, 80, "CAR RACING GAME");
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 3);
        setcolor(WHITE);
        
        outtextxy(SCREEN_WIDTH/2 - 100, 220, "1. Start Game");
        outtextxy(SCREEN_WIDTH/2 - 100, 280, "2. Instructions");
        outtextxy(SCREEN_WIDTH/2 - 100, 340, "3. High Scores");
        outtextxy(SCREEN_WIDTH/2 - 100, 400, "4. Player Details");
        outtextxy(SCREEN_WIDTH/2 - 100, 460, "5. Credits");
        outtextxy(SCREEN_WIDTH/2 - 100, 520, "6. Quit");
        
        if (kbhit()) {
            char ch = getch();
            switch (ch) {
                case '1':
                    state = CAR_SELECTION;
                    break;
                case '2':
                    state = INSTRUCTIONS;
                    break;
                case '3':
                    loadHighScores();
                    state = HIGH_SCORES;
                    break;
                case '4':
                    state = PLAYER_DETAILS;
                    break;
                case '5':
                    state = CREDITS;
                    break;
                case '6':
                    closegraph();
                    exit(0);
            }
        }
    }
    
    void drawCarSelection() {
        cleardevice();
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 4);
        setcolor(LIGHTMAGENTA);
        outtextxy(SCREEN_WIDTH/2 - 200, 80, "SELECT YOUR CAR");
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 3);
        setcolor(WHITE);
        
        // Draw car options
        outtextxy(200, 180, "1. Standard Car (Balanced)");
        outtextxy(200, 240, "2. Sports Car (Fast but fragile)");
        outtextxy(200, 300, "3. Truck (Slow but sturdy)");
        
        // Draw continue prompt
        setcolor(LIGHTCYAN);
        outtextxy(SCREEN_WIDTH/2 - 200, 500, "Press 1-3 to select");
        outtextxy(SCREEN_WIDTH/2 - 200, 550, "ESC to return to menu");
        
        // Handle input
        if (kbhit()) {
            char ch = getch();
            
            if (ch >= '1' && ch <= '3') {
                playerCar.setCarType(ch - '1');
                initializeGame();
                state = PLAYING;
            } else if (ch == 27) { // ESC
                state = MENU;
            }
        }
    }
    
    void drawPlayerDetails() {
        cleardevice();
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 4);
        setcolor(LIGHTMAGENTA);
        outtextxy(SCREEN_WIDTH/2 - 200, 80, "PLAYER DETAILS");
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 2);
        setcolor(WHITE);
        
        if (playerDetails.getName().empty()) {
            outtextxy(SCREEN_WIDTH/2 - 250, 200, "No player details set");
            outtextxy(SCREEN_WIDTH/2 - 250, 300, "Press 1 to enter details");
            outtextxy(SCREEN_WIDTH/2 - 250, 350, "Press 2 to load by ID");
            outtextxy(SCREEN_WIDTH/2 - 250, 450, "ESC to return to menu");
            
            if (kbhit()) {
                char ch = getch();
                if (ch == '1') {
                    playerDetails.inputDetails();
                } else if (ch == '2') {
                    char id[100];
                    outtextxy(SCREEN_WIDTH/2 - 300, 500, "Enter your Player ID: ");
                    cin.getline(id, sizeof(id));
                    playerDetails.loadFromFile(id);
                } else if (ch == 27) {
                    state = MENU;
                }
            }
        } else {
            playerDetails.displayDetails(SCREEN_WIDTH/2 - 250, 200);
            
            outtextxy(SCREEN_WIDTH/2 - 250, 500, "Press 1 to change details");
            outtextxy(SCREEN_WIDTH/2 - 250, 550, "ESC to return to menu");
            
            if (kbhit()) {
                char ch = getch();
                if (ch == '1') {
                    playerDetails.inputDetails();
                } else if (ch == 27) {
                    state = MENU;
                }
            }
        }
    }
    
    void drawInstructions() {
        cleardevice();
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 4);
        setcolor(LIGHTMAGENTA);
        outtextxy(SCREEN_WIDTH/2 - 200, 50, "INSTRUCTIONS");
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 2);
        setcolor(WHITE);
        
        outtextxy(100, 120, "Objective:");
        outtextxy(150, 170, "- Avoid other cars and obstacles");
        outtextxy(150, 220, "- Collect coins for points");
        outtextxy(150, 270, "- Collect power-ups for bonuses");
        
        outtextxy(100, 320, "Controls:");
        outtextxy(150, 370, "W - Move Up");
        outtextxy(150, 420, "S - Move Down");
        outtextxy(150, 470, "A - Move Left");
        outtextxy(150, 520, "D - Move Right");
        outtextxy(150, 570, "P - Pause Game");
        outtextxy(150, 620, "ESC - Quit to Menu");
        
        setcolor(LIGHTCYAN);
        outtextxy(SCREEN_WIDTH/2 - 200, 700, "Press any key to return to menu");
        
        if (kbhit()) {
            getch();
            state = MENU;
        }
    }
    
    void drawHighScores() {
        cleardevice();
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 4);
        setcolor(LIGHTGREEN);
        outtextxy(SCREEN_WIDTH/2 - 150, 50, "HIGH SCORES");
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 2);
        setcolor(WHITE);
        
        // Column headers
        outtextxy(100, 120, "Rank");
        outtextxy(200, 120, "Player");
        outtextxy(400, 120, "Score");
        outtextxy(500, 120, "Car Type");
        outtextxy(700, 120, "Date");
        
        // Display top 10 scores
        for (int i = 0; i < min(10, (int)highScores.size()); i++) {
            const auto& entry = highScores[i];
            
            char rank[10];
            sprintf(rank, "%d.", i+1);
            outtextxy(100, 170 + i*50, rank);
            
            outtextxy(200, 170 + i*50, const_cast<char*>(entry.getPlayerName().c_str()));
            
            char score[20];
            sprintf(score, "%d", entry.getScore());
            outtextxy(400, 170 + i*50, score);
            
            const char* carTypes[] = {"Standard", "Sports", "Truck"};
            outtextxy(500, 170 + i*50, const_cast<char*>(carTypes[entry.getCarType()]));
            
            outtextxy(700, 170 + i*50, const_cast<char*>(entry.getDateString().c_str()));
        }
        
        setcolor(LIGHTCYAN);
        outtextxy(SCREEN_WIDTH/2 - 200, 700, "Press any key to return to menu");
        
        if (kbhit()) {
            getch();
            state = MENU;
        }
    }
    
    void drawCredits() {
        cleardevice();
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 4);
        setcolor(LIGHTMAGENTA);
        outtextxy(SCREEN_WIDTH/2 - 200, 100, "GAME CREDITS");
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 3);
        setcolor(LIGHTCYAN);
        outtextxy(SCREEN_WIDTH/2 - 300, 200, "This game was developed by:");
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 2);
        setcolor(WHITE);
        
        outtextxy(SCREEN_WIDTH/2 - 150, 300, "Harshdeep Singh");
        outtextxy(SCREEN_WIDTH/2 - 150, 350, "Harshit");
        outtextxy(SCREEN_WIDTH/2 - 150, 400, "Harsh");
        outtextxy(SCREEN_WIDTH/2 - 150, 450, "Tejas");
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 1);
        outtextxy(SCREEN_WIDTH/2 - 200, 550, "Graphics Programming using BGI");
        
        setcolor(LIGHTGREEN);
        outtextxy(SCREEN_WIDTH/2 - 200, 700, "Press any key to return to menu");
        
        if (kbhit()) {
            getch();
            state = MENU;
        }
    }
    
    void drawGameOver() {
        cleardevice();
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 5);
        setcolor(LIGHTRED);
        outtextxy(SCREEN_WIDTH/2 - 200, 200, "GAME OVER");
        
        settextstyle(COMPLEX_FONT, HORIZ_DIR, 3);
        setcolor(WHITE);
        
        char scoreText[50];
        sprintf(scoreText, "Your Score: %d", score);
        outtextxy(SCREEN_WIDTH/2 - 150, 300, scoreText);
        
        if (!highScores.empty() && score == highScores[0].getScore()) {
            setcolor(LIGHTGREEN);
            outtextxy(SCREEN_WIDTH/2 - 200, 350, "NEW HIGH SCORE!");
        }
        
        setcolor(LIGHTCYAN);
        outtextxy(SCREEN_WIDTH/2 - 250, 500, "Press any key to return to menu");
        
        if (kbhit()) {
            getch();
            state = MENU;
        }
    }
    
    void drawPauseScreen() {
        // Semi-transparent overlay
        setfillstyle(SOLID_FILL, BLACK);
        bar(SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 100, 
            SCREEN_WIDTH/2 + 250, SCREEN_HEIGHT/2 + 100);
        
            settextstyle(COMPLEX_FONT, HORIZ_DIR, 4);
            setcolor(WHITE);
            outtextxy(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 50, "PAUSED");
            
            settextstyle(COMPLEX_FONT, HORIZ_DIR, 2);
            outtextxy(SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 + 20, "Press P to continue");
            outtextxy(SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 + 50, "ESC to quit to menu");
        }
        
        void handleInput() {
            if (kbhit()) {
                char ch = getch();
                
                if (ch == 'p' || ch == 'P') {
                    gamePaused = !gamePaused;
                } else if (ch == 27) { // ESC
                    if (gamePaused) {
                        gamePaused = false;
                        state = MENU;
                    }
                }
                
                if (!gamePaused) {
                    switch (tolower(ch)) {
                        case 'w': playerCar.moveUp(); break;
                        case 's': playerCar.moveDown(); break;
                        case 'a': playerCar.moveLeft(); break;
                        case 'd': playerCar.moveRight(); break;
                    }
                }
            }
        }
        
        void initializeGame() {
            score = 0;
            level = 1;
            gameSpeed = 5;
            shieldActive = false;
            gamePaused = false;
            
            // Clear all objects
            enemies.clear();
            coins.clear();
            obstacles.clear();
            powerUps.clear();
            
            // Spawn player car
            playerCar.setActive(true);
            playerCar.spawn(SCREEN_WIDTH/2 - playerCar.getWidth()/2, 
                           SCREEN_HEIGHT - playerCar.getHeight() - 20);
        }
        
    public:
        Game() : state(MENU), score(0), level(1), gameSpeed(5), 
                shieldActive(false), shieldTimer(0), gamePaused(false) {
            // Initialize player car
            playerCar.spawn(SCREEN_WIDTH/2 - playerCar.getWidth()/2, 
                           SCREEN_HEIGHT - playerCar.getHeight() - 20);
            
            // Preallocate some objects
            enemies.reserve(10);
            coins.reserve(10);
            obstacles.reserve(5);
            powerUps.reserve(3);
        }
        
        void run() {
            while (true) {
                switch (state) {
                    case MENU:
                        drawMenu();
                        break;
                    case CAR_SELECTION:
                        drawCarSelection();
                        break;
                    case PLAYER_DETAILS:
                        drawPlayerDetails();
                        break;
                    case INSTRUCTIONS:
                        drawInstructions();
                        break;
                    case HIGH_SCORES:
                        drawHighScores();
                        break;
                    case CREDITS:
                        drawCredits();
                        break;
                    case PLAYING:
                        if (!gamePaused) {
                            handleInput();
                            
                            // Game logic
                            spawnEnemies();
                            spawnCoins();
                            spawnObstacles();
                            spawnPowerUps();
                            updateObjects();
                        }
                        
                        // Drawing
                        drawRoad();
                        playerCar.draw();
                        
                        for (const auto& enemy : enemies) enemy.draw();
                        for (const auto& coin : coins) coin.draw();
                        for (const auto& obs : obstacles) obs.draw();
                        for (const auto& pu : powerUps) pu.draw();
                        
                        drawHUD();
                        
                        if (gamePaused) {
                            drawPauseScreen();
                        }
                        break;
                    case GAME_OVER:
                        drawGameOver();
                        break;
                }
                
                delay(30); // Control game speed
            }
        }
    };
    
    int main() {
        // Initialize graphics
        int gd = DETECT, gm;
        initgraph(&gd, &gm, "");
        initwindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Car Racing Game");
        
        // Seed random number generator
        srand(time(0));
        
        // Create and run game
        Game game;
        game.run();
        
        closegraph();
        return 0;
    }