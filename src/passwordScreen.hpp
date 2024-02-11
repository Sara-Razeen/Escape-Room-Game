#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "LTexture.hpp"
using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;


class PassScreen
{
public:
    PassScreen();
    ~PassScreen();
    void run(const string);
    bool isAccessGranted() const
    {
        return accessGranted;
    }

private:
    SDL_Window *mWindow;
    SDL_Renderer *mRenderer;
    TTF_Font *mFont;
    LTexture mPromptTextTexture;
    LTexture mInputTextTexture;
    LTexture mBackgroundTexture;
    bool renderInputText;
    bool quit;
    bool accessGranted;

    void initialize();
    void loadMedia();
    void handleEvents(bool &quit, std::string &inputText, bool &renderText, SDL_Color textColor,const string);
    void render();
    void cleanup();
};

PassScreen::PassScreen()
    : mWindow(NULL), mRenderer(NULL), mFont(NULL), renderInputText(true), quit(false), accessGranted(false)
{
}

PassScreen::~PassScreen()
{
    cleanup();
}

void PassScreen::initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
    {
        cout << "Warning: Linear texture filtering not enabled!" << endl;
    }

    mWindow = SDL_CreateWindow("Password Screen", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (mWindow == NULL)
    {
        cout << "Window could not be created! SDL Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (mRenderer == NULL)
    {
        cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() == -1)
    {
        cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
        exit(EXIT_FAILURE);
    }
}

void PassScreen::loadMedia()
{
    mFont = TTF_OpenFont("Harting_plain.ttf", 35);
    if (mFont == NULL)
    {
        cout << "Failed to load lazy font! SDL_ttf Error: " << TTF_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    SDL_Color textColor = {255, 255, 255, 255};

    if (!mBackgroundTexture.loadFromFile(mRenderer, "images/passwordBg.png"))
    {
        cout << "Failed to load background image!" << endl;
        exit(EXIT_FAILURE);
    }

    if (!mPromptTextTexture.loadFromRenderedText(mRenderer, mFont, "Enter Password", textColor))
    {
        cout << "Failed to render prompt text!" << endl;
        exit(EXIT_FAILURE);
    }
    mInputTextTexture.loadFromRenderedText(mRenderer, mFont, " ", textColor);
}

void PassScreen::handleEvents(bool &quit, std::string &inputText, bool &renderText, SDL_Color textColor,const string expectedPassword)
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            quit = true;
        }
        else
        {
            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_BACKSPACE && inputText.length() > 0)
                {
                    inputText.pop_back();
                    renderText = true;
                }
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit = true; 
                }
                else if (e.key.keysym.sym == SDLK_RETURN)
                {
                    if (inputText == expectedPassword)
                    {
                        renderInputText = false;
                        mPromptTextTexture.free();
                        mInputTextTexture.free(); 
                        mPromptTextTexture.loadFromRenderedText(mRenderer, mFont, "Access Granted", textColor);
                        renderText = true;
                        accessGranted = true;
                        quit = true;
                    }
                    else
                    {
                        renderInputText = false;
                        mPromptTextTexture.free();
                        mInputTextTexture.free(); 
                        mPromptTextTexture.loadFromRenderedText(mRenderer, mFont, "Access Denied", textColor);
                        renderText = true;
                        accessGranted = false;
                        quit = true;
                    }
                }
                else if (e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL)
                {
                    SDL_SetClipboardText(inputText.c_str());
                }
                else if (e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL)
                {
                    char *tempText = SDL_GetClipboardText();
                    inputText = tempText;
                    SDL_free(tempText);
                    renderText = true;
                }
            }
            else if (e.type == SDL_TEXTINPUT)
            {
                if (!(SDL_GetModState() & KMOD_CTRL && (e.text.text[0] == 'c' || e.text.text[0] == 'C' || e.text.text[0] == 'v' || e.text.text[0] == 'V')))
                {
                    inputText += e.text.text;
                    renderText = true;
                }
            }
        }
    }
}

void PassScreen::render()
{
    SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(mRenderer);

    mBackgroundTexture.render(mRenderer, 0, 0);

    mPromptTextTexture.render(mRenderer, (SCREEN_WIDTH - mPromptTextTexture.getWidth()) / 12, 100);
    if (renderInputText)
    {
        mInputTextTexture.render(mRenderer, (SCREEN_WIDTH - mInputTextTexture.getWidth()) / 9, 200);
    }

    SDL_RenderPresent(mRenderer);
}

void PassScreen::cleanup()
{
    mPromptTextTexture.free();
    mInputTextTexture.free();
    mBackgroundTexture.free();
    TTF_CloseFont(mFont);
    mFont = NULL;
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    mWindow = NULL;
    mRenderer = NULL;
    TTF_Quit();
    IMG_Quit();
}

void PassScreen::run(const string pass)
{
    initialize();
    loadMedia();

    SDL_Color textColor = {255, 255, 255, 255};
    std::string inputText = "";
    mInputTextTexture.loadFromRenderedText(mRenderer, mFont, inputText.c_str(), textColor);
    SDL_StartTextInput();

    while (!quit && !accessGranted)
    {
        bool renderText = false;
        handleEvents(quit, inputText, renderText, textColor,pass);

        if (renderText)
        {
            if (inputText != "")
            {
                mInputTextTexture.loadFromRenderedText(mRenderer, mFont, inputText.c_str(), textColor);
            }
            else
            {
                mInputTextTexture.loadFromRenderedText(mRenderer, mFont, " ", textColor);
            }
        }

        render();
    }

    SDL_Delay(1000);

    SDL_StopTextInput();
    cleanup();
}
