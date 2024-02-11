#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <chrono>
#include <ctime>
#include <vector>
#include <thread>
#include <cmath>
#include "DSA.hpp"

using namespace std;
struct KeyButton
{
    SDL_Rect rect;
    string text;
};
class Decoding
{
public:
    Decoding();
    ~Decoding();
    SDL_Window *dWindow;
    SDL_Renderer *dRenderer;
    TTF_Font *gFont;
    SDL_Point mousePos;
    int mouseX, mouseY;
    const int SCWidth = 900;
    const int SCHeight = 700;
    const int TEXT_TYPING_SPEED = 100;
    bool displayPopup = false,textDecoded;
    SDL_Rect startButtonRect, instructButtonRect, backButtonRect, enterButtonRect;
    int DEFAULT_FONT_SIZE = 28;
    vector<KeyButton> keyboardButtons;
    string wordArray[4];
    enum GameState
    {
        MAIN_MENU,
        INSTRUCTIONS_SCREEN,
        CIPHER_TEXT,
        STACK_SCREEN,
        YOU_LOST
    };

    bool init();
    bool loadMedia();
    void close();
    void initializeButtons(vector<KeyButton> &keyboardButtons);
    void renderText(const string &text, int x, int y, SDL_Color textColor);
    void renderVirtualKeyboard(const vector<KeyButton> &keyboardButtons);
    bool renderPopup(const string &message, int popupWidth, int popupHeight);
    bool successfulDecoding();
    string toLowerCase(const string &str);
    string encodeCaesarCipher(const string &plainText, int shift);
    string reverseWithStack(const string &message);
    string run();
};

Decoding::Decoding()
    : dWindow(nullptr), dRenderer(nullptr), gFont(nullptr), SCWidth(900), SCHeight(700), TEXT_TYPING_SPEED(100), DEFAULT_FONT_SIZE(28),
      displayPopup(false), textDecoded(false),startButtonRect({217, 55, 240, 260}), instructButtonRect({522, 120, 80, 110}), backButtonRect({10, SCHeight - 60, 100, 50}), enterButtonRect({SCWidth - 140, SCHeight - 110, 120, 50})

{
    if (!init() || !loadMedia())
    {
        cout << "Failed to initialize, load media, or load images\n";
    }
    initializeButtons(keyboardButtons);
    wordArray[0] = "queue";
    wordArray[1] = "algorithm";
    wordArray[2] = "stack";
    wordArray[3] = "binary tree";
}
void Decoding::initializeButtons(vector<KeyButton> &keyboardButtons)
{
    int buttonWidth = 40;
    int buttonHeight = 40;
    int buttonPaddingX = 8;
    int buttonPaddingY = 10;
    int rows = 5;
    int columns = 10;
    int keyboardWidth = columns * (buttonWidth + buttonPaddingX) - buttonPaddingX;
    int keyboardHeight = rows * (buttonHeight + buttonPaddingY) - buttonPaddingY;
    int startX = (SCWidth - keyboardWidth) / 2;
    int startY = (SCHeight - keyboardHeight) / 2;

    vector<string> rowTexts = {
        "1234567890",
        "QWERTYUIOP",
        " ASDFGHJK ",
        " LZXCVBNM ",
        ""};

    for (int i = 0; i < rows; ++i)
    {
        const string &rowText = rowTexts[i];
        for (int j = 0; j < rowText.length(); ++j)
        {
            KeyButton button;
            button.rect = {startX + j * (buttonWidth + buttonPaddingX), startY + i * (buttonHeight + buttonPaddingY), buttonWidth, buttonHeight};
            button.text = rowText[j];
            keyboardButtons.push_back(button);
        }
    }

    KeyButton spaceButton;
    spaceButton.rect = {startX + 1 * (buttonWidth + buttonPaddingX), startY + 4 * (buttonHeight + buttonPaddingY), 4 * (buttonWidth + buttonPaddingX), buttonHeight};
    spaceButton.text = " ";
    keyboardButtons.push_back(spaceButton);

    KeyButton backspaceButton;
    backspaceButton.rect = {startX + 6 * (buttonWidth + buttonPaddingX), startY + 4 * (buttonHeight + buttonPaddingY), 3 * (buttonWidth + buttonPaddingX), buttonHeight};
    backspaceButton.text = "<=";
    keyboardButtons.push_back(backspaceButton);
}

