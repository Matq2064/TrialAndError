// src/game/interface/MainMenu.h

#pragma once

#include "../../GameReference.h"
#include <SDL.h>

class MainMenu {
private:
    GameReference* m_GameWindow;
    static LoadedMusic sElevatorMusic;
    static LoadedTexture sMenuTexture;
    static LoadedTexture sTextureTitle;
    static LoadedTexture sTexturePlay;
    static LoadedTexture sTextureExit;
    SDL_Rect m_TitleRect;
    SDL_Rect m_PlayButtonRect;
    SDL_Rect m_ExitButtonRect;

    std::vector<std::tuple<Vec2d, Vec2d, double>> m_Stars;

public:
    MainMenu(GameReference* game_window);
    ~MainMenu();

    void Show();
    void HandleEvent(const SDL_Event& event, bool& running, bool& menuOpen);
    void Tick();
    void Render();
};