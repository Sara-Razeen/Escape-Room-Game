#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "DSA.hpp"
using namespace std;

const int screenWIDTH = 900;
const int screenHEIGHT = 700;

struct ridds
{
    string q[5];
    bool a[5] = {0, 0, 1, 0, 1};
};
class RiddlesGame
{
public:
    RiddlesGame();
    void run();
    bool isWin();
    ~RiddlesGame();

private:
    SDL_Renderer *renderer;
    SDL_Window *window;
    TTF_Font *font, *font2;
    ridds RD;
    Node *root, *leftchild, *rightchild;
    int currentRiddleIndex;
    bool flag, ans, won=false;
    void Renderer();
    void renderText(SDL_Renderer *renderer, TTF_Font *font, const string &text, int x, int y, SDL_Color textColor);
    void renderStartScreen();
    void renderRiddle(const string &riddle);
    void renderButtons(SDL_Rect &yesButtonRect, SDL_Rect &noButtonRect);
    void renderIntermediateScreen();
    void renderWinScreen();
    void renderLoseScreen();
    void clearRenderer();
    void clean();
    void readRiddlesFromFile(const string &filename);
    void handleStartButtonClick();
    void handleUserInput(SDL_Rect &yesButtonRect, SDL_Rect &noButtonRect);
};

void RiddlesGame::Renderer()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
    }

    if (TTF_Init() == -1)
    {
        cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
    }

    window = SDL_CreateWindow("Quest Quandary", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWIDTH, screenHEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
    }

    font = TTF_OpenFont("Kanit-Medium.ttf", 24);
    font2 = TTF_OpenFont("JosefinSans-Bold.ttf", 50);
    if (font == nullptr || font2 == nullptr)
    {
        cout << "Unable to load font! SDL_ttf Error: " << TTF_GetError() << endl;
    }
}

RiddlesGame::~RiddlesGame()
{
    clean();
}
void RiddlesGame::clean()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    TTF_CloseFont(font);
    TTF_Quit();
}
void RiddlesGame::renderText(SDL_Renderer *renderer, TTF_Font *font, const string &text, int x, int y, SDL_Color textColor)
{
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    if (textSurface == nullptr)
    {
        cout << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << endl;
        return;
    }

    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr)
    {
        cout << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << endl;
        SDL_FreeSurface(textSurface);
        return;
    }
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    SDL_Rect renderRect = {x - textWidth / 2, y - textHeight / 2, textWidth, textHeight};

    SDL_RenderCopy(renderer, textTexture, nullptr, &renderRect);

    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void RiddlesGame::renderStartScreen()
{
    SDL_RenderClear(renderer);

    SDL_Surface *backgroundSurface = IMG_Load("images/questquandarymain.jpg");
    if (backgroundSurface == nullptr)
    {
        cout << "Unable to load background image! SDL_image Error: " << IMG_GetError() << endl;
    }
    else
    {
        SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
        SDL_FreeSurface(backgroundSurface);

        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
        TTF_Font *font1 = TTF_OpenFont("JosefinSans-Bold.ttf", 25);
        renderText(renderer, font1, "Start", screenWIDTH / 2 + 20, screenHEIGHT / 2 + 105, {0, 0, 0, 255});

        SDL_DestroyTexture(backgroundTexture);
    }
    SDL_RenderPresent(renderer);
}
void RiddlesGame::renderRiddle(const string &riddle)
{
    SDL_RenderClear(renderer);

    SDL_Surface *backgroundSurface = IMG_Load("images/riddlescreen.jpg");
    if (backgroundSurface == nullptr)
    {
        cout << "Unable to load background image! SDL_image Error: " << IMG_GetError() << endl;
    }
    else
    {
        SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
        SDL_FreeSurface(backgroundSurface);

        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);

        stringstream ss(riddle);
        string line;
        int y = 200;
        int maxWidth = screenWIDTH - 40;

        while (getline(ss, line, '\n'))
        {
            size_t startPos = 0;
            while (startPos < line.length())
            {
                size_t spacePos = line.find("  ", startPos);
                if (spacePos == string::npos)
                    spacePos = line.length();

                string subLine = line.substr(startPos, spacePos - startPos);

                renderText(renderer, font, subLine, screenWIDTH / 2, y, {255, 255, 255, 255});

                y += 30;
                startPos = spacePos + 1;
            }
        }

        SDL_DestroyTexture(backgroundTexture);
    }
}
void RiddlesGame::renderButtons(SDL_Rect &yesButtonRect, SDL_Rect &noButtonRect)
{
    TTF_Font *font = TTF_OpenFont("PirataOne-Regular.ttf", 70);
    renderText(renderer, font, "Yes", yesButtonRect.x + yesButtonRect.w / 2, yesButtonRect.y + yesButtonRect.h / 2, {255, 255, 255, 255});

    renderText(renderer, font, "No", noButtonRect.x + noButtonRect.w / 2, noButtonRect.y + noButtonRect.h / 2, {255, 255, 255, 255});

    SDL_RenderPresent(renderer);
}

void RiddlesGame::renderIntermediateScreen()
{
    SDL_RenderClear(renderer);

    SDL_Surface *backgroundSurface = IMG_Load("images/Qinstructions.jpg");
    if (backgroundSurface == nullptr)
    {
        cout << "Unable to load background image! SDL_image Error: " << IMG_GetError() << endl;
    }
    else
    {
        SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
        SDL_FreeSurface(backgroundSurface);

        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);

        SDL_DestroyTexture(backgroundTexture);
    }

    SDL_RenderPresent(renderer);

    SDL_Delay(2000);
}

