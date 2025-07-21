#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>  
#include <ctime>

#include "Grid.h"
#include "Tetromino.h"
#include "Manager.h"
#include "Menu.h"

using namespace sf;
using namespace std;

int ROWS = 22;
int COLS = 12;
int cellSize = 25;

int screenWidth = 540;
int screenHeight = 730;

char tetrominoBag[7] = { 'I', 'J', 'L', 'O', 'S', 'T', 'Z' };

struct GameOver
{
    bool isGameOver = false;
    bool showGameOverText = false;
    Clock gameOverClock;
    bool nameInputReady = false;
    bool waitingForNameConfirm = false;

    operator bool() const
    {
        return isGameOver;
    }
    GameOver& operator=(bool value)
    {
        isGameOver = value;
        return *this;
    }
};

void shuffleBag(char bag[], int size, char lastPiece);
void fillTetromino(bool (*tetroFilled)[4], string* tetro);
void rotateTetromino(string* tetro, bool (*tetroFilled)[4], int row, int col);
void removeLineAndUpdateGrid(Grid& grid, int rowIndex);
void checkLineClearAndUpdateScore(Grid& grid, Music& music, int& totalLines, int& score, int& level);
bool checkGameOver(Grid& grid, bool (*tetroFilled)[4], int row, int col);
string* copyString(string* src);
void drawRulesScreen(RenderWindow& window, Font font);
void loadNextTetromino(char& currentTetroChar, char& nextTetroChar, char& lastTetroChar, int& bagIndex);
void loadGameOverSate(GameOver& gameOver, Manager& scoreManager, bool& showMenu, int& gameScore, int& gameLevel);

