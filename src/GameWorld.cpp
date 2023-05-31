//
// Created by Matq on 22/03/2023.
//

#include "GameWorld.h"
#include "game/Player.h"
#include "game/entities/Entity.h"
#include "game/entities/character/Character.h"
#include "game/entities/character/npc/CharacterNPC.h"
#include <cmath>
Texture* GameWorld::Chad = nullptr;

GameWorld::GameWorld(GameReference* game_window, int width, int height) {
    m_GameWindow = game_window;
    m_Tiles = new TileMap(game_window->RenderClass(), 32, width, height);
    m_Width = m_Tiles->TotalWidth();
    m_Height = m_Tiles->TotalHeight();
    m_ShowNamesVisibility = 0.0;
    m_ShowNames = false;
    m_Paused = false;
    m_x = 0.0;
    m_y = 0.0;
    m_CurrentTick = 0;

    m_First = nullptr;
    m_Last = nullptr;
    for (auto& i : m_FirstType) i = nullptr;
    for (auto& i : m_LastType) i = nullptr;
    m_FirstPlayer = nullptr;
    m_LastPlayer = nullptr;

    char msg[32];
    std::snprintf(msg, sizeof(msg), "Spawned [%ix, %iy]", (int)m_x, (int)m_y);
    m_CoordinatePlate = new TextSurface(m_GameWindow->Assets(),
                                    m_GameWindow->Assets()->TextHandler()->FirstFont(),
                                    msg, { 255, 255, 255 });

    m_Background = GameWindow()->Assets()->ImageHandler()->LoadTexture("assets/images/backgrounds/background_pattern.png", true);
    m_Background->Query(nullptr, nullptr, &m_BackgroundW, &m_BackgroundH);
    //SDL_SetTextureAlphaMod(m_Background->SDLTexture(), 10);
    //SDL_SetTextureBlendMode(m_Background->SDLTexture(), SDL_BLENDMODE_BLEND);

    m_LastWave = 0;
    m_TimeBetweenWaves = 300;
    m_NumEnemiesPerWave = 1;
    m_Round = 0;
}

GameWorld::~GameWorld() {
    delete m_Tiles;
    delete m_CoordinatePlate;


    Entity* CurrentEntity = m_Last;
    while (CurrentEntity) {
        auto NextEntity = CurrentEntity->m_Prev;
        delete CurrentEntity;
        CurrentEntity = NextEntity;
    }
}

unsigned int GameWorld::NextPlayerIndex() const {
    unsigned int Index = 0;
    auto pPlayer = m_FirstPlayer;

    while (true) {
        bool Used = false;
        for (; pPlayer != nullptr; pPlayer = pPlayer->m_Next) {
            if (pPlayer->GetIndex() == Index)
                Used = true;
        }

        if (!Used) return Index;
        Index++;
    }
}

void GameWorld::GetPointInWorld(double relative_x, double relative_y, double& out_x, double& out_y) const {
    out_x = m_x + (relative_x - m_GameWindow->Width() / 2.0);
    out_y = m_y + (relative_y - m_GameWindow->Height() / 2.0);
}

void GameWorld::EnemiesKilled() {
    if (m_CurrentTick - m_LastWave > 600)
        m_LastWave = m_CurrentTick - m_TimeBetweenWaves + 600;
}

void GameWorld::SetCameraPos(double x, double y) {
    m_x = x;
    m_y = y;
}

Player* GameWorld::AddPlayer(Player* player) {
    if (!m_FirstPlayer) {
        m_FirstPlayer = player;
        m_LastPlayer = player;
        player->m_Prev = nullptr;
        player->m_Next = nullptr;
    } else {
        m_LastPlayer->m_Next = player;
        player->m_Prev = m_LastPlayer;
        m_LastPlayer = player;
    }

    return player;
}

// ::RemovePlayer() doesn't reset players Previous and Next player pointers
void GameWorld::RemovePlayer(Player* player) {
    // Remove player from list of all players
    if (player->m_Prev) player->m_Prev->m_Next = player->m_Next;
    if (player->m_Next) player->m_Next->m_Prev = player->m_Prev;
    if (m_FirstPlayer == player) m_FirstPlayer = player->m_Next;
    if (m_LastPlayer == player) m_LastPlayer = player->m_Prev;
}

