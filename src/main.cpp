#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include "passwordScreen.hpp"
#include "maze.hpp"
#include "decoding.hpp"
#include "riddles.hpp"
#include "hiddenObjects.hpp"

using namespace std;
PassScreen passwordScreen;
string door2_Pass, door3_Pass, door4_Pass = "Egress";
class MainScreen
{
public:
    MainScreen() : gWindow(nullptr), gRenderer(nullptr), gBackgroundTexture(nullptr)
    {
        for (int i = 0; i < NUM_DOORS; ++i)
        {
            doorStates[i] = DoorState::Unlocked;
        }
    }

    ~MainScreen()
    {
        close();
    }

    bool init()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            cout << "SDL could not initialize! SDL_Error:" << SDL_GetError() << endl;
            return false;
        }

        gWindow = SDL_CreateWindow("ESCAPE ROOM", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == nullptr)
        {
            cout << "Window could not be created! SDL_Error:" << SDL_GetError();
            return false;
        }

        gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
        if (gRenderer == nullptr)
        {
            cout << "Renderer could not be created! SDL_Error:" << SDL_GetError() << endl;
            close();
            return false;
        }

        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags))
        {
            cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
            return false;
        }

        gBackgroundTexture = loadTexture("images/mainBg.png");
        if (gBackgroundTexture == nullptr)
        {
            return false;
        }

        return true;
    }
    void renderLost()
    {
        SDL_Texture *lostTexture = loadTexture("images/youLost.png"); // Replace with the actual image path
        renderTexture(lostTexture, 0, 0);
        SDL_RenderPresent(gRenderer);
        SDL_Delay(2000);
        close();
    }
    void render()
    {
        
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(gRenderer);


        renderTexture(gBackgroundTexture, 0, 0);

        SDL_Rect door1 = {30, 285, 140, 300};
        SDL_Rect door2 = {225, 285, 142, 300};
        SDL_Rect door3 = {420, 285, 145, 300};
        SDL_Rect door4 = {628, 285, 145, 300};

        SDL_RenderPresent(gRenderer);
    }
    void handleMouseClick(int x, int y)
    {
        if (isPointInsideRect(x, y, 30, 285, 140, 300) && doorStates[0] == DoorState::Unlocked)
        {
            cout << "Clicked on door 1!\n";
            Decoding Decoding;
            door2_Pass = Decoding.run();
            if (!Decoding.successfulDecoding())
            {
                renderLost();
            }
            
            doorStates[0] = DoorState::Locked;
        }
        else if (isPointInsideRect(x, y, 225, 285, 142, 300) && doorStates[1] == DoorState::Unlocked)
        {
            cout << "Clicked on door 2!\n";
            showPasswordScreen(door2_Pass);
            if (passwordScreen.isAccessGranted())
            {
                door3_Pass = runObjectFinding();
                if (!wonLabGame())
                {
                    renderLost();
                }
            }
                doorStates[1] = DoorState::Locked;
            
        }
        else if (isPointInsideRect(x, y, 420, 285, 145, 300) && doorStates[2] == DoorState::Unlocked)
        {
            cout << "Clicked on door 3!\n";
            showPasswordScreen(door3_Pass);
            if (passwordScreen.isAccessGranted())
            {
                RiddlesGame rGame;
                rGame.run();
                if (!rGame.isWin())
                {
                    renderLost();
                }
            }
            doorStates[2] = DoorState::Locked;
        }
        else if (isPointInsideRect(x, y, 628, 285, 145, 300) && doorStates[3] == DoorState::Unlocked)
        {
            cout << "Clicked on door 4!\n";
            showPasswordScreen(door4_Pass);
            if (passwordScreen.isAccessGranted())
            {
                MazeGame mGame;
                mGame.run();
                if (!mGame.WonOrLost())
                {
                    renderLost();
                }
            }
            doorStates[3] = DoorState::Locked;
        }
    }

    bool isPointInsideRect(int x, int y, int rectX, int rectY, int rectWidth, int rectHeight)
    {
        return (x >= rectX && x <= rectX + rectWidth && y >= rectY && y <= rectY + rectHeight);
    }

    void run()
    {
        SDL_Event e;
        bool quit = false;

        while (!quit)
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
                    handleMouseClick(mouseX, mouseY);
                }
            }

            render();
        }
    }

    void close()
    {
        SDL_DestroyTexture(gBackgroundTexture);
        SDL_DestroyRenderer(gRenderer);
        SDL_DestroyWindow(gWindow);
        IMG_Quit();
        SDL_Quit();
    }

private:
    enum DoorState
    {
        Unlocked,
        Locked
    };

    static const int NUM_DOORS = 4;
    DoorState doorStates[NUM_DOORS];
    SDL_Window *gWindow;
    SDL_Renderer *gRenderer;
    SDL_Texture *gBackgroundTexture;
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;

    SDL_Texture *loadTexture(const std::string &path)
    {
        SDL_Texture *newTexture = nullptr;

        SDL_Surface *loadedSurface = IMG_Load(path.c_str());
        if (loadedSurface == nullptr)
        {
            cout << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << endl;
            return nullptr;
        }

        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == nullptr)
        {
            cout << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << endl;
        }

        SDL_FreeSurface(loadedSurface);

        return newTexture;
    }

    void renderTexture(SDL_Texture *texture, int x, int y)
    {
        SDL_Rect renderQuad = {x, y, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(gRenderer, texture, NULL, &renderQuad);
    }

    void showPasswordScreen(const string st)
    {
        passwordScreen = PassScreen();
        passwordScreen.run(st);
    }
};

int main(int argc, char *args[])
{
    MainScreen mainScreen;

    if (!mainScreen.init())
    {
        cout << "Failed to initialize!\n";
        return 1;
    }

    mainScreen.run();

    return 0;
}
