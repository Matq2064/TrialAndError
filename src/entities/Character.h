#ifndef TRIALANDERROR_CHARACTER_H
#define TRIALANDERROR_CHARACTER_H

#include <SDL.h>
#include <string>
#include <iostream>
#include "Entity.h"
#include "../technical stuff/GameControllers.h"
#include "../technical stuff/Colors.h"

class Character : public Entity {
public:
    enum WeaponType{
        WEAPON_NONE,
        WEAPON_FIST,
        WEAPON_GLOCK,
        WEAPON_BURST,
        WEAPON_SHOTGUN,
        NUM_WEAPONS
    };
    enum {
        CONTROL_UP,
        CONTROL_RIGHT,
        CONTROL_DOWN,
        CONTROL_LEFT,
        CONTROL_SHOWNAMES,
        NUM_CONTROLS,
        CHARACTER_MAX_NAME_LENGTH = 32
    };

private:
    int m_PlayerIndex;
    std::string m_Name;
    SDL_Texture* m_Nameplate;
    double m_ColorHue;
    GameController* m_GameController;
    bool m_Movement[NUM_CONTROLS];
    int m_Controls[NUM_CONTROLS];
    bool m_Controllable;
    WeaponType m_Weapon;
    static const int sDefaultControls[NUM_CONTROLS];

    const double m_BaseAcceleration = 0.75;
    double m_xlook, m_ylook;  // direction

    void TickKeyboardControls();
    void TickGameControllerControls();
    void TickControls();

public:
    Character(GameWorld* world, double start_x, double start_y);
    ~Character();

    void SetGameController(GameController* gameController);
    GameController* GetGameController() const { return m_GameController; }
    int PlayerIndex() const { return m_PlayerIndex; }

    void Event(const SDL_Event& currentEvent);
    void Tick() override;
    void Draw() override;
};


#endif //TRIALANDERROR_CHARACTER_H