int main()
{
    // ===============================
    // Window & UI Setup
    // ===============================

    int titleBarHeight = screenHeight / 4;

    RenderWindow window(VideoMode(screenWidth, screenHeight), "TETRIS", Style::Close);

    Font font;
    if (!font.loadFromFile("Gayo Land.ttf")) { cout << "\n The font not loaded !"; }

    // ===============================
    // Game Systems
    // ===============================

    Grid gameGrid(ROWS, COLS);
    gameGrid.clearGrid();

    Tetromino t(ROWS, COLS);
    bool tetroFilled[4][4] = { false };

    Manager scoreManager;
    Menu mainMenu(window, font, screenHeight, screenWidth);

    // ===============================
    // Tetromino Bag System
    // ===============================

    srand(time(0)); 
    char lastTetroChar = ' ';
    shuffleBag(tetrominoBag, 7, lastTetroChar);
    int bagIndex = 0;
    char tetrominoSet[] = { 'I', 'J', 'L', 'O', 'S', 'T', 'Z' };
    
    // ===============================
    // Tetromino Selection
    // ===============================

    string* currentTetro = nullptr;
    char randomTetro = ' ';
    char nextTetroChar = tetrominoSet[rand() % 7];
    char currentTetroChar = ' ';

    // ===============================
    // Player Name & Input
    // ===============================

    string playerName = "";
    bool enterNameMode = false;
    bool nameEntered = false;

    // ===============================
    // Falling Block State
    // ===============================

    bool tetroPlaced = true;
    bool readyForDrop = false;
    int startRow = 0;
    int startCol = 4;
    int blockRow = startRow;
    int blockCol = startCol;
    bool hardDropped = false;

    // ===============================
    // Score & Level System
    // ===============================

    int gameScore = 0;
    int gameLevel = 1;
    int lineCount = 0;
    int comboCount = 0;
    bool lineClear = false;

    // ===============================
    // Game Input & State Flags
    // ===============================

    bool lastUpFrame = false;
    bool lastSpaceFrame = false;
    bool lastEnterFrame = false;
    bool lastMFrame = false;
    bool showMenu = true;
    bool pauseGame = false;
    bool showRules = false;

    // ===============================
    // Game Over & Name Entry State
    // ===============================

    bool showingHighscores = false;
    GameOver gameOver;

    // ===============================
    // Countdown & Timing
    // ===============================

    bool startCountdown = false;
    Clock countdownClock;
    static int countdownValue = 3;

    Clock clock;
    float delay = 0.75f; 

    // ===============================
    // Music & Sounds
    // ===============================

    Music gameMusic;
    gameMusic.openFromFile("Sounds/Tetris Soundtrack.mp3");
    gameMusic.setVolume(20);

    Music gameOverMusic;
    gameOverMusic.openFromFile("Sounds/Game Over.mp3");
    gameOverMusic.setVolume(100);
    
    Music lineClearMusic;
    lineClearMusic.openFromFile("Sounds/Line Clear.mp3");
    lineClearMusic.setVolume(100);

    Music hardDropMusic;
    hardDropMusic.openFromFile("Sounds/Hard Drop.mp3");
    hardDropMusic.setVolume(100);

    Music motionMusic;
    motionMusic.openFromFile("Sounds/Block Change Position.mp3");
    motionMusic.setVolume(100);

    Music clickMusic;
    clickMusic.openFromFile("Sounds/Click Sound.mp3");
    clickMusic.setVolume(100);

    Music countdownMusic;
    countdownMusic.openFromFile("Sounds/Countdown.mp3");
    countdownMusic.setVolume(100);

    bool playGameMusic = true;
    bool playGameOverMusic = true;
    bool playMenuMusic = false;
    bool playCountdownMusic = true;
    int muteVolume = 1;

    int soundValue = 100;
    int musicValue = 100;

    while (window.isOpen())
    {
        if ((playGameMusic && !showMenu && !gameOver && !startCountdown))
        {
            gameMusic.play();
            playGameMusic = false;
        }
        else if (gameOver) 
            gameMusic.stop();
        else if (!playGameMusic && !showMenu && !gameOver && !startCountdown && gameMusic.getStatus() == Sound::Stopped) 
            playGameMusic = true;

        static int prevIndex = -1;
        bool gameStopped = gameOver || pauseGame || startCountdown;

        if (tetroPlaced) 
        {
            loadNextTetromino(currentTetroChar, nextTetroChar, lastTetroChar, bagIndex);

            if (checkGameOver(gameGrid, tetroFilled, blockRow, blockCol))
            {
                loadGameOverSate(gameOver, scoreManager, showMenu, gameScore, gameLevel);
            }
            tetroPlaced = false;
        }

        randomTetro = currentTetroChar;
        currentTetro = t.selectTetro(randomTetro);
        fillTetromino(tetroFilled, currentTetro);

        float elapsed = clock.getElapsedTime().asSeconds();

        Event ev;
        while (window.pollEvent(ev))
        {
            scoreManager.handleTextInput(ev);

            if (ev.type == Event::Closed)
            {
                window.close();
            }
            else if (ev.type == Event::KeyPressed)
            {
                // Mute / Unmute Volume
                if (ev.key.code == Keyboard::M && !lastMFrame)
                {
                    muteVolume = (muteVolume == 0) ? 1 : 0;
                    lastMFrame = true;
                }
                if ((showingHighscores || showRules) && ev.key.code == Keyboard::Enter && !lastEnterFrame)
                {
                    clickMusic.play();
                    showingHighscores = false;
                    showRules = false;
                    showMenu = true;
                    lastEnterFrame = true;
                }
                else if (gameOver)
                {
                    if (ev.key.code == Keyboard::Enter && gameOver.waitingForNameConfirm && !lastEnterFrame)
                    {
                        clickMusic.play();
                        if (scoreManager.isNameSaved())
                        {
                            scoreManager.saveScoreToFile();
                            gameOver = false;
                            gameOver.nameInputReady = false;
                            gameOver.waitingForNameConfirm = false;
                        }
                        lastEnterFrame = true;
                    }
                }

                else if (showMenu)
                {
                    soundValue = 100;

                    // Handle menu navigation
                    if (ev.key.code == Keyboard::Up)
                    {
                        mainMenu.moveUp();
                        clickMusic.play();
                    }
                    else if (ev.key.code == Keyboard::Down)
                    {
                        mainMenu.moveDown();
                        clickMusic.play();
                    }
                    else if (ev.key.code == Keyboard::Enter && !lastEnterFrame)
                    {
                        clickMusic.play();
                        int selection = mainMenu.getSelectedIndex();
                        if (selection == 0)
                        {
                            showMenu = false;           // Start game
                            playGameMusic = true;
                            startCountdown = true;
                            countdownClock.restart();
                        }
                        else if (selection == 1)
                        {
                            showingHighscores = true;   // Show Highscores
                        }
                        else if (selection == 2)
                        {
                            showRules = true;           // Show Rules
                        }
                        else if (selection == 3)
                        {
                            window.close();             // Exit
                        }
                        lastEnterFrame = true;
                    }
                }
                else // Game is running and not in menu
                {
                    if (ev.key.code == Keyboard::Enter && !startCountdown && !lastEnterFrame)
                    {
                        clickMusic.play();
                        pauseGame = !pauseGame;
                        lastEnterFrame = true;
                    }
                    if (pauseGame)
                    {
                        if (ev.key.code == Keyboard::Up) (musicValue == 100) ? musicValue : musicValue += 10;
                        if (ev.key.code == Keyboard::Down) (musicValue == 0) ? musicValue : musicValue-=10;
                        if (ev.key.code == Keyboard::Right) (soundValue == 100) ? soundValue :soundValue+=10;
                        if (ev.key.code == Keyboard::Left) (soundValue == 0) ? soundValue : soundValue-=10;
                    }
                    if (ev.key.code == Keyboard::Up && !lastUpFrame && !gameStopped)
                    {
                        motionMusic.play();
                        gameGrid.clearTetromino(tetroFilled, blockRow, blockCol);
                        if ((t.checkLeftBoundary(gameGrid, tetroFilled, blockRow, blockCol)) &&
                            (t.checkRightBoundary(gameGrid, tetroFilled, blockRow, blockCol)) &&
                            (t.checkBottomBoundary(gameGrid, tetroFilled, blockRow, blockCol)))
                        {   
                            rotateTetromino(currentTetro, tetroFilled, blockRow, blockCol);
                        }
                        gameGrid.clearTetromino(tetroFilled, blockRow, blockCol);
                        gameGrid.placeTetromino(currentTetro, blockRow, blockCol);
                        lastUpFrame = true;
                    }

                    if (ev.key.code == Keyboard::Down && !gameStopped)
                    {
                        motionMusic.play();
                        if (t.checkBottomBoundary(gameGrid, tetroFilled, blockRow, blockCol))
                        {
                            gameGrid.clearTetromino(tetroFilled, blockRow, blockCol);
                            blockRow++;
                            gameGrid.placeTetromino(currentTetro, blockRow, blockCol);
                            tetroPlaced = false;
                        }
                        else
                        {
                            checkLineClearAndUpdateScore(gameGrid, lineClearMusic, lineCount, gameScore, gameLevel);

                            loadNextTetromino(currentTetroChar, nextTetroChar, lastTetroChar, bagIndex);

                            blockRow = startRow;
                            blockCol = startCol;

                            if (checkGameOver(gameGrid, tetroFilled, blockRow, blockCol))
                            {
                                loadGameOverSate(gameOver, scoreManager, showMenu, gameScore, gameLevel);
                            }
                        }
                    }

                    if (ev.key.code == Keyboard::Space && !gameStopped && !lastSpaceFrame)
                    {
                        hardDropMusic.play();
                        while (t.checkBottomBoundary(gameGrid, tetroFilled, blockRow, blockCol))
                        {
                            gameGrid.clearTetromino(tetroFilled, blockRow, blockCol);
                            blockRow++;
                            gameGrid.placeTetromino(currentTetro, blockRow, blockCol);
                        }
                        gameGrid.placeTetromino(currentTetro, blockRow, blockCol);
                        checkLineClearAndUpdateScore(gameGrid, lineClearMusic, lineCount, gameScore, gameLevel);

                        loadNextTetromino(currentTetroChar, nextTetroChar, lastTetroChar, bagIndex);

                        blockRow = startRow;
                        blockCol = startCol;

                        if (checkGameOver(gameGrid, tetroFilled, blockRow, blockCol))
                        {
                            loadGameOverSate(gameOver, scoreManager, showMenu, gameScore, gameLevel);
                        }

                        hardDropped = true;
                        lastSpaceFrame = true;
                    }

                    if (ev.key.code == Keyboard::Left && !gameStopped)
                    {
                        motionMusic.play();
                        gameGrid.clearTetromino(tetroFilled, blockRow, blockCol);
                        if (t.checkLeftBoundary(gameGrid, tetroFilled, blockRow, blockCol)) blockCol--;
                        gameGrid.placeTetromino(currentTetro, blockRow, blockCol);
                    }

                    if (ev.key.code == Keyboard::Right && !gameStopped)
                    {
                        motionMusic.play();
                        gameGrid.clearTetromino(tetroFilled, blockRow, blockCol);
                        if (t.checkRightBoundary(gameGrid, tetroFilled, blockRow, blockCol)) blockCol++;
                        gameGrid.placeTetromino(currentTetro, blockRow, blockCol);
                    }
                }
            }
        }

        if (!Keyboard::isKeyPressed(Keyboard::Up)) lastUpFrame = false;
        if (!Keyboard::isKeyPressed(Keyboard::Enter)) lastEnterFrame = false;
        if (!Keyboard::isKeyPressed(Keyboard::Space)) lastSpaceFrame = false;
        if (!Keyboard::isKeyPressed(Keyboard::M)) lastMFrame = false;

        // Update Music Volumes
        gameMusic.setVolume(muteVolume * (musicValue * 0.5));
        clickMusic.setVolume(muteVolume * soundValue);
        motionMusic.setVolume(muteVolume * soundValue);
        countdownMusic.setVolume(muteVolume * soundValue);
        gameOverMusic.setVolume(muteVolume * soundValue);
        hardDropMusic.setVolume(muteVolume * soundValue);
        lineClearMusic.setVolume(muteVolume * soundValue);

        // Falling down of tetromino with each time frame
        if (elapsed >= delay && !showMenu && !gameStopped && !hardDropped)
        {
            if (t.checkBottomBoundary(gameGrid, tetroFilled, blockRow, blockCol))
            {
                gameGrid.clearTetromino(tetroFilled, blockRow, blockCol);
                blockRow++; 
                gameGrid.placeTetromino(currentTetro, blockRow, blockCol);
            }
            else
            {
                gameGrid.placeTetromino(currentTetro, blockRow, blockCol);

                checkLineClearAndUpdateScore(gameGrid, lineClearMusic, lineCount, gameScore, gameLevel);

                loadNextTetromino(currentTetroChar, nextTetroChar, lastTetroChar, bagIndex);

                blockRow = startRow;
                blockCol = startCol;

                if (checkGameOver(gameGrid, tetroFilled, blockRow, blockCol))
                {
                    loadGameOverSate(gameOver, scoreManager, showMenu, gameScore, gameLevel);
                }
            }
            clock.restart();
        }
        hardDropped = false;

        // Tetris Blocks 
        const int blocks = 9;
        string colorNames[blocks] = { "red", "yellow", "orange", "pink", "cyan", "green", "purple", "blue", "black" };

        Texture colorTextures[blocks];
        Sprite colorBlocks[blocks];

        for (int i = 0; i < blocks; ++i)
        {
            colorTextures[i].loadFromFile("Blocks/" + colorNames[i] + ".png");
            colorBlocks[i].setTexture(colorTextures[i]);
        }
       
        // Set Block Textures
        const Texture* redTex = &colorTextures[0];          // RED
        const Texture* yellowTex = &colorTextures[1];       // YELLOW
        const Texture* orangeTex = &colorTextures[2];       // ORANGE
        const Texture* pinkTex = &colorTextures[3];         // PINK
        const Texture* cyanTex = &colorTextures[4];         // CYAN
        const Texture* greenTex = &colorTextures[5];        // GREEN
        const Texture* purpleTex = &colorTextures[6];       // PURPLE
        const Texture* blueTex = &colorTextures[7];         // BLUE
        const Texture* blackTex = &colorTextures[8];        // BLACK

        // Drawing the title block
        RectangleShape titleArea(Vector2f(screenWidth, screenHeight / 4));
        titleArea.setPosition(0, 0);
        titleArea.setFillColor(Color(15, 15, 40));
        window.draw(titleArea);

        // Draw game grid background
        RectangleShape gameArea(Vector2f(cellSize * COLS, cellSize * ROWS));
        gameArea.setPosition(0, titleBarHeight);
        gameArea.setFillColor(Color(15, 15, 40)); // Slightly darker grey
        window.draw(gameArea);

        // Draw side panel background
        float sidePanelWidth = screenWidth - (cellSize * COLS);
        RectangleShape sidePanel(Vector2f(sidePanelWidth, screenHeight));
        sidePanel.setPosition(cellSize* COLS, titleBarHeight);
        sidePanel.setFillColor(Color(15, 15, 40)); // Deep navy tone
        window.draw(sidePanel);

        Texture titleTexture;
        Sprite titleSprite;

        if (showingHighscores) // Highscores Screen
        {
            scoreManager.drawHighscores(window, font, screenWidth, screenHeight);
        }
        if (showRules) // Rules Screen
        {
            drawRulesScreen(window, font);
        }

        int spriteY = 0;
        if (showMenu && !gameOver && !showingHighscores && !showRules)
        {
            mainMenu.draw(window);

            titleTexture.loadFromFile("Tetris Logo.png");
            titleSprite.setTexture(titleTexture);
            titleSprite.setScale(0.5f, 0.5f);
            spriteY = 20;
        }
        else
        {
            titleTexture.loadFromFile("Tetris Title.png");
            titleSprite.setTexture(titleTexture);
            titleSprite.setScale(1.25f, 1.25f);
            spriteY = 10;
            if (showingHighscores)
            {
                titleSprite.setScale(0.45f, 0.45f);
                spriteY = 25;
            }
            if (showRules)
            {
                titleSprite.setScale(0.4f, 0.4f);
                spriteY = 15;
            }
        }
        titleSprite.setPosition((screenWidth - titleSprite.getGlobalBounds().width) / 2, spriteY);

        window.draw(titleSprite);

        if(!showMenu)
        {
            // Section: NEXT Tetromino
            Text nextLabel("NEXT", font, 27);
            nextLabel.setFillColor(Color(180, 180, 255));
            nextLabel.setPosition(cellSize* COLS + 20, 10 + titleBarHeight);
            window.draw(nextLabel);

            // Dummy next tetromino box
            RectangleShape nextBox(Vector2f(6 * cellSize, 6 * cellSize));
            nextBox.setPosition(cellSize * COLS + 20, 60 + titleBarHeight);
            nextBox.setFillColor(Color(40, 40, 70));
            nextBox.setOutlineColor(Color::White);
            nextBox.setOutlineThickness(2);
            window.draw(nextBox);

            if(!startCountdown)
            {
                // Draw the next Tetromino on its box
                string* nextShape = t.selectTetro(nextTetroChar);

                int nextCellSize = cellSize;
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        if (nextShape[i][j] != '.') {
                            RectangleShape block(Vector2f(nextCellSize - 2, nextCellSize - 2));
                            block.setPosition(nextCellSize * COLS + 20 + j * nextCellSize + nextCellSize / 2,
                                nextBox.getPosition().y + 10 + i * nextCellSize + nextCellSize / 2);

                            // Same color logic
                            if (nextTetroChar == 'L') block.setTexture(orangeTex);
                            else if (nextTetroChar == 'J') block.setTexture(blueTex);
                            else if (nextTetroChar == 'I') block.setTexture(cyanTex);
                            else if (nextTetroChar == 'S') block.setTexture(greenTex);
                            else if (nextTetroChar == 'Z') block.setTexture(redTex);
                            else if (nextTetroChar == 'T') block.setTexture(pinkTex);
                            else if (nextTetroChar == 'O') block.setTexture(yellowTex);

                            window.draw(block);
                        }
                    }
                }
            }

            // Section: Score display
            Text scoreLabel("SCORE", font, 27);
            scoreLabel.setFillColor(Color(255, 220, 100));
            scoreLabel.setPosition(cellSize * COLS + 20, 250 + titleBarHeight);
            window.draw(scoreLabel);

            Text scoreValue(to_string(gameScore), font, 25);
            scoreValue.setFillColor(Color::White);
            scoreValue.setPosition(cellSize * COLS + 20, 300 + titleBarHeight);
            window.draw(scoreValue);

            // Section: Level display
            Text levelLabel("LEVEL", font, 27);
            levelLabel.setFillColor(Color(255, 220, 100));
            levelLabel.setPosition(cellSize * COLS + 20, 350 + titleBarHeight);
            window.draw(levelLabel);

            Text levelValue(to_string(gameLevel), font, 25);
            levelValue.setFillColor(Color::White);
            levelValue.setPosition(cellSize * COLS + 20, 400 + titleBarHeight);
            window.draw(levelValue);
            
            // Section: Level display
            Text linesLabel("LINES", font, 27);
            linesLabel.setFillColor(Color(255, 220, 100));
            linesLabel.setPosition(cellSize * COLS + 20, 450 + titleBarHeight);
            window.draw(linesLabel);

            Text linesValue(to_string(lineCount), font, 25);
            linesValue.setFillColor(Color::White);
            linesValue.setPosition(cellSize * COLS + 20, 500 + titleBarHeight);
            window.draw(linesValue);

            // Optional: Divider line
            /*RectangleShape divider(Vector2f(sidePanelWidth - 40, 2));
            divider.setFillColor(Color(100, 100, 150));
            divider.setPosition(cellSize * COLS + 20, 450 + titleBarHeight);
            window.draw(divider);*/
        }

        if(!showMenu && !startCountdown)   // Draw the blocks if game is being played
        {
            for (int i = 0; i < ROWS; i++)
            {
                for (int j = 0; j < COLS; j++)
                {
                    if (gameGrid[i][j] != '.')
                    {
                        RectangleShape cell(Vector2f(cellSize - 1, cellSize - 1));
                        cell.setPosition(cellSize * j, cellSize * i + titleBarHeight);

                        if (i == ROWS - 1 || j == 0 || j == COLS - 1)
                        {
                            gameGrid[i][j] = 'B';
                            if (gameGrid[i][j] == 'B') cell.setTexture(blackTex);
                        }
                        else if (gameGrid[i][j] == 'L') cell.setTexture(orangeTex);  // Orange
                        else if (gameGrid[i][j] == 'J') cell.setTexture(blueTex);    // Blue
                        else if (gameGrid[i][j] == 'I') cell.setTexture(cyanTex);    // Cyan
                        else if (gameGrid[i][j] == 'S') cell.setTexture(greenTex);   // Green
                        else if (gameGrid[i][j] == 'Z') cell.setTexture(redTex);     // Red
                        else if (gameGrid[i][j] == 'T') cell.setTexture(pinkTex);    // Purple
                        else if (gameGrid[i][j] == 'O') cell.setTexture(yellowTex);  // Yellow
                        else if (gameGrid[i][j] == 'P') cell.setTexture(pinkTex);    // Optional - Pink if used

                        window.draw(cell);
                    }
                }
            }
        } 

        if (gameOver || pauseGame || startCountdown) // Darken the game on being stopped
        {
            RectangleShape overlay(Vector2f(screenWidth, screenHeight - titleBarHeight));
            overlay.setPosition(0, titleBarHeight);
            overlay.setFillColor(Color(0, 0, 0, 200)); // 150 = semi-transparent black
            window.draw(overlay);
        }

        if (pauseGame) // Game Paused Screen
        {
            Text pauseText("GAME PAUSED", font, 45);
            pauseText.setFillColor(Color::Green);
            pauseText.setPosition((screenWidth - pauseText.getGlobalBounds().width) / 2, titleBarHeight + 150);
            window.draw(pauseText);

            Text musicText("Game Music : " + to_string(musicValue), font, 25);
            musicText.setFillColor(Color(152, 251, 152));
            musicText.setPosition((screenWidth - musicText.getGlobalBounds().width) / 2, titleBarHeight + 260);
            window.draw(musicText);

            Text musicControls("Controls : Up/Down", font, 12.5);
            musicControls.setFillColor(Color::White);
            musicControls.setPosition((screenWidth - musicControls.getGlobalBounds().width) / 2, titleBarHeight + 300);
            window.draw(musicControls);
            
            Text soundText("Sound Effects : " + to_string(soundValue), font, 25);
            soundText.setFillColor(Color(152, 251, 152));
            soundText.setPosition((screenWidth - soundText.getGlobalBounds().width) / 2, titleBarHeight + 340);
            window.draw(soundText);

            Text soundControls("Controls : Left/Right", font, 12.5);
            soundControls.setFillColor(Color::White);
            soundControls.setPosition((screenWidth - soundControls.getGlobalBounds().width) / 2, titleBarHeight + 380);
            window.draw(soundControls);
            
            Text resumeText("Press Enter to resume", font, 15);
            resumeText.setFillColor(Color(180, 180, 180));
            resumeText.setPosition((screenWidth - resumeText.getGlobalBounds().width) / 2, screenHeight - 80);
            window.draw(resumeText);
        }

        // Handle Game Over Screen

        if (gameOver && gameOver.showGameOverText)
        {
            gameMusic.stop();
            playCountdownMusic = true;

            if (playGameOverMusic)
            {
                gameOverMusic.play();
                playGameOverMusic = false;
            }
            Text gameOverText("GAME OVER", font, 50);
            gameOverText.setFillColor(Color::Red);
            gameOverText.setStyle(Text::Bold);
            gameOverText.setPosition((screenWidth - gameOverText.getGlobalBounds().width) / 2, 
                (screenHeight - gameOverText.getGlobalBounds().left) / 2);
            window.draw(gameOverText);

            // Wait for 3 seconds before showing name input
            if (gameOver.gameOverClock.getElapsedTime().asSeconds() >= 3.0f)
            {
                gameOver.showGameOverText = false;
                gameOver.nameInputReady = true;
                gameOver.waitingForNameConfirm = true;  // Now we're waiting for Enter
            }
        }

        // Handle Countdown Screen 

        if (startCountdown)
        {
            if(playCountdownMusic)
            {
                countdownMusic.play();
                playCountdownMusic = false;
            }

            Text countdownText(" ", font, 80);
            countdownText.setFillColor(Color(72, 145, 220));
            countdownText.setStyle(Text::Bold);
            
            if (countdownClock.getElapsedTime().asSeconds() < 1.0f)
            {
                countdownText.setString("3");
            }
            else if (countdownClock.getElapsedTime().asSeconds() < 2.0f && countdownClock.getElapsedTime().asSeconds() >= 1.0f)
            {
                countdownText.setString("2");
            }
            else if (countdownClock.getElapsedTime().asSeconds() < 3.0f && countdownClock.getElapsedTime().asSeconds() >= 2.0f)
            {
                countdownText.setString("1");
            }
            else if (countdownClock.getElapsedTime().asSeconds() < 4.0f && countdownClock.getElapsedTime().asSeconds() >= 3.0f)
            {
                countdownText.setString("GO!");
            }
            else { startCountdown = false; }

            countdownText.setPosition((screenWidth - countdownText.getGlobalBounds().width) / 2,
                (screenHeight - countdownText.getGlobalBounds().left) / 2);
            window.draw(countdownText);
        }

        // Handle The Name Input 

        if (gameOver.nameInputReady)
        {
            scoreManager.drawNameInput(window, font, screenWidth, titleBarHeight);
        }
        if (scoreManager.isNameSaved())
        {
            scoreManager.saveScoreToFile();
            gameOver = false;
            gameGrid.clearGrid();
            showMenu = true;
            gameOver.nameInputReady = false; // reset for next round
        }

        window.display();          
    }
}

