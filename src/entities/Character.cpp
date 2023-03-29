//
// Created by 11dpjgailis on 16.03.2023.
//
#include "Character.h"
#include <cmath>
#include <iostream>
#include "Bullets.h"
#include <vector>

static double sDiagonalLength = 1.0 / std::sqrt(2.0);
const int Character::sDefaultControls[NUM_CONTROLS] = {SDL_SCANCODE_W, SDL_SCANCODE_D, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_SPACE };

Character::Character(GameWorld* world, double start_x, double start_y, double start_xvel, double start_yvel)
 : Entity(world, GameWorld::ENTTYPE_CHARACTER, start_x, start_y, 50, 50, 0.93) {
    m_PlayerIndex = 0;
    m_ColorHue = double(rand()%360);
    m_Weapon = WEAPON_GLOCK;
    m_Shoot = false;
    m_ShootSound = m_World->GameWindow()->SoundHandler()->LoadSound("assets/Shoot1.wav", true); // TODO: Shouldn't load sounds here
    m_GameController = nullptr;
    for (bool& State : m_Movement)
        State = false;

    m_World->GetNextPlayerIndex(this);
    if (m_PlayerIndex == 1) { memcpy(m_Controls, sDefaultControls, sizeof(m_Controls)); }  // Controls are copied
    else { memset(m_Controls, 0, sizeof(m_Controls)); }  // All controls are set to 0
    m_Controllable = true;

    m_xvel = start_xvel;
    m_yvel = start_yvel;
    m_xLook = 1.0;
    m_yLook = 0.0;
    m_LastShot = 0;

    m_xHook = 0.0;
    m_yHook = 0.0;
    m_xvelHook = 0.0;
    m_yvelHook = 0.0;
    m_HookDeployed = false;
    m_Hook = false;
    m_LastHook = false;
    m_HookGrabbedWall = false;

    char Name[CHARACTER_MAX_NAME_LENGTH];
    std::snprintf(Name, CHARACTER_MAX_NAME_LENGTH, "Player%i", m_PlayerIndex);
    m_Name = Name;
    TextManager* TextHandler = world->GameWindow()->TextHandler();
    TTF_Font* Font = TextHandler->FirstFont();
    m_Nameplate = TextHandler->Render(Font, Name, { 255, 255, 255 }, true);
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

    m_xLook = m_World->CameraX() - m_x + XMouse - m_World->GameWindow()->Width() / 2.0;
    m_yLook = m_World->CameraY() - m_y + YMouse - m_World->GameWindow()->Height() / 2.0;
    double Distance = std::sqrt(std::pow(m_xLook, 2) + std::pow(m_yLook, 2));

    if (Distance != 0.0) {
        m_xLook /= Distance;
        m_yLook /= Distance;
    } else {
        m_xLook = 1.0;
        m_yLook = 0.0;
    }

    auto MouseState = SDL_GetMouseState(nullptr, nullptr);
    m_Shoot = MouseState & SDL_BUTTON(SDL_BUTTON_LEFT);  // If clicked, shoot = true
    m_Hook = MouseState & SDL_BUTTON(SDL_BUTTON_RIGHT);
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
            m_xLook = AxisX2 / Length;
            m_yLook = AxisY2 / Length;
        } else {
            m_xLook = 1.0;
            m_yLook = 0.0;
        }
    }

    //Shooting
    m_Shoot = m_GameController->GetRightTrigger() > 0.7;
    m_Hook = m_GameController->GetButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
}

void Character::TickControls() {
    if (!m_Controllable)
        return;

    if (m_GameController) TickGameControllerControls();
    else TickKeyboardControls();
}

void Character::TickHook() {
    if (!m_HookDeployed && m_Hook && !m_LastHook) {
        m_HookDeployed = true;
        m_xHook = m_x;
        m_yHook = m_y;
        m_xvelHook = m_xLook * 35;
        m_yvelHook = m_yLook * 35;
    } else if (m_HookDeployed && !m_Hook && m_LastHook) {  // Instant retraction for now
        m_HookDeployed = false;
        m_HookGrabbedWall = false;
    }

    if (!m_HookDeployed)
        return;

    m_xHook += m_xvelHook;
    m_yHook += m_yvelHook;

    double TravelX = m_xHook - m_x;
    double TravelY = m_yHook - m_y;
    double Length = std::sqrt(std::pow(TravelX, 2) + std::pow(TravelY, 2));
    if (Length != 0.0) {
        TravelX /= Length;
        TravelY /= Length;
    }

    if (!m_HookGrabbedWall) {
        const double MaxLength = 300.0;
        if (Length > MaxLength) {
            m_xHook = m_x + TravelX * MaxLength;
            m_yHook = m_y + TravelY * MaxLength;
            m_xvelHook -= TravelX * 2.0;
            m_yvelHook -= TravelY * 2.0;
        }

        // Hook snaps to wall - fix later cus ugly
        if (m_xHook < 0.0) {
            m_xHook = 0.0;
            m_xvelHook = 0.0;
            m_yvelHook = 0.0;
            m_HookGrabbedWall = true;
        } else if (m_yHook < 0.0) {
            m_yHook = 0.0;
            m_xvelHook = 0.0;
            m_yvelHook = 0.0;
            m_HookGrabbedWall = true;
        } else if (m_xHook > m_World->Width()) {
            m_xHook = m_World->Width();
            m_xvelHook = 0.0;
            m_yvelHook = 0.0;
            m_HookGrabbedWall = true;
        } else if (m_yHook > m_World->Height()) {
            m_yHook = m_World->Height();
            m_xvelHook = 0.0;
            m_yvelHook = 0.0;
            m_HookGrabbedWall = true;
        }
    } else {
        m_xvel += TravelX * 1.5;
        m_yvel += TravelY * 1.5;
    }
}

