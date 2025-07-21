#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace sf;

class Manager
{
private:
    string name;
    int score;
    int level;

    bool enteringName = false;
    bool nameEntered = false;

public:
    Manager() : name(""), score(0), level(1) {}

    void startNameEntry()
    {
        name = "";
        enteringName = true;
        nameEntered = false;
    }

    bool isNameBeingEntered() const { return enteringName; }

    void handleTextInput(Event& ev)
    {
        if (!enteringName) return;

        if (ev.type == Event::TextEntered)
        {
            if (ev.text.unicode == '\b' && !name.empty())
                name.pop_back();
            else if ((ev.text.unicode == '\r' || ev.text.unicode == '\n') && !name.empty())
            {
                for (char& c : name) c = toupper(c);
                enteringName = false;
                nameEntered = true;
            }
            else if (ev.text.unicode < 128 && isprint(ev.text.unicode) && name.length() < 15)
                name += static_cast<char>(ev.text.unicode);
        }
    }

    void drawNameInput(RenderWindow& window, Font& font, int screenWidth, int titleBarHeight)
    {
        if (!enteringName) return;

        RectangleShape box(Vector2f(400, 200));
        box.setPosition((screenWidth - 400) / 2, titleBarHeight + 150);
        box.setFillColor(Color(20, 20, 60));
        box.setOutlineColor(Color::White);
        box.setOutlineThickness(2);
        window.draw(box);

        Text prompt("Enter Your Name:", font, 24);
        prompt.setFillColor(Color::White);
        prompt.setPosition(box.getPosition().x + 20, box.getPosition().y + 20);
        window.draw(prompt);

        Text nameText(name, font, 28);
        nameText.setFillColor(Color(255, 220, 100));
        nameText.setPosition(box.getPosition().x + 20, box.getPosition().y + 70);
        window.draw(nameText);

        Text hint("Press Enter to Save", font, 18);
        hint.setFillColor(Color(180, 180, 180));
        hint.setPosition(box.getPosition().x + 20, box.getPosition().y + 130);
        window.draw(hint);
    }

    bool isNameSaved() const { return nameEntered; }

    void saveScoreToFile()
    {
        ofstream file("scores.txt", ios::app);
        if (file.is_open())
        {
            file << name << " " << score << " " << level << "\n";
            file.close();
            cout << "Saved: " << name << " " << score << " " << level << endl;
        }
        else {
            cout << "Failed to open scores.txt\n";
        }

        nameEntered = false;
    }

    void setScoreAndLevel(int score, int level)
    {
        this->score = score;
        this->level = level;
    }

    void drawHighscores(RenderWindow& window, Font& font, int screenWidth, int screenHeight)
    {
        ifstream file("scores.txt");

        const int MAX_SCORES = 1000;
        string names[MAX_SCORES];
        int scores[MAX_SCORES];
        int levels[MAX_SCORES];
        int count = 0;

        string line;
        while (getline(file, line) && count < MAX_SCORES)
        {
            istringstream iss(line);
            vector<string> tokens;
            string token;

            // Tokenize the line
            while (iss >> token)
                tokens.push_back(token);

            if (tokens.size() < 3)
                continue;

            string scoreStr = tokens[tokens.size() - 2];
            string levelStr = tokens[tokens.size() - 1];

            // Manual validation: check if scoreStr and levelStr are digits
            bool valid = true;
            for (char c : scoreStr) if (!isdigit(c)) valid = false;
            for (char c : levelStr) if (!isdigit(c)) valid = false;
            if (!valid) continue;

            int score = stoi(scoreStr);
            int level = stoi(levelStr);

            string name = tokens[0];  // First word only

            // Convert name to uppercase
            for (char& c : name)
                c = toupper(c);

            names[count] = name;
            scores[count] = score;
            levels[count] = level;
            count++;
        }
        while (count < 10)
        {
            names[count] = "-";
            scores[count] = 0;
            levels[count] = 0;
            count++;
        }
        file.close();

        // Sort scores descending
        for (int i = 0; i < count - 1; ++i) {
            for (int j = 0; j < count - i - 1; ++j) {
                if (scores[j] < scores[j + 1]) {
                    swap(scores[j], scores[j + 1]);
                    swap(names[j], names[j + 1]);
                    swap(levels[j], levels[j + 1]);
                }
            }
        }

        // Background box
        RectangleShape box(Vector2f(500, 700));
        box.setPosition((screenWidth - 500) / 2, (screenHeight - 700) / 2);
        box.setFillColor(Color(20, 20, 60));
        box.setOutlineColor(Color::White);
        box.setOutlineThickness(3);
        window.draw(box);

        Text title("HIGHSCORES", font, 35);
        title.setFillColor(Color::White);
        title.setPosition((screenWidth - title.getGlobalBounds().width) / 2, box.getPosition().y + 75);
        window.draw(title);

        int yOffset = 150;
        int maxDisplay = 10;

        float nameX = box.getPosition().x + 30;
        float scoreX = box.getPosition().x + 230;
        float levelX = box.getPosition().x + 360;

        Text nameHeader("NAME", font, 25);
        nameHeader.setFillColor(Color(72, 145, 220));
        nameHeader.setPosition(nameX, box.getPosition().y + yOffset);
        window.draw(nameHeader);

        Text scoreHeader("SCORE", font, 25);
        scoreHeader.setFillColor(Color(72, 145, 220));
        scoreHeader.setPosition(scoreX, box.getPosition().y + yOffset);
        window.draw(scoreHeader);

        Text levelHeader("LEVEL", font, 25);
        levelHeader.setFillColor(Color(72, 145, 220));
        levelHeader.setPosition(levelX, box.getPosition().y + yOffset);
        window.draw(levelHeader);

        yOffset += 60;

        // Cap to 7 entries if needed
        if (count > 10) count = 10;

        for (int i = 0; i < count; ++i)
        {
            // Name
            Text nameText(names[i], font, 20);
            nameText.setFillColor(Color(135, 206, 250));
            nameText.setPosition(nameX, box.getPosition().y + yOffset);
            window.draw(nameText);

            // Score
            Text scoreText(to_string(scores[i]), font, 20);
            scoreText.setFillColor(Color(135, 206, 250));
            scoreText.setPosition(scoreX, box.getPosition().y + yOffset);
            window.draw(scoreText);

            // Level
            Text levelText(to_string(levels[i]), font, 20);
            levelText.setFillColor(Color(135, 206, 250));
            levelText.setPosition(levelX, box.getPosition().y + yOffset);
            window.draw(levelText);

            yOffset += 40;
        }


        Text hint("Press Enter to return", font, 18);
        hint.setFillColor(Color(180, 180, 180));
        hint.setPosition((screenWidth - hint.getGlobalBounds().width) / 2, screenHeight - 75);
        window.draw(hint);
    }
};