
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <ctime>
#include <chrono>
#include <thread>
#include "DSA.hpp"

using namespace std;

const int sc_Width = 800;
const int sc_Height = 600;
const int CELL_SIZE = 20;
const int ROWS = sc_Height / CELL_SIZE;
const int COLS = sc_Width / CELL_SIZE;

enum class CellState
{
    WALL,
    PATH,
    START,
    END,
};
struct Cell
{
    int row, col;
    CellState state;
    bool visited;
};

class MazeGame
{
private:
    SDL_Window *gWindow;
    SDL_Renderer *gRenderer;
    SDL_Texture *startTexture;
    SDL_Rect startButtonRect;
    SDL_Texture *wonTexture;
    bool Won;

    Cell maze[ROWS][COLS];
    int playerRow, playerCol;
    bool gameStarted;

    chrono::high_resolution_clock::time_point startTime;
    const int gameDurationInSeconds;

    void renderStartScreen()
    {
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(gRenderer);

        if (startTexture != nullptr)
        {
            SDL_RenderCopy(gRenderer, startTexture, nullptr, nullptr);
        }

        SDL_RenderPresent(gRenderer);
    }
    void loadStartScreen()
    {
        string imagePath = "images/MAZE.png";
        SDL_Surface *startImage = IMG_Load(imagePath.c_str());

        if (startImage != nullptr)
        {
            startTexture = SDL_CreateTextureFromSurface(gRenderer, startImage);
            SDL_FreeSurface(startImage);

            startButtonRect.x = sc_Width - 155;
            startButtonRect.y = sc_Height - 153;
            startButtonRect.w = 112;
            startButtonRect.h = 125;
        }
    }
    void handleStartButtonClick(int mouseX, int mouseY)
    {
        if (mouseX >= startButtonRect.x && mouseX < startButtonRect.x + startButtonRect.w &&
            mouseY >= startButtonRect.y && mouseY < startButtonRect.y + startButtonRect.h)
        {

            renderTimer();
            SDL_Delay(1000);
            startTimer();
            Stack<Cell> stack;
            generateMaze(0, 0, stack);
            renderMaze();

            gameStarted = true;
        }
    }
    void startTimer()
    {
        startTime = chrono::high_resolution_clock::now();
    }
    bool isTimeUp()
    {
        auto currentTime = chrono::high_resolution_clock::now();
        auto elapsedSeconds = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();
        return elapsedSeconds >= gameDurationInSeconds;
    }
    void renderMaze()
    {
        SDL_SetRenderDrawColor(gRenderer, 245, 245, 220, 255);
        SDL_RenderClear(gRenderer);

        for (int i = 0; i < ROWS; ++i)
        {
            for (int j = 0; j < COLS; ++j)
            {
                SDL_Rect rect = {j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE};
                switch (maze[i][j].state)
                {
                case CellState::WALL:
                    SDL_SetRenderDrawColor(gRenderer, 139, 69, 19, 255);
                    break;
                case CellState::PATH:
                    SDL_SetRenderDrawColor(gRenderer, 245, 245, 220, 255);
                    break;
                case CellState::START:
                    SDL_SetRenderDrawColor(gRenderer, 0, 70, 0, 255); 
                    break;
                case CellState::END:
                    SDL_SetRenderDrawColor(gRenderer, 119, 0, 0, 255);
                    break;
                }
                SDL_RenderFillRect(gRenderer, &rect);
            }
        }

        SDL_RenderPresent(gRenderer);
    }
    bool isValid(int row, int col)
    {
        return row >= 0 && row < ROWS && col >= 0 && col < COLS && !maze[row][col].visited;
    }
    void generateMaze(int startRow, int startCol, Stack<Cell> &stack)
    {
        srand(static_cast<unsigned>(time(nullptr)));
        stack.push(maze[startRow][startCol]);
        maze[startRow][startCol].visited = true;

        while (!stack.IsEmpty())
        {
            Cell currentCell = stack.Top();
            stack.Pop();

            int row = currentCell.row;
            int col = currentCell.col;

            maze[row][col].state = CellState::PATH;
            renderMaze();

            int neighbors[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
            for (int i = 0; i < 4; ++i)
            {
                int randIndex = rand() % 4;
                swap(neighbors[i], neighbors[randIndex]);
            }

            for (int i = 0; i < 4; ++i)
            {
                int newRow = row + neighbors[i][0] * 2;
                int newCol = col + neighbors[i][1] * 2;

                if (isValid(newRow, newCol))
                {
                    stack.push(maze[newRow][newCol]);
                    maze[newRow][newCol].visited = true;
                    maze[row + neighbors[i][0]][col + neighbors[i][1]].state = CellState::PATH;
                    renderMaze();

                    stack.push(maze[newRow][newCol]);
                }
            }
        }

        maze[1][1].state = CellState::START;
        maze[ROWS - 2][COLS - 2].state = CellState::END;
    }
    void movePlayer(SDL_Keycode key)
    {
        int newRow = playerRow;
        int newCol = playerCol;

        switch (key)
        {
        case SDLK_UP:
            newRow = playerRow - 1;
            break;
        case SDLK_DOWN:
            newRow = playerRow + 1;
            break;
        case SDLK_LEFT:
            newCol = playerCol - 1;
            break;
        case SDLK_RIGHT:
            newCol = playerCol + 1;
            break;
        }

        if (newRow >= 0 && newRow < ROWS && newCol >= 0 && newCol < COLS && maze[newRow][newCol].state != CellState::WALL)
        {
            maze[playerRow][playerCol].state = CellState::PATH;
            playerRow = newRow;
            playerCol = newCol;
            maze[playerRow][playerCol].state = CellState::START;

            renderMaze();
        }
    }
    void renderTimer()
    {
        SDL_SetRenderDrawColor(gRenderer, 159, 89, 39, 255);
        SDL_RenderClear(gRenderer);
        SDL_Color textColor = {245, 245, 220};
        SDL_Surface *textSurface = nullptr;
        SDL_Texture *textTexture = nullptr;

        SDL_Rect textRect;
        textRect.x = sc_Width / 4;
        textRect.y = sc_Height / 4;
        textRect.w = sc_Width / 2;
        textRect.h = sc_Height / 2;

        SDL_RenderPresent(gRenderer);

        TTF_Font *font = TTF_OpenFont("JosefinSans-Bold.ttf", 100);
        if (font == nullptr)
        {
            cout << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << endl;
            return;
        }

        string timerText = "Time: " + to_string(gameDurationInSeconds) + " sec";

        textSurface = TTF_RenderText_Blended(font, timerText.c_str(), textColor);

        if (textSurface == nullptr)
        {
            cout << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << endl;
        }
        else
        {
            textTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
            if (textTexture == nullptr)
            {
                cout << "Unable to create text texture! SDL Error: " << SDL_GetError() << endl;
            }
            else
            {
                SDL_RenderCopy(gRenderer, textTexture, nullptr, &textRect);
                SDL_RenderPresent(gRenderer);
            }
        }

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
        TTF_CloseFont(font);
    }
    void renderWonOrLost()
    {
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(gRenderer);

        SDL_Rect textureRect;
        textureRect.x = 0;
        textureRect.y = 0;
        textureRect.w = sc_Width;
        textureRect.h = sc_Height;

        if (Won)
        {
            SDL_RenderCopy(gRenderer, wonTexture, nullptr, &textureRect);
        }
        SDL_RenderPresent(gRenderer);
        SDL_Delay(2000);
    }

    void loadTextures()
    {
        wonTexture = loadTexture("images/youWon.png");
    }
    SDL_Texture *loadTexture(const std::string &path)
    {
        SDL_Surface *surface = IMG_Load(path.c_str());
        if (surface == nullptr)
        {
            cout << "Failed to load image " << path << "! SDL_Image Error: " << IMG_GetError() << endl;
            return nullptr;
        }

        SDL_Texture *texture = SDL_CreateTextureFromSurface(gRenderer, surface);
        if (texture == nullptr)
        {
            cout << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << endl;
        }

        SDL_FreeSurface(surface);

        return texture;
    }
    void cleanupSDL()
    {
        SDL_DestroyRenderer(gRenderer);
        SDL_DestroyWindow(gWindow);
        TTF_Quit();
    }

public:
    MazeGame() : gWindow(nullptr), gRenderer(nullptr), playerRow(1), playerCol(1), gameStarted(false),Won(false), gameDurationInSeconds(90)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
            exit(1);
        }

