#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "game/World.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <random>
#include "tests/Vector2DTests.hpp"
#include "tests/AstarTests.hpp"
#include "tests/FuzzyTests.hpp"
#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#define path(x) x
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#define path(x) "gp-aai/" x
#endif

using std::thread, std::cout, std::endl, std::string;

string get_current_dir() {
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir(buff, FILENAME_MAX);
    string current_working_dir(buff);
    return current_working_dir;
}

#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800

void drawTile(SDL_Renderer* renderer, SDL_Texture* texture, int u, int v, int x, int y) {
    SDL_Rect SrcR { 8 * u, 8 * v, 8, 8 };
    SDL_Rect DestR { 16 * x, 16 * y, 16, 16 };
    SDL_RenderCopy(renderer, texture, &SrcR, &DestR);
}

void display_loop(World* world) {
    if (SDL_Init(SDL_INIT_VIDEO) == 0 && TTF_Init() == 0) {
        SDL_Window* window = NULL;
        SDL_Renderer* renderer = NULL;
        SDL_Surface* Loading_Surf;
        SDL_Texture* Background_Tx;

        //this opens a font style and sets a size
        TTF_Font* Sans = TTF_OpenFont(path("Arial.ttf"), 24);

        // this is the color in rgb format,
        // maxing out all would give you the color white,
        // and it will be your text's color
        SDL_Color White = {255, 255, 255};

        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(0,100);

        int arr[WINDOW_WIDTH/16 * (WINDOW_HEIGHT*16+1)];
        for(int i=0; i<WINDOW_WIDTH/16 * (WINDOW_HEIGHT*16+1); i++) {
            arr[i] = dist6(rng);
        }

        if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer) == 0) {
            SDL_bool done = SDL_FALSE;

            Loading_Surf = IMG_Load(path("background.bmp"));
            Background_Tx = SDL_CreateTextureFromSurface(renderer, Loading_Surf);
            SDL_FreeSurface(Loading_Surf); 

            while (!done) {
                SDL_Event event;
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                SDL_RenderClear(renderer);
                    
                for(int i=0; i<WINDOW_WIDTH/16; i++) {
                    for(int o=0; o<WINDOW_HEIGHT/16+1; o++) {

                        switch(arr[o*WINDOW_HEIGHT/16+i]) {
                            case 1:
                            case 2:
                            case 3:
                            case 4:
                            case 5:
                            case 6:
                                drawTile(renderer, Background_Tx, 6, 4, i, o);
                                break;
                            case 7:
                            case 8:
                            case 9:
                                drawTile(renderer, Background_Tx, 6, 3, i, o);
                                break;
                            case 10:
                            case 11:
                            case 12:
                                drawTile(renderer, Background_Tx, 6, 5, i, o);
                                break;
                            case 13:
                                drawTile(renderer, Background_Tx, 4, 3, i, o);
                                break;
                            default:
                                drawTile(renderer, Background_Tx, 0, 0, i, o);

                        }
                    }
                }

                // as TTF_RenderText_Solid could only be used on
                // SDL_Surface then you have to create the surface first
                SDL_Surface* surfaceMessage =
                    TTF_RenderText_Solid(Sans, "put your text here", White); 

                // now you can convert it into a texture
                SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

                SDL_Rect Message_rect; //create a rect
                Message_rect.x = 500;  //controls the rect's x coordinate 
                Message_rect.y = 500; // controls the rect's y coordinte
                Message_rect.w = 100; // controls the width of the rect
                Message_rect.h = 30; // controls the height of the rect

                // (0,0) is on the top left of the window/screen,
                // think a rect as the text's box,
                // that way it would be very simple to understand

                // Now since it's a texture, you have to put RenderCopy
                // in your game loop area, the area where the whole code executes

                // you put the renderer's name first, the Message,
                // the crop size (you can ignore this if you don't want
                // to dabble with cropping), and the rect which is the size
                // and coordinate of your texture
                SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

                // Don't forget to free your surface and texture
                SDL_FreeSurface(surfaceMessage);
                SDL_DestroyTexture(Message);


                world->render(renderer);
                

                SDL_RenderPresent(renderer);

                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        done = SDL_TRUE;
                    } else if(event.type == SDL_MOUSEBUTTONDOWN) {
                        SDL_MouseButtonEvent ev = event.button;
                        world->event(WorldEvent::mouseClick, Vector2D(ev.x, ev.y));
                    }
                }
            }
        }

        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
    SDL_Quit();
}

void logic_loop(World* world, bool* running) {
    while(*running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 30));
        world->update(1);
    }
}

int main(int argc, char* argv[])  {
    cout << get_current_dir() << endl;

    // Run tests first
    run_Vector2D_tests();
    run_Fuzzy_tests();
    run_Astar_tests();
    //return 0;


    World* world = new World(WINDOW_WIDTH, WINDOW_HEIGHT);
    bool running = true;

    // We're handing out references since this all isn't threadsafe anyways
    thread display_thread = thread(display_loop, world);
    thread logic_thread = thread(logic_loop, world, &running);

    // Wait for display thread to finish
    display_thread.join();
    running = false;
    logic_thread.join();
    return 0;
}
