#include <map>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "DSA.hpp"
#include "potionGame.hpp"

using namespace std;
SDL_Window *objWindow = nullptr;
SDL_Renderer *objRenderer = nullptr;
TTF_Font *gFont = nullptr;

Queue<string> imagePathQueue;
map<pair<int, int>, string> imageCoordinateMap;

SDL_Texture *backgroundTexture1 = nullptr;
SDL_Texture *backgroundTexture2 = nullptr;
SDL_Texture *endTexture = nullptr;
SDL_Rect leftSection = {0, 0, 600, 600};
SDL_Rect rightSection = {600, 0, 200, 600};
const char *imagePaths[] = {
    "knife.png", "palm.png", "letter.png", "syringe.png", "key.png"};

void renderText(const string &name, int m, int n, TTF_Font *font, SDL_Color textColor)
{
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, name.c_str(), textColor);
    if (!textSurface)
    {
        cout << "Failed to create surface: " << TTF_GetError() << endl;
        return;
    }

    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(objRenderer, textSurface);
    if (!textTexture)
    {
        cout << "Failed to create texture from surface: " << SDL_GetError() << endl;
        return;
    }

    SDL_Rect destRect = {m, n, textSurface->w, textSurface->h};
    SDL_RenderCopy(objRenderer, textTexture, nullptr, &destRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}
void wrapText(const string &text, int x, int y, bool wrapText)
{
    SDL_Color textColor = {255, 255, 255};
    TTF_Font *font = TTF_OpenFont("Boxtoon.ttf", 25);

    if (!font)
    {
        cout << "Failed to load font: " << TTF_GetError() << endl;
        return;
    }

    if (wrapText)
    {
        int wrapLength = 75;
        vector<string> wrappedLines;

    
        size_t startPos = 0;
        while (startPos < text.length())
        {
            size_t endPos = min(startPos + wrapLength, text.length());
            size_t lastSpace = text.find_last_of(" ", endPos);
            if (lastSpace != string::npos && lastSpace > startPos)
            {
                endPos = lastSpace;
            }
            wrappedLines.push_back(text.substr(startPos, endPos - startPos));
            startPos = endPos + 1;
        }

        int yOffset = y;
        for (const auto &line : wrappedLines)
        {
            renderText(line, x, yOffset, font, textColor);
            yOffset += TTF_FontHeight(font); 
        }
    }
    else
    {
        renderText(text, x, y, font, textColor);
    }

    TTF_CloseFont(font);
}
void initializeImagePathQueue()
{
    ifstream imagePathFile("textFiles/image_paths.txt");
    if (imagePathFile.is_open())
    {
        string line;
        while (getline(imagePathFile, line))
        {
            if (!line.empty())
            {
                imagePathQueue.enqueue(line);
            }
        }
        imagePathFile.close();
    }
    else
    {
        cout<< "Failed to open image_paths.txt" << endl;
    }
}

class GameObjectNode
{
public:
    GameObjectNode(SDL_Texture *texture, int width, int height)
        : texture(texture), x(0), y(0), width(width), height(height) {}

    void render() const
    {
        SDL_Rect destRect = {x, y, width, height};
        SDL_RenderCopy(objRenderer, texture, nullptr, &destRect);
    }

    bool isClicked(int mouseX, int mouseY) const
    {
        return mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height;
    }

    void hide()
    {
        hidden = true;
    }

    bool isHidden() const
    {
        return hidden;
    }

    void setCoordinates(int newX, int newY)
    {
        x = newX;
        y = newY;
    }

public:
    SDL_Texture *texture;
    int x, y, width, height;
    bool hidden = false;
    GameObjectNode *next = nullptr;
};

class GameObjectList
{
public:
    ~GameObjectList()
    {
        clear();
    }

    void addNode(GameObjectNode *newNode)
    {
        if (head == nullptr)
        {
            head = newNode;
        }
        else
        {
            newNode->next = head;
            head = newNode;
        }
    }