        if (TTF_Init() == -1)
        {
            cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
            exit(1);
        }

        gWindow = SDL_CreateWindow("MAZE GAME", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, sc_Width, sc_Height, SDL_WINDOW_SHOWN);
        if (gWindow == nullptr)
        {
            cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
            exit(1);
        }

        gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
        if (gRenderer == nullptr)
        {
            cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
            exit(1);
        }
        for (int i = 0; i < ROWS; ++i)
        {
            for (int j = 0; j < COLS; ++j)
            {
                maze[i][j].row = i;
                maze[i][j].col = j;
                maze[i][j].state = CellState::WALL;
                maze[i][j].visited = false;
            }
        }
    }

    void run()
    {
        bool quit = false;
        SDL_Event e;
        loadStartScreen();
        renderStartScreen();
        SDL_Delay(3000);
        loadTextures();
        while (!quit && !gameStarted)
        {
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
                else if (e.type == SDL_MOUSEBUTTONDOWN)
                {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);

                    handleStartButtonClick(mouseX, mouseY);
                }
            }
        }

        while (!quit && gameStarted)
        {
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
                else if (e.type == SDL_KEYDOWN)
                {
                    switch (e.key.keysym.sym)
                    {
                    case SDLK_UP:
                    case SDLK_DOWN:
                    case SDLK_LEFT:
                    case SDLK_RIGHT:
                        movePlayer(e.key.keysym.sym);
                        if (playerRow == ROWS - 2 && playerCol == COLS - 2)
                        {
                            Won = true;
                            quit = true;
                        }
                        break;
                    }
                }
            }

            if (isTimeUp())
            {
                
                break;
                
            }

            auto currentTime = chrono::high_resolution_clock::now();
            auto elapsedSeconds = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();

            if (elapsedSeconds == gameDurationInSeconds - 10)
            {
                const SDL_MessageBoxButtonData buttons[] = {
                    {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "OK"}};
                const SDL_MessageBoxData messageboxdata = {
                    SDL_MESSAGEBOX_INFORMATION,
                    gWindow,
                    "Time Alert",
                    "10 seconds left!",
                    SDL_arraysize(buttons),
                    buttons,
                    nullptr};
                int buttonid;
                SDL_ShowMessageBox(&messageboxdata, &buttonid);
            }

            renderMaze();

            if (Won)
            {
                renderWonOrLost();
                SDL_Delay(1000);
                SDL_Quit();
            }
        }
        if (!Won)
        {
            cleanupSDL();
        }
        
    }
    bool WonOrLost()
    {
        return Won;
    }
    ~MazeGame()
    {
        cleanupSDL();
    }
};
