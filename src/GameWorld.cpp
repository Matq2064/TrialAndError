//
// Created by Matq on 22/03/2023.
//

#include "GameWorld.h"
#include "entities/Entity.h"
#include "entities/Character.h"

GameWorld::GameWorld(GameReference* gameWindow, double width, double height) {
    m_GameWindow = gameWindow;
    m_Width = width;
    m_Height = height;
    m_LastEntity = nullptr;
    m_Paused = false;
}

GameWorld::~GameWorld() {
    Entity* CurrentEntity = m_LastEntity;
    while (CurrentEntity) {
        auto NextEntity = CurrentEntity->m_PrevEntity;
        delete CurrentEntity;
        CurrentEntity = NextEntity;
    }
}

int GameWorld::NextPlayerIndex() {
    int Index = 1;
    while (true) {
        for (auto Current = m_LastEntity; Current != nullptr; Current = Current->m_NextEntity) {
            if (Current->EntityType() != ENTTYPE_CHARACTER)
                continue;

            auto CurrentPlayer = (Character *) Current;
            if (CurrentPlayer->PlayerIndex() == Index) {
                Index++;
                continue;
            }

            return Index;
        }
    }
    lol;
}

void GameWorld::AddEntity(Entity* entity) {
    if (!m_LastEntity) {
        m_LastEntity = entity;
        entity->m_PrevEntity = nullptr;
        entity->m_NextEntity = nullptr;
    } else {
        m_LastEntity->m_NextEntity = entity;
        entity->m_PrevEntity = m_LastEntity;
        m_LastEntity = entity;
    }
}

void GameWorld::RemoveEntity(Entity* entity) {
    if (m_LastEntity == entity) {
        if (entity->m_PrevEntity) {
            m_LastEntity = entity->m_PrevEntity;
            m_LastEntity->m_NextEntity = nullptr;
        } else { m_LastEntity = nullptr; }
    } else {
        Entity* TempNext = entity->m_NextEntity;
        if (entity->m_NextEntity) { entity->m_NextEntity->m_PrevEntity = entity->m_PrevEntity; }
        if (entity->m_PrevEntity) { entity->m_PrevEntity->m_NextEntity = TempNext; }
    }
}

void GameWorld::DestroyPlayerByController(GameController* DeletedController) {
    for (Entity* CurrentEntity = m_LastEntity; CurrentEntity != nullptr; CurrentEntity = CurrentEntity->m_PrevEntity) {
        if (CurrentEntity->EntityType() != ENTTYPE_CHARACTER)
            continue;

        auto CurrentPlayer = (Character*)CurrentEntity;
        if (CurrentPlayer->GetGameController() != DeletedController)
            continue;

        delete CurrentEntity;
        return;
    }
}

void GameWorld::ShowNames() {
    m_ShowNames = 1.0;
}

void GameWorld::SetPaused(bool state) {
    m_Paused = state;
}

void GameWorld::Event(const SDL_Event& currentEvent) {
    Entity* NextEntity; // allows deletion while looping
    for (Entity* CurrentEntity = m_LastEntity; CurrentEntity != nullptr; CurrentEntity = NextEntity) {
        NextEntity = CurrentEntity->m_PrevEntity;
        if (CurrentEntity->EntityType() != ENTTYPE_CHARACTER)
            continue;

        auto CurrentPlayer = (Character*)CurrentEntity;
        CurrentPlayer->Event(currentEvent);
    }
}

void GameWorld::Tick() {
    if (m_Paused)
        return;

    m_ShowNames *= 0.95;

    Entity* NextEntity; // allows deletion while looping
    for (Entity* CurrentEntity = m_LastEntity; CurrentEntity != nullptr; CurrentEntity = NextEntity) {
        NextEntity = CurrentEntity->m_PrevEntity;
        CurrentEntity->Tick();
    }
}

void GameWorld::Draw() {
    SDL_Renderer* Renderer = m_GameWindow->Renderer();

    SDL_Rect DrawRect = { 0, 0, int(m_Width), int(m_Height) };
    SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(Renderer, &DrawRect);

    for (Entity* CurrentEntity = m_LastEntity; CurrentEntity != nullptr; CurrentEntity = CurrentEntity->m_PrevEntity)
        CurrentEntity->Draw();
}