    void clear()
    {
        while (head != nullptr)
        {
            GameObjectNode *temp = head;
            head = head->next;
            delete temp;
        }
    }

    bool isOverlap(const SDL_Rect &rect1, const SDL_Rect &rect2)
    {
        return (rect1.x < rect2.x + rect2.w &&
                rect1.x + rect1.w > rect2.x &&
                rect1.y < rect2.y + rect2.h &&
                rect1.y + rect1.h > rect2.y);
    }

    void setRandomPositions()
    {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> xDistribution(leftSection.x, leftSection.x + leftSection.w - 100); 
        uniform_int_distribution<int> yDistribution(leftSection.y, leftSection.y + leftSection.h - 100); 

        const char **imagePathPtr = imagePaths;

        GameObjectNode *current = head;
        while (current != nullptr)
        {
            bool overlap = true;
            int locX, locY;

            while (overlap)
            {
                overlap = false;

                GameObjectNode *checkNode = head;
                while (checkNode != current && !overlap)
                {
                    if (!checkNode->isHidden() &&
                        isOverlap({current->x, current->y, current->width, current->height},
                                  {checkNode->x, checkNode->y, checkNode->width, checkNode->height}))
                    {
                        overlap = true;
                    }
                    checkNode = checkNode->next;
                }

                if (!overlap)
                {
                    locX = xDistribution(gen);
                    locY = yDistribution(gen);

                    if (*imagePathPtr)
                    {
                        imageCoordinateMap[{locX, locY}] = "images/" + string(*imagePathPtr);
                        ++imagePathPtr;
                    }
                }
            }

            current->x = locX;
            current->y = locY;

            current = current->next;
        }
    }
    bool handleMouseClick(int mouseX, int mouseY)
    {
        GameObjectNode *current = head;

        while (current != nullptr)
        {
            if (!current->isHidden() && current->isClicked(mouseX, mouseY))
            {
                auto it = imageCoordinateMap.find({current->x, current->y});

                if (it != imageCoordinateMap.end())
                {
                    if (!imagePathQueue.empty())
                    {
                        string imagePath = imagePathQueue.front();

                        if (it->second == imagePath)
                        {
                            current->hide();

                            
                            imagePathQueue.dequeue();

                           
                            static int correctCount = 0;
                            if (++correctCount >= 5)
                            {
                               
                                imagePathQueue.clear();

                                correctCount = 0; 
                            }

                            return true;
                        }
                        else
                        {
                            
                            cout << "Incorrect image path clicked." << endl;
                        }
                    }
                    else
                    {
                        
                        cout << "Image path queue is empty." << endl;
                    }
                }
                else
                {
                    
                    cout << "Image not found at coordinates (" << current->x << ", " << current->y << ")." << endl;
                }
            }
            current = current->next;
        }

        return false;
    }

public:
    GameObjectNode *head = nullptr;
};

GameObjectList objects;

bool init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cout << "SDL initialization failed: " << SDL_GetError() << endl;
        return false;
    }

    objWindow = SDL_CreateWindow("Hidden Object Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (objWindow == nullptr)
    {
        cout << "Window creation failed: " << SDL_GetError() << endl;
        return false;
    }

    objRenderer = SDL_CreateRenderer(objWindow, -1, SDL_RENDERER_ACCELERATED);
    if (objRenderer == nullptr)
    {
        cout<< "Renderer creation failed: " << SDL_GetError() << endl;
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        cout << "SDL_image initialization failed: " << IMG_GetError() << endl;
        return false;
    }

    if (TTF_Init() == -1)
    {
        cout << "SDL_ttf initialization failed: " << TTF_GetError() << endl;
        return false;
    }

    gFont = TTF_OpenFont("Boxtoon.ttf", 25);

    if (gFont == nullptr)
    {
        cout << "Failed to load font: " << TTF_GetError() << endl;
        return false;
    }

    return true;
}

