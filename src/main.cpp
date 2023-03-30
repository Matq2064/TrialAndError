#define SDL_MAIN_HANDLED
#include "GameReference.h"
#include "GameWorld.h"
#include "technical stuff/GameControllers.h"
#include "entities/Character.h"
#include "entities/Bullets.h"
#include <vector>
#include <iostream>

GameReference* GameWindow;
GameWorld* World;
GameControllers* Controllers;
Texture* TextTexture;

bool Initialize() {
    srand(time(nullptr));
    GameWindow = new GameReference();
    if (!GameWindow->Initialize())
        return false;

    World = new GameWorld(GameWindow, 10000, 1000);
    World->SetCameraPos(30, 30);
    GameWindow->RenderClass()->SetWorld(World);

    TextManager* TextHandler = GameWindow->TextHandler();
    TTF_Font* Font1 = TextHandler->LoadFont("GROBOLD.ttf", 16);
    TextTexture = TextHandler->Render(Font1, "Jesse -.. .. .", { 255, 255, 255 }, true);

    Controllers = new GameControllers();
    new Character(World, 30, 30, 10, 10);
    return true;
}

int main() {
    if (!Initialize()) {
        std::printf("Terminating..");
        exit(1);
    }

    Clock* Timer = GameWindow->Timer();
    Drawing* Draw = GameWindow->RenderClass();
    SoundManager* SoundHandler = GameWindow->SoundHandler();
    ImageManager* ImageHandler = GameWindow->ImageHandler();

    // Load the PNG images
    Texture* TextureStart = ImageHandler->LoadTexture("assets/Start.png", true);
    Texture* TextureSettings = ImageHandler->LoadTexture("assets/Settings.png", true);
    Texture* TextureConnected = ImageHandler->LoadTexture("assets/chain.png", true);
    Texture* TextureDisconnected = ImageHandler->LoadTexture("assets/dis_chain.png", true);
    Texture* TextureIcon = ImageHandler->LoadTexture("assets/PS4_Controller_Icon.png", true);
    Texture* Vignette = ImageHandler->LoadTexture("assets/vignette.png", true);
    Vignette->SetAlpha(200);

    Sound* Background = SoundHandler->LoadSound("assets/background_theme.mp3", true);
    Sound* Fail_Death = SoundHandler->LoadSound("assets/fail_death.mp3", true);
    Sound* Basic_Death = SoundHandler->LoadSound("assets/basic_death.wav", true);
    Sound* Epic_Death = SoundHandler->LoadSound("assets/epic_death.wav", true);
    Sound* LowSound = SoundHandler->LoadSound("assets/Low.wav", true);
    Sound* HighSound = SoundHandler->LoadSound("assets/High.wav", true);
    Sound* QuitSound = SoundHandler->LoadSound("assets/Quit.wav", true);

    SDL_Rect ConnectedRect = { 120, 375, 80, 44 };
    SDL_Rect DisconnectedRect = { 200, 375, 80, 44 };
    SDL_Rect IconRect = { 100, 400, 200, 109 };

    bool Running = true;
    bool m_Config = true;
    while (Running) {
        // Render the Start button
        SDL_Rect startButtonRect = { GameWindow->Width()/2-150, GameWindow->Height()/2-200, 300, 100 };
        // Render the Settings button
        SDL_Rect settingsButtonRect = { GameWindow->Width()/2-150, GameWindow->Height()/2-50, 300, 100 };
        // Input and events
        SDL_Event CurrentEvent;
        while (SDL_PollEvent(&CurrentEvent)) {
            GameWindow->Event(CurrentEvent);
            World->Event(CurrentEvent);
            Controllers->Event(CurrentEvent);

            switch (CurrentEvent.type) {
                case SDL_QUIT: {
                    Running = false;
                } break;
                case SDL_KEYDOWN: {
                    if (CurrentEvent.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                        World->SetPaused(!World->Paused());
                    // else if (CurrentEvent.key.keysym.scancode == SDL_SCANCODE_F11)
                    //     SDL_SetWindowFullscreen(Window, !(SDL_GetWindowFlags(Window) & SDL_WINDOW_FULLSCREEN));
                } break;
                case SDL_CONTROLLERDEVICEADDED: {
                    int DeviceID = CurrentEvent.cdevice.which;
                    GameController* CurrentController = Controllers->OpenController(DeviceID);
                    auto* NewPlayer = new Character(World, 30, 30, 10, 10); // Add new player
                    NewPlayer->SetGameController(CurrentController);
                    SoundHandler->PlaySound(Epic_Death);
                } break;
                case SDL_CONTROLLERDEVICEREMOVED: {
                    int InstanceID = CurrentEvent.cdevice.which;
                    GameController* DeletedController = Controllers->CloseController(InstanceID);
                    World->DestroyPlayerByController(DeletedController);
                    SoundHandler->PlaySound(Fail_Death);
                } break;
                case SDL_MOUSEBUTTONDOWN: {
                    if (CurrentEvent.button.button == SDL_BUTTON_LEFT)
                    {
                        int x = CurrentEvent.button.x;
                        int y = CurrentEvent.button.y;
                        if (x >= startButtonRect.x && x < startButtonRect.x + startButtonRect.w &&
                            y >= startButtonRect.y && y < startButtonRect.y + startButtonRect.h)
                        {
                            World->SetPaused(false);
                        }
                        else if (x >= settingsButtonRect.x && x < settingsButtonRect.x + settingsButtonRect.w &&
                            y >= settingsButtonRect.y && y < settingsButtonRect.y + settingsButtonRect.h)
                        {
                            m_Config = !m_Config;
                        }
                    }
                } break;
            }
        }

        // Ticking
        World->Tick();

        // Drawing
        Draw->SetColor(120, 0, 120, 255);
        Draw->Clear();

        World->Draw();

        Draw->RenderTextureFullscreen(Vignette->SDLTexture(), nullptr);

        SDL_Rect DestinationRect;
        TextTexture->Query(nullptr, nullptr, &DestinationRect.w, &DestinationRect.h);
        DestinationRect.x = 0;
        DestinationRect.y = GameWindow->Height() - DestinationRect.h;
        Draw->RenderTexture(TextTexture->SDLTexture(), nullptr, DestinationRect);

        if (World->Paused()) {
            Draw->SetBlendingMode(SDL_BLENDMODE_BLEND);
            Draw->SetColor(0, 0, 0, 100);
            Draw->BlendFullscreen();
            Draw->SetBlendingMode(SDL_BLENDMODE_NONE);

            // start
            Draw->SetColor(90, 20, 20, 255);
            Draw->FillRect({ GameWindow->Width()/2-150, GameWindow->Height()/2-200, 300, 100 });
            //   setting
            Draw->SetColor(20, 20, 90, 255);
            Draw->FillRect({ GameWindow->Width()/2-150, GameWindow->Height()/2-50, 300, 100 });

            Draw->RenderTexture(TextureStart->SDLTexture(), nullptr, startButtonRect);
            Draw->RenderTexture(TextureSettings->SDLTexture(), nullptr, settingsButtonRect);

            if (m_Config == false) {
                Draw->SetBlendingMode(SDL_BLENDMODE_BLEND);
                Draw->SetColor(0, 0, 0, 100);
                Draw->BlendFullscreen();
                Draw->SetBlendingMode(SDL_BLENDMODE_NONE);

                //   setting
                Draw->SetColor(20, 20, 90, 255);
                Draw->FillRect(settingsButtonRect);

                Draw->RenderTexture(TextureSettings->SDLTexture(), nullptr, settingsButtonRect);

                // for() {
                //     Draw->RenderTexture(TextureIcon->SDLTexture(), nullptr, IconRect);
                //     if () {
                //         Draw->RenderTexture(TextureConnected->SDLTexture(), nullptr, ConnectedRect);
                //     }
                //     if () {
                //         Draw->RenderTexture(TextureDisconnected->SDLTexture(), nullptr, DisconnectedRect);
                //     }
                // }
            }
        }

        Draw->UpdateWindow();
        Timer->Tick();
    }

    SoundHandler->PlaySound(QuitSound);
    GameWindow->Deinitialize(true); // close everything except sound

    delete Controllers;
    delete World;
    while(Mix_Playing(-1)) { }  // wait until last sound is done playing
    delete GameWindow;
    return 0;
}
