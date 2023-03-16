#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "Clock.h"
#include "entities/Character.h"
#include <iostream>

SDL_Window* Window;
SDL_Renderer* Renderer;
Clock* Timer;

Character* Player;

bool Initialize() {
    int Result = SDL_Init(SDL_INIT_EVERYTHING);
    if (Result) {
        std::printf("Error while initializing %s", SDL_GetError());
        return false;
    }

    Window = SDL_CreateWindow("TrialAndError", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              900, 700, 0);
    if (!Window) {
        std::printf("Error while creating the window %s", SDL_GetError());
        return false;
    }

    Renderer = SDL_CreateRenderer(Window, -1, 0);
    if (!Renderer) {
        std::printf("Error while creating the renderer %s", SDL_GetError());
        return false;
    }

    // Initialize up to 3 controllers
    for (int i = 0; i < 3; i++) {
        SDL_GameControllerOpen(i);
    }

    Timer = new Clock(60);
    Player = new Character(Renderer, 100, 100);

    return true;
}


// Function to handle controller events
void handleControllerEvent(SDL_Event event)
{
    if (event.type == SDL_CONTROLLERBUTTONDOWN) {
        std::cout << "Button " << static_cast<int>(event.cbutton.button) << " on controller " << static_cast<int>(event.cbutton.which) << " was pressed" << std::endl;
    } else if (event.type == SDL_CONTROLLERBUTTONUP) {
        std::cout << "Button " << static_cast<int>(event.cbutton.button) << " on controller " << static_cast<int>(event.cbutton.which) << " was released" << std::endl;
    } else if (event.type == SDL_CONTROLLERAXISMOTION) {
        std::cout << "Axis " << static_cast<int>(event.caxis.axis) << " on controller " << static_cast<int>(event.caxis.which) << " moved to " << static_cast<int>(event.caxis.value) << std::endl;
    }
}


int main() {
    if (!Initialize()) {
        std::printf("Terminating..");
        exit(1);
    }

    bool Running = true;
    while (Running) {
        SDL_Event CurrentEvent;
        while (SDL_PollEvent(&CurrentEvent)) {
            switch (CurrentEvent.type) {
                case SDL_QUIT: {
                    Running = false;
                } break;
                case SDL_KEYDOWN: {
                    if (CurrentEvent.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                        Running = false;
                    else if (CurrentEvent.key.keysym.scancode == SDL_SCANCODE_F11)
                        SDL_SetWindowFullscreen(Window, !(SDL_GetWindowFlags(Window) & SDL_WINDOW_FULLSCREEN));
                } break;

                case SDL_CONTROLLERBUTTONDOWN:
                    std::cout<<"SDL_CONTROLLERBUTTONDOWN ";

                case SDL_CONTROLLERBUTTONUP:
                    std::cout<<"SDL_CONTROLLERBUTTONUP ";

                case SDL_CONTROLLERAXISMOTION:
                    std::cout<<"SDL_CONTROLLERAXISMOTION ";

                    handleControllerEvent(CurrentEvent);
                    break;
            }
        }

        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
        SDL_RenderClear(Renderer);

        Player->Draw();

        SDL_RenderPresent(Renderer);
        Timer->Tick();
    }

    delete Player;
    delete Timer;
    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);
    for (int i = 0; i < 3; i++) {
        SDL_GameControllerClose(SDL_GameControllerFromInstanceID(i));
    }
    return 0;
}