bool loadMedia()
{
    SDL_Surface *backgroundSurface1 = IMG_Load("images/room.jpg");
    if (backgroundSurface1 == nullptr)
    {
        cout << "Failed to load background image 1: " << IMG_GetError() << endl;
        return false;
    }
    backgroundTexture1 = SDL_CreateTextureFromSurface(objRenderer, backgroundSurface1);
    SDL_FreeSurface(backgroundSurface1);

    if (backgroundTexture1 == nullptr)
    {
        cout<< "Failed to create background texture 1: " << SDL_GetError() << endl;
        return false;
    }

    SDL_Surface *backgroundSurface2 = IMG_Load("images/list.png");
    if (backgroundSurface2 == nullptr)
    {
        cout << "Failed to load background image 2: " << IMG_GetError() << endl;
        return false;
    }
    backgroundTexture2 = SDL_CreateTextureFromSurface(objRenderer, backgroundSurface2);
    SDL_FreeSurface(backgroundSurface2);

    if (backgroundTexture2 == nullptr)
    {
        cout << "Failed to create background texture 2: " << SDL_GetError() << endl;
        return false;
    }
    SDL_Surface *endSurface = IMG_Load("images/transition.png");
    if (endSurface == nullptr)
    {
        cout << "Failed to load end image: " << IMG_GetError() <<endl;
        return false;
    }

    endTexture = SDL_CreateTextureFromSurface(objRenderer, endSurface);
    SDL_FreeSurface(endSurface);

    if (endTexture == nullptr)
    {
        cout << "Failed to create end texture: " << SDL_GetError() << endl;
        return false;
    }
    objects.addNode(new GameObjectNode(SDL_CreateTextureFromSurface(objRenderer, IMG_Load("images/knife.png")), 30, 50));
    objects.addNode(new GameObjectNode(SDL_CreateTextureFromSurface(objRenderer, IMG_Load("images/letter.png")), 50, 50));
    objects.addNode(new GameObjectNode(SDL_CreateTextureFromSurface(objRenderer, IMG_Load("images/palm.png")), 30, 30));
    objects.addNode(new GameObjectNode(SDL_CreateTextureFromSurface(objRenderer, IMG_Load("images/syringe.png")), 30, 30));
    objects.addNode(new GameObjectNode(SDL_CreateTextureFromSurface(objRenderer, IMG_Load("images/key.png")), 30, 30));
    return true;
}

SDL_Texture *instructionTexture = nullptr;
SDL_Texture *loadEndTexture(const string &end)
{
    SDL_Surface *endSurface = IMG_Load(end.c_str());
    if (endSurface == nullptr)
    {
        cout << "Failed to load end image: " << IMG_GetError() << endl;
        return nullptr;
    }

    SDL_Texture *endTexture = SDL_CreateTextureFromSurface(objRenderer, endSurface);
    SDL_FreeSurface(endSurface);

    if (endTexture == nullptr)
    {
        cout << "Failed to create end texture: " << SDL_GetError() << endl;
        return nullptr;
    }

    return endTexture;
}

void renderEndImage()
{
    const string end = "images/transition.png";

    SDL_Texture *endTexture = loadEndTexture(end);

    if (endTexture != nullptr)
    {
        SDL_RenderCopy(objRenderer, endTexture, nullptr, nullptr);

        SDL_DestroyTexture(endTexture);
    }
}

bool isButtonClicked(int mouseX, int mouseY)
{
    return mouseX >= 400 && mouseX <= 600 && mouseY >= 400 && mouseY <= 600;
}
bool loadInstructionMedia()
{
    SDL_Surface *instructionSurface = IMG_Load("images/objMain.png");
    if (instructionSurface == nullptr)
    {
        cout << "Failed to load instruction image: " << IMG_GetError() << endl;
        return false;
    }

    instructionTexture = SDL_CreateTextureFromSurface(objRenderer, instructionSurface);
    SDL_FreeSurface(instructionSurface);

    if (instructionTexture == nullptr)
    {
        cout<< "Failed to create instruction texture: " << SDL_GetError() << endl;
        return false;
    }

    return true;
}

