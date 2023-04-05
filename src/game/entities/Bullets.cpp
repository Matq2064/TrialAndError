//
// Created by janis on 3/22/2023.
//

#include "Bullets.h"
#include <cmath>
#include <iostream>
#include "Character.h"
Texture* Bullets::ms_Texture = nullptr;

Bullets::Bullets(GameWorld* world, double start_x, double start_y, double start_xvel, double start_yvel)
 : Entity(world, GameWorld::ENTTYPE_BULLET, start_x, start_y, 8, 8, 1.0){
    m_xvel = start_xvel;
    m_yvel = start_yvel;
}

void Bullets::TickImpact() {
    if (m_x < 0 || m_x > m_World->Width() ||
        m_y < 0 || m_y > m_World->Height())
        delete this;
    else for (auto Current = m_World->GetPlayers(); Current != nullptr; Current = (Character*)Current->m_PrevEntityType) {
        if ((Current->m_x-25 < m_x) & (Current->m_x+25 > m_x) & (Current->m_y-25 < m_y) & (Current->m_y+25 > m_y)){
            Current->hp -=10;

            delete this;
        }
    }
}

void Bullets::Tick() {
    TickVelocity();
    TickImpact();
}

void Bullets::Draw() {
    Drawing* Render = m_World->GameWindow()->RenderClass();

    SDL_Rect BulletRect = { int(m_x - m_w / 2.0), int(m_y - m_h / 2.0), int(m_w), int(m_h) };
    Render->RenderTextureWorld(ms_Texture->SDLTexture(), nullptr, BulletRect);
}
