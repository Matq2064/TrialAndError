//
// Created by Matq on 03/06/2023.
//

#include "ItemEntity.h"

Texture* ItemEntity::ms_TextureGlock = nullptr;

void ItemEntity::SetTexture(ItemType item_type) {
    switch (item_type) {
        case ITEM_GLOCK: {
            m_Texture = ms_TextureGlock;
        } break;
    }
}

ItemEntity::ItemEntity(GameWorld* world,
                       ItemType item_type,
                       double start_x, double start_y,
                       double start_w, double start_h)
    : Entity(world,
             ENTFORM_NORMAL,
             ENTTYPE_ITEM,
             start_x, start_y,
             start_w, start_h,
             0.0, 0.0,
             0.95) {
    m_ItemType = item_type;
    m_Texture = nullptr;

    SetTexture(item_type);
}

void ItemEntity::Tick() {

}

void ItemEntity::Draw() {
    Drawing* Render = m_World->GameWindow()->Render();

    SDL_Rect DrawRect = { int(m_Core.Pos.x - m_Core.Size.x / 2.0),
                          int(m_Core.Pos.y - m_Core.Size.y / 2.0),
                          int(m_Core.Size.x),
                          int(m_Core.Size.y) };

    Render->RenderTextureCamera(m_Texture->SDLTexture(), nullptr, DrawRect);
}