void shuffleBag(char bag[], int size, char lastPiece)
{
    // Fill the bag with standard order
    char pieces[] = { 'I', 'J', 'L', 'O', 'S', 'T', 'Z' };
    for (int i = 0; i < size; i++)
        bag[i] = pieces[i];

    // Fisher–Yates shuffle
    for (int i = size - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        swap(bag[i], bag[j]);
    }

    // Ensure first piece isn't same as lastPiece
    if (bag[0] == lastPiece)
    {
        // Find a different piece in the bag to swap with
        for (int i = 1; i < size; i++)
        {
            if (bag[i] != lastPiece)
            {
                swap(bag[0], bag[i]);
                break;
            }
        }
    }
}
void fillTetromino(bool (*tetroFilled)[4], string* tetro)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            tetroFilled[i][j] = (tetro[i][j] == '.') ? false : true;
        }
    }
}
void rotateTetromino(string* tetro, bool (*tetroFilled)[4], int row, int col)
{
    string newTetro[4];
    bool newFilled[4][4] = { false };

    for (int i = 0; i < 4; i++)
        newTetro[i] = "....";

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            newTetro[i][j] = tetro[3 - j][i];
            newFilled[i][j] = tetroFilled[3 - j][i];
        }
    }

    for (int i = 0; i < 4; i++)
    {
        tetro[i] = newTetro[i];
        for (int j = 0; j < 4; j++)
        {
            tetroFilled[i][j] = newFilled[i][j];
        }
    }
}
void removeLineAndUpdateGrid(Grid& grid, int rowIndex)
{
    for (int i = rowIndex; i > 0; i--)
    {
        for (int j = 1; j < COLS - 1; j++)
        {
            grid[i][j] = grid[i - 1][j];
        }
    }

    for (int j = 1; j < COLS - 1; j++)
    {
        grid[1][j] = '.';
    }
}
void checkLineClearAndUpdateScore(Grid& grid, Music& music, int& totalLines, int& score, int& level)
{
    int lineCount = 0;

    for (int i = 1; i < ROWS - 1; i++)
    {
        bool isFull = true;

        for (int j = 1; j < COLS - 1; j++)
        {
            if (grid[i][j] == '.')
            {
                isFull = false;
                break;
            }
        }

        if (isFull)
        {
            music.play();
            removeLineAndUpdateGrid(grid, i);
            lineCount++;
            totalLines++;
            if (totalLines % 10 == 0) level++;
            i--; // Important: recheck same row after shifting down
        }
    }

    if (lineCount == 1) score += 40 * level;
    else if (lineCount == 2) score += 100 * level;
    else if (lineCount == 3) score += 300 * level;
    else if (lineCount == 4) score += 1200 * level;
}
bool checkGameOver(Grid& grid, bool (*tetroFilled)[4], int row, int col)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (tetroFilled[i][j])
            {
                int gridRow = row + i;
                int gridCol = col + j;

                // Ignore border or out-of-bound checks
                if (gridRow >= 0 && gridRow < ROWS && gridCol > 0 && gridCol < COLS - 1)
                {
                    if (grid[gridRow][gridCol] != '.')  // Already filled
                        return true;
                }
            }
        }
    }
    return false;
}
string* copyString(string* src) 
{
    string* dest = new string[4];
    for (int i = 0; i < 4; ++i)
        dest[i] = src[i];
    return dest;
}
void drawRulesScreen(RenderWindow& window, Font font) 
{
    int posY = 100;
    int posX = 50;

    Text heading("GAME RULES", font, 20);
    heading.setFillColor(Color::White);
    heading.setPosition((screenWidth - heading.getGlobalBounds().width) / 2, 70);
    window.draw(heading);

    Text Headings(" ", font, 15);
    Headings.setFillColor(Color(72, 145, 220));
    Headings.setPosition(posX, posY);
    Headings.setString(
        "CONTROLS\n"
        "\n\n\n\n\n\n\n\n\n"
        "SCORING SCHEME\n"
        "\n\n\n\n\n\n\n"
        "LEVELING UP\n\n\n\n"
        "GAME OVER\n\n\n\n"
    );
    window.draw(Headings);

    Text bodyRight(" ", font, 15);
    bodyRight.setFillColor(Color(135, 206, 250));
    bodyRight.setPosition(posX + 150, posY);
    bodyRight.setString(
        "\n\n"
        ":  Right Arrow\n"
        ":  Left Arrow\n"
        ":  Down Arrow\n"
        ":  Spacebar\n"
        ":  Up Arrow\n"
        ":  Enter\n"
        ":  M"
        "\n\n\n\n"
        "POINTS\n"
        "40  x  level\n"
        "100  x  level\n"
        "300  x  level\n"
        "1200  x  level\n\n"
    );
    window.draw(bodyRight);

    Text body(" ", font, 15);
    body.setFillColor(Color(135, 206, 250));
    body.setPosition(posX, posY);
    body.setString(
        "\n\n"
        "Move Right\n"
        "Move Left\n"
        "Soft Drop\n"
        "Hard Drop\n"
        "Rotate\n"
        "Pause/Resume\n"
        "Mute"
        "\n\n\n\n"
        "LINES CLEARED\n"
        "1 - Single\n"
        "2 - Double\n"
        "3 - Triple\n"
        "4 - Tetris\n\n"
        "\n\n"
        "- Level increases by 1 for every 10 lines cleared.\n\n"
        "\n\n"
        "- Game ends if a new tetromino cannot be placed\n"
        "- It cannot move left or right due to blockage."
    ); 
    window.draw(body);

    Text footer("Press Enter to go back", font, 12.5);
    footer.setFillColor(Color(180, 180, 180));
    footer.setPosition((screenWidth - footer.getGlobalBounds().width) / 2, screenHeight - 40);
    window.draw(footer);
}
void loadNextTetromino(char& currentTetroChar, char& nextTetroChar, char& lastTetroChar, int& bagIndex)
{
    currentTetroChar = nextTetroChar;

    if (bagIndex >= 7)
    {
        lastTetroChar = tetrominoBag[6]; // last tetromino from old bag
        shuffleBag(tetrominoBag, 7, lastTetroChar);
        bagIndex = 0;
    }

    nextTetroChar = tetrominoBag[bagIndex];
    bagIndex++;
}
void loadGameOverSate(GameOver& gameOver, Manager& scoreManager, bool& showMenu, int& gameScore, int& gameLevel)
{
    showMenu = false;
    gameOver = true;

    scoreManager.setScoreAndLevel(gameScore, gameLevel);
    scoreManager.startNameEntry();

    gameOver.showGameOverText = true;
    gameOver.gameOverClock.restart();
}