class RenderGame
{
public:
    RenderGame(GameObjectList &gameObjectList) : gameObjectList(gameObjectList), gameCompleted(false)
    {
        names = new string[7];
        names[0] = "FOLLOW THE ORDER";
        names[1] = "1.Key";
        names[2] = "2.Syringe";
        names[3] = "3.Palm Print";
        names[4] = "4.Sealed letter";
        names[5] = "5.Knife";
        names[6] = "NEXT GAME";
    }
    ~RenderGame()
    {
        delete[] names;
    }

    void setGameCompleted(bool completed)
    {
        gameCompleted = completed;
    }

    void renderObjects()
    {
        SDL_RenderCopy(objRenderer, backgroundTexture1, nullptr, &leftSection);
        SDL_RenderCopy(objRenderer, backgroundTexture2, nullptr, &rightSection);

        GameObjectNode *current = gameObjectList.head;
        while (current != nullptr)
        {
            if (!current->isHidden())
            {
                if (current->x >= leftSection.x && current->x + current->width <= leftSection.x + leftSection.w &&
                    current->y >= leftSection.y && current->y + current->height <= leftSection.y + leftSection.h)
                {
                    current->render();
                }
            }
            current = current->next;
        }
    }

    void renderNames()
    {
        int m;
        int n = 50;

        for (int i = 0; i < 7; ++i)
        {
            if (i == 0)
            {
                wrapText(names[i], m = 605, n, true);
                n = 100;
            }
            else
            {
                renderText(names[i], m = 610, n, gFont, {245, 245, 220});
            }

            n += 50;
        }
    }

    void renderAll()
    {
        renderObjects();
        renderNames();
    }

public:
    GameObjectList &gameObjectList;
    string *names;
    bool gameCompleted;
};

RenderGame game(objects);
bool gameWon=false;
void close()
{
    objects.clear();
    TTF_CloseFont(gFont);
    SDL_DestroyRenderer(objRenderer);
    SDL_DestroyWindow(objWindow);

    IMG_Quit();
    TTF_Quit();
    
}
void renderInstruction()
{
    SDL_RenderCopy(objRenderer, instructionTexture, nullptr, nullptr);
    SDL_RenderPresent(objRenderer);

    SDL_Event e;
    bool instructionCompleted = false;

   
    while (!instructionCompleted)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                close();
                exit(1);
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                if (isButtonClicked(mouseX, mouseY))
                {
                    instructionCompleted = true;
                }
            }
        }
    }
}
string runObjectFinding()
{
    if (!init() || !loadMedia())
    {
        cout << "Failed to initialize or load media." << endl;
        close();
    }
    if (!loadInstructionMedia())
    {
        cout<< "Failed to load instruction media." << endl;
        close();
    }

    renderInstruction();
    initializeImagePathQueue();
    objects.setRandomPositions();

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
            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit = true;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                objects.handleMouseClick(mouseX, mouseY);
            }
        }

        SDL_RenderClear(objRenderer);

        if (!game.gameCompleted && imagePathQueue.QueueSize() == 0)
        {
            game.setGameCompleted(true);
        }

        if (!game.gameCompleted)
        {
            game.renderAll();
        }
        else
        {
            renderEndImage();
        }

        SDL_RenderPresent(objRenderer);
        SDL_Delay(1000);
    }
    SDL_DestroyWindow(objWindow);
    SDL_DestroyRenderer(objRenderer);
    string pass;
    PotionMixingGame pGame;
    pass = pGame.run();
    gameWon = pGame.WonOrLost();
    close();
    return pass;
}
bool wonLabGame(){
    return gameWon;
}