Entity* GameWorld::AddEntity(Entity* entity) {
    EntityType Enttype = entity->EntityType();
    Entity*& FirstType = m_FirstType[Enttype];
    Entity*& LastType = m_LastType[Enttype];

    if (!FirstType) { // Then there also shouldn't be a last type
        FirstType = entity;
        LastType = entity;
        entity->m_PrevType = nullptr;
        entity->m_NextType = nullptr;
    } else { // Then there also should be a last type
        LastType->m_NextType = entity;
        entity->m_PrevType = LastType;
        LastType = entity;
    }

    if (!m_First) {
        m_First = entity;
        m_Last = entity;
        entity->m_Prev = nullptr;
        entity->m_Next = nullptr;
    } else {
        m_Last->m_Next = entity;
        entity->m_Prev = m_Last;
        m_Last = entity;
    }

    return entity;
}

// ::RemoveEntity() doesn't reset entities Previous and Next entity pointers
void GameWorld::RemoveEntity(Entity* entity) {
    EntityType Type = entity->EntityType();
    Entity*& FirstType = m_FirstType[Type];
    Entity*& LastType = m_LastType[Type];

    // Remove entity from list of same type
    if (entity->m_PrevType) entity->m_PrevType->m_NextType = entity->m_NextType;
    if (entity->m_NextType) entity->m_NextType->m_PrevType = entity->m_PrevType;
    if (FirstType == entity) FirstType = entity->m_NextType;
    if (LastType == entity) LastType = entity->m_PrevType;

    // Remove entity from list of all entities
    if (entity->m_Prev) entity->m_Prev->m_Next = entity->m_Next;
    if (entity->m_Next) entity->m_Next->m_Prev = entity->m_Prev;
    if (m_First == entity) m_First = entity->m_Next;
    if (m_Last == entity) m_Last = entity->m_Prev;
}

void GameWorld::DestroyPlayerByController(GameController* DeletedController) const {
    Player* Plr = m_FirstPlayer;
    for (; Plr; Plr = Plr->m_Next) {
        Character* Char = Plr->GetCharacter();
        if (Char->GetGameController() == DeletedController) {
            delete Plr;
            return;
        }
    }
}

void GameWorld::DestroyCharacterByController(GameController* DeletedController) const {
    Character* Char = FirstPlayer();
    for (; Char; Char = (Character*)(Char->NextType())) {
        if (Char->GetGameController() == DeletedController) {
            delete Char;
            return;
        }
    }
}

void GameWorld::ToggleShowNames() {
    m_ShowNames = !m_ShowNames;
    if (m_ShowNames)
        m_ShowNamesVisibility = 1.0;
}

void GameWorld::SetPaused(bool state) {
    m_Paused = state;
}

void GameWorld::Event(const SDL_Event& currentEvent) {
    if (m_Paused) return;

    if (currentEvent.type == SDL_KEYDOWN) {
        if (currentEvent.key.keysym.scancode == SDL_SCANCODE_SPACE)
            ToggleShowNames();
        else if (currentEvent.key.keysym.scancode == SDL_SCANCODE_O) {
            m_Tiles->LoadTilemap("assets/tilemaps/test_level");
            m_Width = m_Tiles->TotalWidth();
            m_Height = m_Tiles->TotalHeight();
        }
        else if (currentEvent.key.keysym.scancode == SDL_SCANCODE_P)
            m_Tiles->SaveTilemap("assets/tilemaps/test_level");
    }

    // Loop allows self-destruction in the ::Event() method
    Character* Next, *Current = FirstPlayer();
    for (; Current; Current = Next) {
        Next = (Character*)(Current->NextType());
        Current->Event(currentEvent);
    }
}

void GameWorld::TickCamera() {
    if (m_FirstType[ENTTYPE_CHARACTER]) {
        int num_player = 0;

        double CameraX = 0.0;
        double CameraY = 0.0;

        auto Char = FirstPlayer();
        for (; Char; Char = (Character*)Char->NextType()) {
            if (Char->IsNPC())
                continue;

            num_player++;

            EntityCore* CharCore = Char->GetCore();
            CameraX += CharCore->m_x;
            CameraY += CharCore->m_y;
        }

        if (num_player) {
            CameraX /= num_player;
            CameraY /= num_player;
            // Accelerate camera closer to
            // the midpoint of characters
            // TODO: Zoom value
            m_x += (-m_x + CameraX) * 0.1;
            m_y += (-m_y + CameraY) * 0.1;
        }
    }
}

