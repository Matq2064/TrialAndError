#define SDL_MAIN_HANDLED
#include "GameReference.h"
#include "GameWorld.h"
#include "technical stuff/GameControllers.h"
#include "entities/Character.h"
#include "entities/Bullets.h"
#include <vector>
#include <iostream>

SDL_DisplayMode dm;
GameReference* GameWindow;
GameWorld* World;
GameControllers* Controllers;
SDL_Texture* TextTexture;

bool Initialize() {
    GameWindow = new GameReference();
    if (!GameWindow->Initialize())
        return false;

    World = new GameWorld(GameWindow, 1000, 1000);

    TextManager* TextHandler = GameWindow->TextHandler();
    SDL_Renderer* Renderer = GameWindow->Renderer();

    TTF_Font* Font1 = TextHandler->LoadFont("GROBOLD.ttf", 16);
    SDL_Surface* TempSurface = TTF_RenderText_Solid(Font1, "Text", SDL_Color{ 255, 255, 255, 255 });
    TextTexture = SDL_CreateTextureFromSurface(Renderer, TempSurface);
    SDL_FreeSurface(TempSurface);

    Controllers = new GameControllers();
    new Character(World, 100, 100);

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

    SDL_Window* Window = GameWindow->Window();
    SDL_Renderer* Renderer = GameWindow->Renderer();
    Clock* Timer = GameWindow->Timer();

    bool Running = true;
    while (Running) {
        // Input and events
        SDL_Event CurrentEvent;
        while (SDL_PollEvent(&CurrentEvent)) {
            Controllers->Event(CurrentEvent);
            World->Event(CurrentEvent);

            switch (CurrentEvent.type) {
                case SDL_QUIT: {
                    Running = false;
                } break;
                case SDL_KEYDOWN: {
                    if (CurrentEvent.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                        Running = false;
                    // else if (CurrentEvent.key.keysym.scancode == SDL_SCANCODE_F11)
                    //     SDL_SetWindowFullscreen(Window, !(SDL_GetWindowFlags(Window) & SDL_WINDOW_FULLSCREEN));
                } break;
                case SDL_CONTROLLERDEVICEADDED: {
                    int DeviceID = CurrentEvent.cdevice.which;
                    GameController* CurrentController = Controllers->OpenController(DeviceID);
                    auto* NewPlayer = new Character(World, 0, 0); // Add new player
                    NewPlayer->SetGameController(CurrentController);
                } break;
                case SDL_CONTROLLERDEVICEREMOVED: {
                    int InstanceID = CurrentEvent.cdevice.which;
                    GameController* DeletedController = Controllers->CloseController(InstanceID);
                    Character* CorrespondingPlayer = nullptr;
                    for (Entity* CurrentEntity : World->Entities()) {
                        if (CurrentEntity->EntityType() != GameWorld::ENTTYPE_CHARACTER)
                            continue;

                        auto CurrentPlayer = (Character*)CurrentEntity;
                        if (CurrentPlayer->GetGameController() == DeletedController) {
                            CorrespondingPlayer = CurrentPlayer;
                            break;
                        }
                    }

                    delete CorrespondingPlayer;
                } break;
            }
        }

        // Ticking
        World->Tick();

        // Drawing
        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
        SDL_RenderClear(Renderer);

        World->Draw();

        SDL_Rect DestinationRect;
        DestinationRect.x = 400;
        DestinationRect.y = 400;
        SDL_QueryTexture(TextTexture, nullptr, nullptr, &DestinationRect.w, &DestinationRect.h);
        SDL_RenderCopy(Renderer, TextTexture, nullptr, &DestinationRect);

        SDL_RenderPresent(Renderer);
        Timer->Tick();
    }
    delete Controllers;
    SDL_DestroyTexture(TextTexture);
    delete World;
    delete GameWindow;
    return 0;
}
