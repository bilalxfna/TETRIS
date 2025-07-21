#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

using namespace sf;
using namespace std;

class Menu
{
private:
    Text options[4];
    int selectedIndex = 0;
    Font font;

public:
    Menu(RenderWindow& window, Font menuFont, int screenHeight, int screenWidth) : font(menuFont)
    {
        string labels[4] = { "Play Game", "Scorecard", "Controls / Rules", "Exit Game" };

        for (int i = 0; i < 4; i++)
        {
            options[i].setFont(font);
            options[i].setString(labels[i]);
            options[i].setCharacterSize(32);
            options[i].setPosition((screenWidth - options[i].getGlobalBounds().width) / 2, 
                screenHeight / 2.0f + 50 + i * 62.5f);
        }

        updateVisuals();
    }

    void updateVisuals()
    {
        for (int i = 0; i < 4; i++)
        {
            if (i == selectedIndex)
            {
                options[i].setFillColor(Color(72, 145, 220));
            }
            else
            {
                options[i].setFillColor(Color(135, 206, 250));
            }
        }
    }

    void draw(RenderWindow& window)
    {
        for (int i = 0; i < 4; i++)
        {
            window.draw(options[i]);
        }
    }

    void moveUp()
    {
        selectedIndex = (selectedIndex + 3) % 4; // Wrap around
        updateVisuals();
    }

    void moveDown()
    {
        selectedIndex = (selectedIndex + 1) % 4;
        updateVisuals();
    }

    int getSelectedIndex() const
    {
        return selectedIndex;
    }
};
