#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace std;


struct ImageNode {
    string imageName;
    SDL_Texture* texture;
    ImageNode* next;
    ImageNode* prev;

    ImageNode(const string& name, SDL_Texture* tex) : imageName(name), texture(tex), next(nullptr), prev(nullptr) {}
};


const int IMAGE_WIDTH = 200;
const int IMAGE_HEIGHT = 150;


class ImageViewer {
private:
    ImageNode* current;
    SDL_Renderer* renderer;

public:
    ImageViewer(SDL_Renderer* rend) : current(nullptr), renderer(rend) {}


    void addImage(const string& imageName) {
        SDL_Surface* surface = IMG_Load(imageName.c_str());
        if (!surface) {
            cerr << "Failed to load image: " << IMG_GetError() << endl;
            return;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        ImageNode* newNode = new ImageNode(imageName, texture);

        if (!current) {
          
            newNode->next = newNode;
            newNode->prev = newNode;
            current = newNode;
        } else {
           
            newNode->next = current->next;
            current->next->prev = newNode;
            newNode->prev = current;
            current->next = newNode;
        }
    }


    void displayCurrentImage() const {
        if (current) {
          
            SDL_RenderClear(renderer);

           
            SDL_RenderCopy(renderer, current->texture, nullptr, nullptr);

       
            SDL_RenderPresent(renderer);
        } else {
            cout << "No images in the viewer." << endl;
        }
    }
    

   
    void nextImage() {
        if (current) {
           
            current = current->next;
        } else {
            cout << "No images in the viewer." << endl;
        }
    }

    void prevImage() {
        if (current) {
          
            current = current->prev;
        } else {
            cout << "No images in the viewer." << endl;
        }
    }
};

