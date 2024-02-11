#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <cstdlib>
#include <ctime>
#include "imageViewer.hpp"
#include "LTexture.hpp"

using namespace std;

const int width = 768;
const int height = 512;

SDL_Window *window;
SDL_Renderer *renderer;
TTF_Font *font;
SDL_Texture *startTexture;
SDL_Rect startButtonRect;
bool gameStarted = false, won = false;
class PotionMixingGame
{
private:
    unordered_map<string, string> potions;
    SDL_Rect inputRect;
    LTexture gPromptTextTexture;
    LTexture gInputTextTexture;
    LTexture gBackgroundTexture;
    string selectedPotionCode;
    void loadMedia();
    void renderText(string, int, int, bool, int);
    void renderWonOrLost();
    string getUserInputSDL();

public:
    PotionMixingGame();
    ~PotionMixingGame();
    void initializeSDL();
    void readPotionsFromFile(const string &filename);
    void displayAvailablePotions();
    void createPotion(const string &chosenPotion);
    void IngredientSequenceForChosen(const string &chosenPotion);
    string getRandomPotion();
    string run();
    void cleanUp();
    bool WonOrLost();
};

PotionMixingGame::PotionMixingGame()
{
    initializeSDL();
    readPotionsFromFile("textFiles/potions.txt");
    loadMedia();
}

PotionMixingGame::~PotionMixingGame()
{
    cleanUp();
}

void PotionMixingGame::initializeSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() < 0)
    {
        cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("Potion Mixing Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!window)
    {
        cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    font = TTF_OpenFont("JosefinSans-Bold.ttf", 25);
    if (!font)
    {
        cout << "Failed to load font: " << TTF_GetError() << endl;
        exit(EXIT_FAILURE);
    }
}

void PotionMixingGame::loadMedia()
{
    if (!gBackgroundTexture.loadFromFile(renderer, "images/Mystic Mix.png"))
    {
        cout << "Failed to load background texture!" << endl;
        exit(EXIT_FAILURE);
    }
}

void PotionMixingGame::renderText(string text, int x, int y, bool wrapText, int fontSize)
{
    SDL_Color textColor = {255, 255, 255};
    TTF_Font *customFont;
    if (fontSize > 0)
    {
        customFont = TTF_OpenFont("JosefinSans-Bold.ttf", fontSize);
        if (!customFont)
        {
            cout << "Failed to load custom font: " << TTF_GetError() << endl;
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        customFont = font;
    }

    if (wrapText)
    {
        int wrapLength = 62;
        vector<string> wrappedLines;

        size_t startPos = 0;
        while (startPos < text.length())
        {
            size_t endPos = min(startPos + wrapLength, text.length());
            size_t lastSpace = text.find_last_of(" ", endPos);
            if (lastSpace != std::string::npos && lastSpace > startPos)
            {
                endPos = lastSpace;
            }
            wrappedLines.push_back(text.substr(startPos, endPos - startPos));
            startPos = endPos + 1;
        }

        int yOffset = y;
        for (const auto &line : wrappedLines)
        {
            gPromptTextTexture.loadFromRenderedText(renderer, customFont, line, textColor);
            gPromptTextTexture.render(renderer, x, yOffset);
            yOffset += gPromptTextTexture.getHeight();
        }
    }
    else
    {
        gPromptTextTexture.loadFromRenderedText(renderer, customFont, text, textColor);
        gPromptTextTexture.render(renderer, x, y);
    }
    if (fontSize > 0)
    {
        TTF_CloseFont(customFont);
    }
}

string PotionMixingGame::getUserInputSDL()
{
    bool renderText = true;
    string inputText;

    SDL_StartTextInput();

    bool inputCompleted = false;
    SDL_Event e;
    gBackgroundTexture.render(renderer, 0, 0);
    SDL_RenderPresent(renderer);
    while (!inputCompleted)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                cleanUp();
                exit(EXIT_SUCCESS);
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER)
                {
                    inputCompleted = true;
                }
                else if (e.key.keysym.sym == SDLK_BACKSPACE && inputText.length() > 0)
                {
                    inputText.pop_back();
                    renderText = true;
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

        if (renderText)
        {
            SDL_RenderClear(renderer);

            gBackgroundTexture.render(renderer, 0, 0);
            gPromptTextTexture.render(renderer, 100, 200);

            if (inputText != "")
            {
                gInputTextTexture.loadFromRenderedText(renderer, font, inputText, {255, 255, 255});
                gInputTextTexture.render(renderer, 250, 250);
            }
            else
            {
                gInputTextTexture.loadFromRenderedText(renderer, font, " ", {255, 255, 255});
                gInputTextTexture.render(renderer, 200, 250);
            }

            SDL_RenderPresent(renderer);
            renderText = false;
        }
    }

    SDL_StopTextInput();
    return inputText;
}

void PotionMixingGame::readPotionsFromFile(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "Error: Unable to open file " << filename << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    while (getline(file, line))
    {
        size_t delimiterPos = line.find("!");
        if (delimiterPos != string::npos)
        {
            string scenario = line.substr(0, delimiterPos);
            string code = line.substr(delimiterPos + 1);

            potions[scenario] = code;
        }
    }

    file.close();
}

void PotionMixingGame::displayAvailablePotions()
{
    ImageViewer imageViewer(renderer);

    imageViewer.addImage("images/2.png");
    imageViewer.addImage("images/3.png");
    imageViewer.addImage("images/4.png");
    imageViewer.addImage("images/5.png");
    imageViewer.addImage("images/6.png");
    imageViewer.addImage("images/7.png");

    imageViewer.displayCurrentImage();

    bool quit = false;
    SDL_Event e;
    Uint32 startTime = SDL_GetTicks();
    Uint32 duration = 10000;
    while (!quit && (SDL_GetTicks() - startTime) < duration)
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
                case SDLK_RIGHT:
                    imageViewer.nextImage();
                    imageViewer.displayCurrentImage();
                    break;
                case SDLK_LEFT:
                    imageViewer.prevImage();
                    imageViewer.displayCurrentImage();
                    break;
                }
            }
        }
    }
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void PotionMixingGame::createPotion(const string &chosenPotion)
{
    string potionCode = potions[chosenPotion];
    IngredientSequenceForChosen(chosenPotion);

    renderText("Enter the potion code you think is suitable here:", 100, 250, false,0);
    SDL_RenderPresent(renderer);
    string userInput = getUserInputSDL();

    if (userInput == potionCode)
    {
        won = true;
        selectedPotionCode = potionCode;
      
    }
   

    SDL_RenderPresent(renderer);
    SDL_Delay(2000);
}
void PotionMixingGame::renderWonOrLost()
{
    SDL_RenderClear(renderer); // Clear the renderer
    gBackgroundTexture.render(renderer, 0, 0);
    SDL_Color textColor = {255, 255, 255};
    SDL_Surface *textSurface = nullptr;
    SDL_Texture *textTexture = nullptr;

    TTF_Font *font = TTF_OpenFont("JosefinSans-Bold.ttf", 50);
    if (font == nullptr)
    {
        cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << endl;
        return;
    }

    const char *message = (won) ? "Hurrah! Correct Code!" : "Oops! Incorrect Code";

    textSurface = TTF_RenderText_Solid(font, message, textColor);

    if (textSurface == nullptr)
    {
        cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << endl;
        TTF_CloseFont(font);
        return;
    }

    // Calculate the position to center the text
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    SDL_Rect textRect;
    textRect.x = (width - textWidth) / 2;
    textRect.y = (height - textHeight) / 2;
    textRect.w = textWidth;
    textRect.h = textHeight;

    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr)
    {
        cerr << "Unable to create text texture! SDL Error: " << SDL_GetError() << endl;
        SDL_FreeSurface(textSurface);
        TTF_CloseFont(font);
        return;
    }

    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    SDL_RenderPresent(renderer);
    SDL_Delay(2000);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
}
void PotionMixingGame::IngredientSequenceForChosen(const string &chosenPotion)
{
    SDL_RenderClear(renderer);
    gBackgroundTexture.render(renderer, 0, 0);

    renderText(chosenPotion, 60, 60, true,20);

    SDL_RenderPresent(renderer);
    SDL_Delay(5000);
}
string PotionMixingGame::getRandomPotion()
{
    int randomIndex = rand() % potions.size();
    auto it = begin(potions);
    advance(it, randomIndex);
    return it->first;
}