void RiddlesGame::renderWinScreen()
{
    clearRenderer();

    SDL_Surface *backgroundSurface = IMG_Load("images/Qpassword.jpg");
    if (backgroundSurface == nullptr)
    {
        cout << "Unable to load background image! SDL_image Error: " << IMG_GetError() << endl;
    }
    else
    {
        SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
        SDL_FreeSurface(backgroundSurface);

        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);

        renderText(renderer, font2, "Egress", screenWIDTH / 2, screenHEIGHT / 2 + 10, {128, 128, 128, 255});

        SDL_DestroyTexture(backgroundTexture);
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(5000);
}

void RiddlesGame::renderLoseScreen()
{
    clearRenderer();

    SDL_Surface *backgroundSurface = IMG_Load("images/wrongpath.png");
    if (backgroundSurface == nullptr)
    {
        cout << "Unable to load background image! SDL_image Error: " << IMG_GetError() << endl;
    }
    else
    {
        SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
        SDL_FreeSurface(backgroundSurface);

        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);

        SDL_DestroyTexture(backgroundTexture);
    }

    SDL_RenderPresent(renderer);

    SDL_Delay(5000);
}

void RiddlesGame::clearRenderer()
{
    SDL_RenderClear(renderer);
}

RiddlesGame::RiddlesGame() : currentRiddleIndex(0)
{
    Renderer();
    readRiddlesFromFile("textFiles/riddles.txt");

    root = newNode(5);
    root->left = newNode(3);
    root->left->right = newNode(4);
    root->left->left = newNode(2);
    root->left->left->left = newNode(1);
    root->left->left->right = newNode(0);
    root->right = newNode(9);
    root->right->left = newNode(7);
    root->right->right = newNode(10);
    root->right->right->right = newNode(12);
    root->right->right->left = newNode(11);
    leftchild = root->left;
    rightchild = root->right;
}

void RiddlesGame::run()
{
    renderStartScreen();

    SDL_Event startEvent;
    bool startButtonClicked = false;

    while (!startButtonClicked)
    {
        if (SDL_PollEvent(&startEvent) != 0)
        {
            if (startEvent.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                if (mouseX >= 0 && mouseX <= screenWIDTH &&
                    mouseY >= 0 && mouseY <= screenHEIGHT)
                {
                    renderIntermediateScreen();
                    startButtonClicked = true;
                }
            }
        }
    }

    SDL_Rect buttonsRect = {screenWIDTH / 2 - 100, screenHEIGHT / 2 + 150, 250, 50};
    SDL_Rect yesButtonRect = {buttonsRect.x, buttonsRect.y, 100, 50};
    SDL_Rect noButtonRect = {buttonsRect.x + 150, buttonsRect.y, 100, 50};

    while (currentRiddleIndex < 5)
    {
        renderRiddle(RD.q[currentRiddleIndex]);
        renderButtons(yesButtonRect, noButtonRect);
        handleUserInput(yesButtonRect, noButtonRect);
        SDL_Delay(700);

        if (ans == RD.a[currentRiddleIndex])
        {
            if (root->right != NULL)
            {
                root = rightchild;
                rightchild = root->right;
                cout << "AT node: " << root->data << endl;
                renderText(renderer, font2, "Correct", screenWIDTH / 2, screenHEIGHT / 2 + 300, {255, 255, 255, 255});
                currentRiddleIndex++;
            }
            else
            {
                break;
            }
        }
        else
        {
            if (root->left != NULL)
            {
                root = leftchild;
                leftchild = root->left;
                cout << "AT node: " << root->data << endl;
                renderText(renderer, font2, "Incorrect", screenWIDTH / 2, screenHEIGHT / 2 + 300, {255, 255, 255, 255});
                currentRiddleIndex++;
            }
            else
            {
                break;
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(500);
    }

    if (root->data == 12)
    {
        cout << "YOU WON" << endl;
        cout << root->data << endl;
        renderWinScreen();
        won = true;
    }
    else
    {
        cout << "You Lost" << endl;
        cout << root->data << endl;
        renderLoseScreen();
       
        clean();
    }
}
bool RiddlesGame::isWin(){
    return won;
}
void RiddlesGame::readRiddlesFromFile(const string &filename)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cout << "Unable to open file: " << filename << endl;
        return;
    }

    for (int i = 0; i < 5; ++i)
    {
        if (!getline(file, RD.q[i]))
        {
            cout << "Error reading riddle from file: " << filename << endl;
            break;
        }
    }

    file.close();
}

void RiddlesGame::handleUserInput(SDL_Rect &yesButtonRect, SDL_Rect &noButtonRect)
{
    SDL_Event e;
    bool userInputEntered = false;

    while (!userInputEntered)
    {
        if (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                cout << "Mouse clicked at: (" << mouseX << ", " << mouseY << ")" << endl;

                if (mouseX >= yesButtonRect.x && mouseX <= yesButtonRect.x + yesButtonRect.w &&
                    mouseY >= yesButtonRect.y && mouseY <= yesButtonRect.y + yesButtonRect.h)
                {
                    cout << "Clicked Yes button" << endl;
                    flag = true;
                    userInputEntered = true;
                }
                else if (mouseX >= noButtonRect.x && mouseX <= noButtonRect.x + noButtonRect.w &&
                         mouseY >= noButtonRect.y && mouseY <= noButtonRect.y + noButtonRect.h)
                {
                    cout << "Clicked No button" << endl;
                    flag = false;
                    userInputEntered = true;
                }
            }
            
        }
        ans = flag;
    }
}