void Character::TickWeapon() {
    if (!m_Shoot || m_Weapon == WEAPON_NONE)
        return;

    auto CurrentTick = m_World->CurrentTick();
    if (m_Weapon == WEAPON_GLOCK) {
        if (CurrentTick - m_LastShot < 24)
            return;
        m_LastShot = CurrentTick;
        m_World->GameWindow()->SoundHandler()->PlaySound(m_ShootSound);
        new Bullets(m_World, m_x, m_y, m_xLook * 10, m_yLook * 10);
        m_xvel += -m_xLook * 10;
        m_yvel += -m_yLook * 10;
    }
    else if (m_Weapon == WEAPON_BURST) {
        if (CurrentTick - m_LastShot < 64)
            return;
        m_LastShot = CurrentTick;
        m_World->GameWindow()->SoundHandler()->PlaySound(m_ShootSound);
        new Bullets(m_World, m_x , m_y , m_xLook * 10, m_yLook * 10);
        new Bullets(m_World, m_x*1.1 , m_y*1.1 , m_xLook * 10, m_yLook * 10);
        new Bullets(m_World, m_x*1.05 , m_y*1.05 , m_xLook * 10, m_yLook * 10);
        m_xvel += -m_xLook;
        m_yvel += -m_yLook;
    }
    else if (m_Weapon == WEAPON_SHOTGUN){
        if (CurrentTick - m_LastShot < 128)
            return;
        m_LastShot = CurrentTick;
        m_World->GameWindow()->SoundHandler()->PlaySound(m_ShootSound);
        new Bullets(m_World, m_x, m_y, m_xLook * 10, m_yLook * 10);
        new Bullets(m_World, m_x, m_y, (m_xLook+0.25) * 10, (m_yLook+0.25) * 10);
        new Bullets(m_World, m_x, m_y, (m_xLook+0.50) * 10, (m_yLook+0.50) * 10);
        new Bullets(m_World, m_x, m_y, (m_xLook-0.25) * 10, (m_yLook-0.25) * 10);
        new Bullets(m_World, m_x, m_y, (m_xLook-0.50) * 10, (m_yLook-0.50) * 10);
        m_xvel += -m_xLook * 30;
        m_yvel += -m_yLook * 30;
    }
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
    TickHook();
    TickWeapon();

    m_Shoot = false;  // Reset shooting at end of each tick
    m_LastHook = m_Hook;
}

void Character::Draw() {
    Clock* Timer = m_World->GameWindow()->Timer();
    Drawing* Render = m_World->GameWindow()->RenderClass();

    double Light = 0.5 + (std::sin(Timer->GetTotalTimeElapsed() - m_ExistsSince) + 1.0) / 4;
    SDL_Color Color = HSLtoRGB({ m_ColorHue, Light, 1.0 });

    if (m_HookDeployed) {
        Render->SetColor(Color.r, Color.g, Color.b, 255);
        Render->LineWorld(m_x, m_y, m_xHook, m_yHook);
    }

    SDL_FRect DrawRect = {float(m_x) - float(m_w/2),
                          float(m_y) - float(m_h/2),
                          float(m_w),
                          float(m_h)};
    Color = HSLtoRGB({ m_ColorHue, 1.0, Light });
    Render->SetColor(Color.r, Color.g, Color.b, 255);
    Render->FillRectFWorld(DrawRect);

    double XLook = m_x + m_xLook * 50.0;
    double YLook = m_y + m_yLook * 50.0;
    Color = HSLtoRGB({ m_ColorHue, 1.0 - Light, 1.0 });
    Render->SetColor(Color.r, Color.g, Color.b, 255);
    Render->LineWorld(int(m_x), int(m_y), int(XLook), int(YLook));

    if (m_World->NamesShown() <= 0.05)  // Visibility under 5% - don't render
        return;

    int Opacity = int(m_World->NamesShown() * 255.0);

    int nameplate_w, nameplate_h;
    m_Nameplate->Query(nullptr, nullptr, &nameplate_w, &nameplate_h);
    SDL_Rect NameplateRect = { int(m_x - nameplate_w / 2.0), int(m_y - m_h / 2.0 - nameplate_h), nameplate_w, nameplate_h };
    SDL_SetTextureAlphaMod(m_Nameplate->SDLTexture(), Opacity);
    Render->RenderTextureWorld(m_Nameplate->SDLTexture(), nullptr, NameplateRect);

    TextManager* TextHandler = m_World->GameWindow()->TextHandler();
    char msg[64];
    std::snprintf(msg, sizeof(msg), "%ix, %iy", int(m_x), int(m_y));
    auto CoordinateTexture = TextHandler->Render(TextHandler->FirstFont(), msg, { Color.r, Color.g, Color.b, 255 }, false);
    int coordinate_w, coordinate_h;
    CoordinateTexture->Query(nullptr, nullptr, &coordinate_w, &coordinate_h);
    SDL_Rect CoordinateRect = { int(m_x - coordinate_w / 2.0), int(NameplateRect.y - coordinate_h), coordinate_w, coordinate_h };
    SDL_SetTextureAlphaMod(CoordinateTexture->SDLTexture(), Opacity);
    Render->RenderTextureWorld(CoordinateTexture->SDLTexture(), nullptr, CoordinateRect);
    delete CoordinateTexture;
}