void renderStartScreen()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    if (startTexture != nullptr)
    {
        SDL_RenderCopy(renderer, startTexture, nullptr, nullptr);
    }
    SDL_RenderPresent(renderer);
}

void handleStartButtonClick(int mouseX, int mouseY)
{
    if (mouseX >= startButtonRect.x && mouseX < startButtonRect.x + startButtonRect.w &&
        mouseY >= startButtonRect.y && mouseY < startButtonRect.y + startButtonRect.h)
    {
        gameStarted = true;
    }
}
void PotionMixingGame::cleanUp()
{
    SDL_DestroyTexture(startTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
}
string PotionMixingGame::run()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    string startImagePath = "images/1.png";
    SDL_Surface *startImage = IMG_Load(startImagePath.c_str());
    startTexture = SDL_CreateTextureFromSurface(renderer, startImage);
    SDL_FreeSurface(startImage);

    startButtonRect.x = width - 460;
    startButtonRect.y = height - 120;
    startButtonRect.w = 105;
    startButtonRect.h = 105;

    while (!gameStarted)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
            {
                cleanUp();
                // return;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                handleStartButtonClick(mouseX, mouseY);
            }
        }

        renderStartScreen();
    }

    displayAvailablePotions();
    string chosenPotion = getRandomPotion();
    cout << "You need to create the potion: " << chosenPotion << endl;
    createPotion(chosenPotion);
    renderWonOrLost();
    cleanUp();
    return selectedPotionCode;
}
bool PotionMixingGame::WonOrLost()
{
    return won;
}