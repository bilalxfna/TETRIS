#pragma once
#include<iostream>
using namespace std;

class Tetromino
{
private:
    string tetro[7][4];
    int ROWS, COLS;
public:

    Tetromino(int r = 0, int c = 0) : ROWS(r), COLS(c)
    {
        // I shape
        tetro[0][0] = "....";
        tetro[0][1] = "IIII";
        tetro[0][2] = "....";
        tetro[0][3] = "....";

        // J shape
        tetro[1][0] = "....";
        tetro[1][1] = "..J.";
        tetro[1][2] = "..J.";
        tetro[1][3] = ".JJ.";

        // L shape
        tetro[2][0] = "....";
        tetro[2][1] = "..L.";
        tetro[2][2] = "..L.";
        tetro[2][3] = "..LL";

        // O shape (square)
        tetro[3][0] = "....";
        tetro[3][1] = ".OO.";
        tetro[3][2] = ".OO.";
        tetro[3][3] = "....";

        // S shape
        tetro[4][0] = "....";
        tetro[4][1] = "..SS";
        tetro[4][2] = ".SS..";
        tetro[4][3] = "....";

        // T shape
        tetro[5][0] = "....";
        tetro[5][1] = ".TTT";
        tetro[5][2] = "..T.";
        tetro[5][3] = "....";

        // Z shape
        tetro[6][0] = "....";
        tetro[6][1] = ".ZZ.";
        tetro[6][2] = "..ZZ";
        tetro[6][3] = "....";
    }
    string* selectTetro(char index)
    {
        if (index == 'I') return tetro[0];
        else if (index == 'J') return tetro[1];
        else if (index == 'L') return tetro[2];
        else if (index == 'O') return tetro[3];
        else if (index == 'S') return tetro[4];
        else if (index == 'T') return tetro[5];
        else if (index == 'Z') return tetro[6];
    }


    bool checkLeftBoundary(Grid& grid, bool (*tetroFilled)[4], int row, int col)
    {
        for (int i = 0; i < 4; i++) 
        {
            for (int j = 0; j < 4; j++)
            {
                if (tetroFilled[i][j]) 
                {
                    int gridRow = row + i;
                    int gridCol = col + j - 1;

                    if (gridCol < 0) 
                        return false;

                    if (grid[gridRow][gridCol] != '.')
                        return false;

                    break; 
                }
            }
        }

        return true;
    }

    bool checkRightBoundary(Grid& grid, bool (*tetroFilled)[4], int row, int col)
    {
        for (int i = 0; i < 4; i++) 
        {
            for (int j = 3; j >= 0; j--)
            {
                if (tetroFilled[i][j]) 
                {
                    int gridRow = row + i;
                    int gridCol = col + j + 1;

                    if (gridCol >= COLS) 
                        return false;

                    if (grid[gridRow][gridCol] != '.')
                        return false;

                    break; 
                }
            }
        }

        return true;
    }

    bool checkBottomBoundary(Grid& grid, bool (*tetroFilled)[4], int row, int col)
    {
        for (int i = 0; i < 4; i++) 
        {
            for (int j = 3; j >= 0; j--) 
            {
                if (tetroFilled[j][i]) 
                {
                    int gridRow = row + j + 1;
                    int gridCol = col + i;

                    if (gridRow >= ROWS)
                        return false;

                    if (grid[gridRow][gridCol] != '.')
                        return false;

                    break; 
                }
            }
        }

        return true; 
    }
    bool checkUpperBoundary(Grid& grid, bool (*tetroFilled)[4], int row, int col)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (tetroFilled[i])
                {
                    int gridRow = row + j + 1;
                }
            }
        }
    }


};