#pragma once
#include<iostream>
using namespace std;

class Grid
{
private:
    char** grid;
    int ROWS, COLS;
public:
    Grid(int r = 0, int c = 0) : ROWS(r), COLS(c)
    {
        grid = new char* [ROWS];
        for (int i = 0; i < ROWS; i++)
        {
            grid[i] = new char[COLS];
        }
    }
    int getRows() { return ROWS; }
    int getCols() { return COLS; }

    char* operator[](int index)
    {
        return grid[index];
    }
    void placeTetromino(string* tetromino, int row, int col)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                int gridRow = row + i;
                int gridCol = col + j;

                // Bounds check before accessing grid
                if (gridRow >= 0 && gridRow < ROWS && gridCol >= 0 && gridCol < COLS)
                {
                    if (grid[gridRow][gridCol] == '.' && tetromino[i][j] != '.')
                    {
                        grid[gridRow][gridCol] = tetromino[i][j];
                    }
                }
            }
        }
    }

    void clearGrid()
    {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (i == ROWS - 1 || j == 0 || j == COLS - 1)
                    grid[i][j] = 'B'; // border
                else
                    grid[i][j] = '.'; // empty
            }
        }

    }
    void clearTetromino(bool tetroFilled[][4], int row, int col)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if(tetroFilled[i][j])
                {
                    grid[row + i][col + j] = '.';
                    
                }
            }
        }
    }

};