void GameWorld::TickSpawner() {
    if (m_CurrentTick - m_LastWave < m_TimeBetweenWaves)
        return;

    m_LastWave = m_CurrentTick;
    m_Round += 1;
    m_TimeBetweenWaves = (unsigned long long)((5 + m_Round * 3) * m_GameWindow->Timer()->GetFramerate());
    m_NumEnemiesPerWave = 2 + int(m_Round / 0.75);

    double Width2 = m_Width / 2.0;
    double Height2 = m_Height / 2.0;

    for (int i = 0; i < m_NumEnemiesPerWave; i++) {
        double Angle = (180.0 + (rand()%180)) / 180.0 * M_PI;
        double XSpawn = Width2 + std::cos(Angle) * Width2;
        double YSpawn = Height2 + std::sin(Angle) * Height2;
        auto NewNPC = new CharacterNPC(this, 10.0 + m_Round, XSpawn, YSpawn, 0.0, 0.0, NPC_TURRET);
        int Weaponizer = rand()%100;
        if (m_Round >= 20) {
            if (Weaponizer < 10) NewNPC->GiveWeapon(WEAPON_GLOCK);
            else if (Weaponizer < 20) NewNPC->GiveWeapon(WEAPON_SHOTGUN);
            else if (Weaponizer < 30) NewNPC->GiveWeapon(WEAPON_BURST);
            else if (Weaponizer < 40) NewNPC->GiveWeapon(WEAPON_MINIGUN);
        } else if (m_Round >= 15) {
            if (Weaponizer < 10) NewNPC->GiveWeapon(WEAPON_GLOCK);
            else if (Weaponizer < 20) NewNPC->GiveWeapon(WEAPON_SHOTGUN);
            else if (Weaponizer < 30) NewNPC->GiveWeapon(WEAPON_BURST);
        } else if (m_Round >= 10) {
            if (Weaponizer < 10) NewNPC->GiveWeapon(WEAPON_GLOCK);
            else if (Weaponizer < 20) NewNPC->GiveWeapon(WEAPON_SHOTGUN);
        } else if (m_Round >= 5) {
            if (Weaponizer < 10) NewNPC->GiveWeapon(WEAPON_GLOCK);
        }
    }
}

void GameWorld::TickEntities() {
    // Loop through each entity and tick
    for (auto Current = m_First; Current; Current = Current->m_Next)
        Current->Tick();
}

void GameWorld::TickDestroy() {
    // Loop through each entity and destroy
    Entity* Current, *Next;
    for (Current = m_First; Current; Current = Next) {
        Next = Current->m_Next;
        if (!Current->IsAlive())
            delete Current;
    }
}

void GameWorld::Tick() {
    if (m_Paused)
        return;

    if (!m_ShowNames)
        m_ShowNamesVisibility *= 0.98;

    TickSpawner();
    TickEntities();
    TickDestroy();
    TickCamera();

    if (NamesShown() > 0.05 &&
        ((int)(m_x) != (int)(m_xLast) ||
        (int)(m_y) != (int)(m_yLast)))
        m_CoordinatePlate->FlagForUpdate();

    m_CurrentTick++;
    m_xLast = m_x;
    m_yLast = m_y;
}

void GameWorld::Draw() {
    Drawing* Render = m_GameWindow->RenderClass();

    SDL_Rect DestinationRect = {0, 0, int(m_Width), int(m_Height) };
    Render->RenderTextureWorld(m_Background->SDLTexture(), nullptr, DestinationRect);

    SDL_Rect DrawRect = { 0, 0, int(m_Width), int(m_Height) };
    Render->SetColor(255, 0, 0, 255);
    Render->DrawRectWorld(DrawRect);

    for (auto Current : m_FirstType) {
        for (; Current; Current = Current->NextType())
            Current->Draw();
    }

    m_Tiles->Draw();

    if (m_ShowNamesVisibility <= 0.05)
        return;

    int Opacity = int(m_ShowNamesVisibility * 255.0);

    char msg[64];
    std::snprintf(msg, sizeof(msg), "%ix, %iy", int(m_x), int(m_y));
    m_CoordinatePlate->SetText(msg);
    Texture* CoordinateTexture = m_CoordinatePlate->RequestUpdate();

    int coordinate_w, coordinate_h;
    CoordinateTexture->Query(nullptr, nullptr, &coordinate_w, &coordinate_h);
    SDL_Rect CoordinateRect = { int(m_x - coordinate_w / 2.0), int(m_y - coordinate_h / 2.0), coordinate_w, coordinate_h };
    SDL_SetTextureAlphaMod(CoordinateTexture->SDLTexture(), Opacity);
    Render->RenderTextureWorld(CoordinateTexture->SDLTexture(), nullptr, CoordinateRect);
}