Decoding::~Decoding()
{
    close();
}


bool Decoding::loadMedia()
{
    return true;
}

bool Decoding::init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return false;
    }

    dWindow = SDL_CreateWindow("Decoding Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCWidth, SCHeight, SDL_WINDOW_SHOWN);
    if (dWindow == nullptr)
    {
        cout << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
        return false;
    }

    dRenderer = SDL_CreateRenderer(dWindow, -1, SDL_RENDERER_ACCELERATED);
    if (dRenderer == nullptr)
    {
        cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << "\n";
        return false;
    }

    if (TTF_Init() == -1)
    {
        cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << "\n";
        return false;
    }

    gFont = TTF_OpenFont("Kanit-Medium.ttf", 28);
    if (gFont == nullptr)
    {
        cout << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << "\n";
        return false;
    }

    return true;
}

void Decoding::close()
{
    SDL_DestroyRenderer(dRenderer);
    SDL_DestroyWindow(dWindow);
    TTF_CloseFont(gFont);
    TTF_Quit();
}

void Decoding::renderText(const string &text, int x, int y, SDL_Color textColor)
{
    SDL_Surface *textSurface = TTF_RenderText_Solid(gFont, text.c_str(), textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(dRenderer, textSurface);
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    SDL_Rect renderQuad = {x, y, textWidth, textHeight};
    SDL_RenderCopy(dRenderer, textTexture, NULL, &renderQuad);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void Decoding::renderVirtualKeyboard(const vector<KeyButton> &keyboardButtons)
{
    int borderWidth = 2; 

    for (const auto &button : keyboardButtons)
    {
        SDL_SetRenderDrawColor(dRenderer, 80, 0, 0, 255);
        SDL_RenderFillRect(dRenderer, &button.rect);

        SDL_SetRenderDrawColor(dRenderer, 255, 255, 255, 255); 

        SDL_Rect borderRect = {
            button.rect.x - borderWidth,
            button.rect.y - borderWidth,
            button.rect.w + 2 * borderWidth,
            button.rect.h + 2 * borderWidth};
        SDL_RenderDrawRect(dRenderer, &borderRect);

        renderText(button.text, button.rect.x + button.rect.w / 2 - 10, button.rect.y + button.rect.h / 2 - 20, {255, 255, 255, 255});
    }
}

bool Decoding::renderPopup(const string &message, int popupWidth, int popupHeight)
{
    SDL_Texture *popupTexture = IMG_LoadTexture(dRenderer, "images/popup_bg.png");

    if (popupTexture == nullptr)
    {
        cout << "Failed to load pop-up background image! SDL_Error: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_Rect popupRect = {SCWidth / 2 - popupWidth / 2, SCHeight / 2 - popupHeight / 2, popupWidth, popupHeight};

    SDL_RenderCopy(dRenderer, popupTexture, NULL, &popupRect);

    renderText(message, popupRect.x + 10, popupRect.y + 10, {255, 255, 255, 255});

    SDL_Texture *closeButtonTexture = IMG_LoadTexture(dRenderer, "images/x.png");

    if (closeButtonTexture == nullptr)
    {
        cout << "Failed to load close button image! SDL_Error: " << SDL_GetError() << "\n";
        SDL_DestroyTexture(popupTexture);
        return false;
    }

    int closeButtonSize = 30;
    SDL_Rect closeButtonRect = {popupRect.x + popupRect.w - closeButtonSize - 20, popupRect.y + 10, closeButtonSize, closeButtonSize};
    SDL_RenderCopy(dRenderer, closeButtonTexture, NULL, &closeButtonRect);

    SDL_DestroyTexture(popupTexture);
    SDL_DestroyTexture(closeButtonTexture);
    if (mouseX >= closeButtonRect.x && mouseX <= closeButtonRect.x + closeButtonRect.w &&
        mouseY >= closeButtonRect.y && mouseY <= closeButtonRect.y + closeButtonRect.h)
    {
        return true;
    }

    return false;
}

string Decoding::toLowerCase(const string &str)
{
    string lowerStr = str;
    for (char &c : lowerStr)
    {
        c = tolower(c);
    }
    return lowerStr;
}

string Decoding::encodeCaesarCipher(const string &plainText, int shift)
{
    string encodedText = "";

    for (char c : plainText)
    {
        if (isalpha(c))
        {
            char base = (isupper(c)) ? 'A' : 'a';
            encodedText += (c - base + shift) % 26 + base;
        }
        else if (c == ' ')
        {
            encodedText += ' ';
        }
        else
        {
            encodedText += c;
        }
    }

    return encodedText;
}

string Decoding::reverseWithStack(const string &message)
{
    Stack<char> charStack;
    for (char c : message)
    {
        charStack.push(c);
    }

    string reversedMessage = "";
    while (!charStack.IsEmpty())
    {
        reversedMessage += charStack.Top();
        charStack.Pop();
    }

    return reversedMessage;
}
string Decoding::run()
{
    SDL_Event e;
    bool quit = false;
    GameState currentState = MAIN_MENU;
    bool textFullyTyped = false;
    string typedText = "";
    string selectedWords = "";
    string enteredText = "";
    string reversedText = "";
    SDL_Texture *popupTexture = IMG_LoadTexture(dRenderer, "images/popup_bg.png");
    if (popupTexture == nullptr)
    {
        cout << "Failed to load pop-up background image! SDL_Error: " << SDL_GetError() << "\n";
    }
    SDL_Texture *closeButtonTexture = IMG_LoadTexture(dRenderer, "images/x.png");
    if (closeButtonTexture == nullptr)
    {
        cout << "Failed to load close button image! SDL_Error: " << SDL_GetError() << "\n";
        SDL_DestroyTexture(popupTexture);
    }
    SDL_Texture *mainMenuBackgroundTexture = IMG_LoadTexture(dRenderer, "images/cipherBg2.png");
    if (mainMenuBackgroundTexture == nullptr)
    {
        cout << "Failed to load main menu background image! SDL_Error: " << SDL_GetError() << "\n";
        SDL_DestroyTexture(popupTexture);
        SDL_DestroyTexture(closeButtonTexture);
    }
    SDL_Texture *InstructionScreenTexture = IMG_LoadTexture(dRenderer, "images/cipherbg3.png");
    if (InstructionScreenTexture == nullptr)
    {
        cout << "Failed to load main menu background image! SDL_Error: " << SDL_GetError() << "\n";
        SDL_DestroyTexture(popupTexture);
        SDL_DestroyTexture(closeButtonTexture);
        SDL_DestroyTexture(mainMenuBackgroundTexture);
    }
    SDL_Texture *cipherScreenBackgroundTexture = IMG_LoadTexture(dRenderer, "images/cipherbg1.png");
    if (cipherScreenBackgroundTexture == nullptr)
    {
        cout << "Failed to load cipher screen background image! SDL_Error: " << SDL_GetError() << "\n";
        SDL_DestroyTexture(popupTexture);
        SDL_DestroyTexture(closeButtonTexture);
        SDL_DestroyTexture(InstructionScreenTexture);
        SDL_DestroyTexture(mainMenuBackgroundTexture);
    }
    SDL_Texture *newScreenTexture = IMG_LoadTexture(dRenderer, "images/cipherbg5.png");
    if (newScreenTexture == nullptr)
    {
        cout << "Failed to load new screen image! SDL_Error: " << SDL_GetError() << "\n";
        SDL_DestroyTexture(popupTexture);
        SDL_DestroyTexture(closeButtonTexture);
        SDL_DestroyTexture(InstructionScreenTexture);
        SDL_DestroyTexture(mainMenuBackgroundTexture);
        SDL_DestroyTexture(cipherScreenBackgroundTexture);
    }
    SDL_Texture *lostScreenTexture = IMG_LoadTexture(dRenderer, "images/cipherbg4.png");
    if (newScreenTexture == nullptr)
    {
        cout << "Failed to load new screen image! SDL_Error: " << SDL_GetError() << "\n";
        SDL_DestroyTexture(lostScreenTexture);
        SDL_DestroyTexture(popupTexture);
        SDL_DestroyTexture(closeButtonTexture);
        SDL_DestroyTexture(InstructionScreenTexture);
        SDL_DestroyTexture(mainMenuBackgroundTexture);
        SDL_DestroyTexture(cipherScreenBackgroundTexture);
    }
    int count = 0;
    int shift = 3;
    srand(time(nullptr));
    int randomIndex = rand() % 4; 
    string originalText = wordArray[randomIndex];
    string cipheredText = encodeCaesarCipher(originalText, shift);
    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit = true; 
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    SDL_GetMouseState(&mouseX, &mouseY);

                    if (currentState == MAIN_MENU)
                    {
                        if (mouseX >= startButtonRect.x && mouseX <= startButtonRect.x + startButtonRect.w &&
                            mouseY >= startButtonRect.y && mouseY <= startButtonRect.y + startButtonRect.h)
                        {
                            currentState = CIPHER_TEXT;
                        }
                        if (mouseX >= instructButtonRect.x && mouseX <= instructButtonRect.x + instructButtonRect.w &&
                            mouseY >= instructButtonRect.y && mouseY <= instructButtonRect.y + instructButtonRect.h)
                        {
                            currentState = INSTRUCTIONS_SCREEN;
                        }
                    }
                    else if (currentState == INSTRUCTIONS_SCREEN)
                    {
                        
                        if (mouseX >= backButtonRect.x && mouseX <= backButtonRect.x + backButtonRect.w &&
                            mouseY >= backButtonRect.y && mouseY <= backButtonRect.y + backButtonRect.h)
                        {
                            currentState = MAIN_MENU; 
                        }
                    }
                    else if (currentState == CIPHER_TEXT)
                    {
                        if (mouseX >= backButtonRect.x && mouseX <= backButtonRect.x + backButtonRect.w &&
                            mouseY >= backButtonRect.y && mouseY <= backButtonRect.y + backButtonRect.h)
                        {
                            currentState = MAIN_MENU; 
                            randomIndex = rand() % 4; 
                            originalText = wordArray[randomIndex];
                        }
                        else if (mouseX >= enterButtonRect.x && mouseX <= enterButtonRect.x + enterButtonRect.w &&
                                 mouseY >= enterButtonRect.y && mouseY <= enterButtonRect.y + enterButtonRect.h)
                        {
                            cout << "Enter button clicked!" << endl;
                            enteredText = typedText;
                            cout << enteredText << endl;
                            if (toLowerCase(originalText) == toLowerCase(enteredText))
                            {
                                currentState = STACK_SCREEN;
                                textDecoded = true;
                                cout << "Here's your first passowrd" << endl;
                            }
                            else
                            {
                                cout << "Wrong! Try again.\n";
                                count++;
                                if (count < 3)
                                {
                                    displayPopup = true;
                                    typedText = "";
                                    selectedWords = "";
                                }
                                else
                                {
                                    currentState = YOU_LOST;
                                    textDecoded = false;
                                }
                            }
                        }

                        for (const auto &button : keyboardButtons)
                        {
                            if (mouseX >= button.rect.x && mouseX <= button.rect.x + button.rect.w &&
                                mouseY >= button.rect.y && mouseY <= button.rect.y + button.rect.h)
                            {

                                if (button.text == "<=" && !selectedWords.empty())
                                {
                                    typedText = typedText.substr(0, typedText.length() - 1);
                                    selectedWords = selectedWords.substr(0, selectedWords.length() - 1);
                                }
                                else if (button.text == " " && button.text.length() == 1)
                                {
                                    typedText += ' ';
                                    selectedWords += ' ';
                                }
                                else if (button.text.length() == 1)
                                {
                                    typedText += button.text;
                                    selectedWords += button.text;
                                }
                            }
                        }
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(dRenderer, 255, 255, 255, 255);
        SDL_RenderClear(dRenderer);
        if (currentState == MAIN_MENU)
        {
            SDL_RenderCopy(dRenderer, mainMenuBackgroundTexture, NULL, NULL);
        }
        if (currentState == INSTRUCTIONS_SCREEN)
        {
            SDL_RenderCopy(dRenderer, InstructionScreenTexture, NULL, NULL);
            SDL_SetRenderDrawColor(dRenderer, 80, 0, 0, 255);
            SDL_RenderFillRect(dRenderer, &backButtonRect);

            SDL_SetRenderDrawColor(dRenderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(dRenderer, &backButtonRect);

            renderText("Back", 29, SCHeight - 53, {255, 255, 255, 255});
        }
        else if (currentState == CIPHER_TEXT)
        {
            SDL_RenderCopy(dRenderer, cipherScreenBackgroundTexture, NULL, NULL);

            SDL_SetRenderDrawColor(dRenderer, 80, 0, 0, 255);
            SDL_RenderFillRect(dRenderer, &backButtonRect);

            SDL_SetRenderDrawColor(dRenderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(dRenderer, &backButtonRect);

            renderText("Back", 29, SCHeight - 53, {255, 255, 255, 255});

            SDL_SetRenderDrawColor(dRenderer, 80, 0, 0, 255);
            SDL_RenderFillRect(dRenderer, &enterButtonRect);

            SDL_SetRenderDrawColor(dRenderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(dRenderer, &enterButtonRect);

            renderText("Enter", SCWidth - 115, SCHeight - 110, {255, 255, 255, 255});

            if (textFullyTyped)
            {
                renderText("Ciphered Text: " + cipheredText, 50, 50, {200, 200, 200, 255});
            }
            else
            {
                string partialText;
                for (int i = 0; i < cipheredText.length(); ++i)
                {
                    partialText += cipheredText[i];
                    renderText("Ciphered Text: " + partialText, 50, 50, {200, 200, 200, 255});
                    SDL_RenderPresent(dRenderer);
                    this_thread::sleep_for(chrono::milliseconds(TEXT_TYPING_SPEED));
                }

                textFullyTyped = true;
            }

            renderText("Selected Words: " + selectedWords, 50, SCHeight - 150, {200, 200, 200, 255});
            renderVirtualKeyboard(keyboardButtons);
            if (displayPopup)
            {
                if (renderPopup(" ", 400, 200))
                {
                    displayPopup = false;
                }
            }
        }

        else if (currentState == STACK_SCREEN)
        {

            int screenWidth, screenHeight;
            SDL_GetWindowSize(dWindow, &screenWidth, &screenHeight);
            SDL_Rect imageRect = {0, 0, screenWidth, screenHeight};
            SDL_RenderCopy(dRenderer, newScreenTexture, NULL, &imageRect);

            bool typewriterEffectDone = false;
            string Text = "Password to the next door is: ";
            int textLength = Text.length();
            static int charactersTyped = 0;

            if (!typewriterEffectDone && charactersTyped < textLength)
            {
                string partialText = Text.substr(0, charactersTyped + 1);
                renderText(partialText, screenWidth / 2 - 180, screenHeight / 2 - 25, {255, 255, 255, 255});
                SDL_RenderPresent(dRenderer);
                this_thread::sleep_for(chrono::milliseconds(TEXT_TYPING_SPEED));

                charactersTyped++;

                if (charactersTyped == textLength)
                {
                    typewriterEffectDone = true;
                }
            }

            if (charactersTyped >= textLength)
            {
                renderText(Text, screenWidth / 2 - 180, screenHeight / 2 - 25, {255, 255, 255, 255});
            }

            reversedText = reverseWithStack(enteredText);
            renderText("PASSWORD: " + reversedText, screenWidth / 2 - 180, screenHeight / 2 + 25, {255, 255, 255, 255});
        }
        else if (currentState == YOU_LOST)
        {
            int screenWidth, screenHeight;
            SDL_GetWindowSize(dWindow, &screenWidth, &screenHeight);
            SDL_Rect imageRect = {0, 0, screenWidth, screenHeight};
            SDL_RenderCopy(dRenderer, lostScreenTexture, NULL, &imageRect);
        }

        SDL_RenderPresent(dRenderer);
        
    }
    
    if (!textDecoded)
    {
        close();
    }
    
    SDL_DestroyTexture(lostScreenTexture);
    SDL_DestroyTexture(newScreenTexture);
    SDL_DestroyTexture(mainMenuBackgroundTexture);
    SDL_DestroyTexture(cipherScreenBackgroundTexture);
    return reversedText;
}
bool Decoding::successfulDecoding(){
    return textDecoded;
}
