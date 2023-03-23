//
// Created by 11dpjgailis on 16.03.2023.
//
#include "Character.h"
#include <cmath>
#include <iostream>
#include "Bullets.h"
#include <vector>

std::vector<Bullets*> Bullet;
static double sDiagonalLength = 1.0 / std::sqrt(2.0);
const int Character::sDefaultControls[NUM_CONTROLS] = {SDL_SCANCODE_W, SDL_SCANCODE_D, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_SPACE };

Character::Character(GameWorld* world, double start_x, double start_y)
 : Entity(world, GameWorld::ENTTYPE_CHARACTER, start_x, start_y, 50, 50, 0.93) {
    m_PlayerIndex = 0; // Must be initialized before assigning a new one
    m_ColorHue = double(rand()%360);

    m_GameController = nullptr;
    for (bool& State : m_Movement)
        State = false;

    m_PlayerIndex = world->NextPlayerIndex();
    if (m_PlayerIndex == 1) { memcpy(m_Controls, sDefaultControls, sizeof(m_Controls)); }  // Controls are copied
    else { memset(m_Controls, 0, sizeof(m_Controls)); }  // All controls are set to 0
    m_Controllable = true;
    m_Weapon = WEAPON_BURST;

    m_xvel = 0.0;
    m_yvel = 0.0;
    m_xlook = 1.0;
    m_ylook = 0.0;

    char Name[CHARACTER_MAX_NAME_LENGTH];
    std::snprintf(Name, CHARACTER_MAX_NAME_LENGTH, "Player%i", m_PlayerIndex);
    m_Name = Name;
    TextManager* TextHandler = world->GameWindow()->TextHandler();
    TTF_Font* Font = TextHandler->FirstFont();
    m_Nameplate = TextHandler->Render(Font, Name, { 255, 255, 255 });
}

Character::~Character() {

}

void Character::SetGameController(GameController* gameController) {
    m_GameController = gameController;
}

void Character::TickKeyboardControls() {
    // Check if buttons are held
    bool MoveUp = m_Movement[CONTROL_UP];
    bool MoveRight = m_Movement[CONTROL_RIGHT];
    bool MoveDown = m_Movement[CONTROL_DOWN];
    bool MoveLeft = m_Movement[CONTROL_LEFT];
    bool ShowNames = m_Movement[CONTROL_SHOWNAMES];

    bool Horizontally = MoveLeft != MoveRight;
    bool Vertically = MoveUp != MoveDown;

    // Accelerate when buttons are held
    double LengthPerAxis = (Horizontally && Vertically) ? sDiagonalLength : 1.0;
    double SpeedPerAxis = m_BaseAcceleration * LengthPerAxis;

    if (MoveDown != MoveUp) m_yvel += SpeedPerAxis * double(MoveDown ? 1 : -1);
    if (MoveRight != MoveLeft) m_xvel += SpeedPerAxis * double(MoveRight ? 1 : -1);

    // Show names button
    if (ShowNames)
        m_World->ShowNames();

    // Update look direction
    int XMouse, YMouse;
    SDL_GetMouseState(&XMouse, &YMouse);

    m_xlook = XMouse - m_x;
    m_ylook = YMouse - m_y;
    double Distance = std::sqrt(std::pow(m_xlook, 2) + std::pow(m_ylook, 2));

    if (Distance != 0.0) {
        m_xlook /= Distance;
        m_ylook /= Distance;
    } else {
        m_xlook = 1.0;
        m_ylook = 0.0;
    }
}

void Character::TickGameControllerControls() {
    // Check for current joystick values
    double AxisX, AxisY;
    m_GameController->GetJoystick1(AxisX, AxisY);

    // AxisX**2 + AxisY**2 <= 1 (keep direction length of 1)
    double Length = std::sqrt(std::pow(AxisX, 2) + std::pow(AxisY, 2));
    if (Length > 0.2) {  // Fix controller drifting
        if (Length > 0.0) {
            AxisX /= Length;
            AxisY /= Length;
        }

        // Accelerate in that direction
        m_xvel += m_BaseAcceleration * AxisX;
        m_yvel += m_BaseAcceleration * AxisY;
    }

    // Show names button
    bool ShowNames = m_GameController->GetButton(SDL_CONTROLLER_BUTTON_Y);  // triangle
    if (ShowNames)
        m_World->ShowNames();

    // Update look direction
    double AxisX2, AxisY2;
    m_GameController->GetJoystick2(AxisX2, AxisY2);

    Length = std::sqrt(std::pow(AxisX2, 2) + std::pow(AxisY2, 2));
    if (Length > 0.6) {  // Fix controller drifting
        if (Length != 0.0) {
            m_xlook = AxisX2 / Length;
            m_ylook = AxisY2 / Length;
        } else {
            m_xlook = 1.0;
            m_ylook = 0.0;
        }
    }

    //Shooting
    bool Shoot = m_GameController->GetButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    if (Shoot){
        m_GameController->Vibrate(0xffff, 0xffff, 50);
        if(m_Weapon == WEAPON_BURST){
            for(int x = 0; x < 5; x++){

                 new Bullets(m_World, m_x, m_y, m_xlook*10, m_ylook*10);
            }
        }
    }
}

void Character::TickControls() {
    if (!m_Controllable)
        return;

    if (m_GameController)
        TickGameControllerControls();
    else
        TickKeyboardControls();
}

void Character::Event(const SDL_Event& currentEvent) {
    if (!m_Controllable || m_GameController)
        return;

    if (currentEvent.type == SDL_KEYDOWN ||
        currentEvent.type == SDL_KEYUP) {
        bool State = currentEvent.type == SDL_KEYDOWN;
        for (int i = 0; i < NUM_CONTROLS; i++) {
            if (currentEvent.key.keysym.scancode == m_Controls[i])
                m_Movement[i] = State;
        }
    }
}

void Character::Tick() {
    TickControls();  // Do stuff depending on the current held buttons
    TickVelocity();  // Move the chracter entity
    TickWalls();
}

void Character::Draw() {
    Clock* Timer = m_World->GameWindow()->Timer();
    SDL_Renderer* Renderer = m_World->GameWindow()->Renderer();

    double Light = 0.5 + (std::sin(Timer->GetTotalTimeElapsed() - m_ExistsSince) + 1.0) / 4;
    SDL_Color Color = HSLtoRGB({ m_ColorHue, 1.0, Light });

    SDL_FRect DrawRect = {float(m_x) - float(m_w/2),
                          float(m_y) - float(m_h/2),
                          float(m_w),
                          float(m_h)};
    SDL_SetRenderDrawColor(Renderer, Color.r, Color.g, Color.b, 255);
    SDL_RenderFillRectF(Renderer, &DrawRect);

    double XLook = m_x + m_xlook * 50.0;
    double YLook = m_y + m_ylook * 50.0;
    Color = HSLtoRGB({ m_ColorHue, 1.0 - Light, 1.0 });
    SDL_SetRenderDrawColor(Renderer, Color.r, Color.g, Color.b, 255);
    SDL_RenderDrawLine(Renderer, int(m_x), int(m_y), int(XLook), int(YLook));

    if (!m_World->NamesShown())
        return;

    int w, h;
    SDL_QueryTexture(m_Nameplate, nullptr, nullptr, &w, &h);
    SDL_Rect NameplateRect = { int(m_x - w / 2.0), int(m_y - m_h / 2.0 - h), w, h };
    SDL_RenderCopy(Renderer, m_Nameplate, nullptr, &NameplateRect);
}
