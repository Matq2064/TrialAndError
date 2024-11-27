//
// Created by janis on 3/22/2023.
//

#include "Projectile.h"
#include <cmath>
#include <iostream>
#include "Crate.h"
#include "characters/character/Character.h"

Texture* Projectile::ms_TextureGlock = nullptr;
Texture* Projectile::ms_TextureBurst = nullptr;
Texture* Projectile::ms_TextureShotgun = nullptr;
Texture* Projectile::ms_TextureMinigun = nullptr;

Projectile::Projectile(GameWorld* world,
                       Entity* shooter,
                       WeaponType weapon_type,
                       double damage,
                       const Vec2d& start_pos,
                       const Vec2d& start_vel)
    : Entity(world,
             NORMAL_ENTITY,
             PROJECTILE_ENTITY,
             start_pos,
             Vec2d(6, 10),
             start_vel,
             1.0,
             false) {
    switch (weapon_type) {
        case WEAPON_GLOCK: {
            m_Texture = ms_TextureGlock;
        }
            break;
        case WEAPON_BURST: {
            m_Texture = ms_TextureBurst;
        }
            break;
        case WEAPON_SHOTGUN: {
            m_Texture = ms_TextureShotgun;
        }
            break;
        case WEAPON_MINIGUN: {
            m_Texture = ms_TextureMinigun;
        }
            break;
        case WEAPON_SNIPER: {
            m_Texture = ms_TextureGlock;
        }
    }
    m_Shooter = shooter;
    m_Damage = damage;
    m_StillCollidesShooter = true;
}

void Projectile::TickCollision() {
    // Sense
    bool ShooterIsCharacter = m_Shooter->GetType() == CHARACTER_ENTITY;
    auto ShooterCharacter = (Character*)m_Shooter; // ⚠ Check for ShooterIsCharacter ⚠
    // Check if position collides any of the players
    auto HealthEntity = m_World->FirstHasHealth();
    for (; HealthEntity; HealthEntity = HealthEntity->NextHasHealth()) {
        auto Ent = (Entity*)HealthEntity;
        bool IsShooter = m_Shooter == Ent;
        if (!Ent->IsAlive()) continue;

        // Ignore npc friendly fire for now
//        if (HasHealthComponent->GetType() == CHARACTER) {
//            auto ShootableCharacter = (Character*)HasHealthComponent;
//            if (ShooterIsCharacter && ShooterCharacter->IsNPC() == ShootableCharacter->IsNPC())
//                continue;
//        }

        // Check for (Projectile <-> Entity) collision at the position
        bool Collides = Ent->PointCollides(m_Core.Pos);
        if (IsShooter && !Collides) { m_StillCollidesShooter = false; }
        else if (Collides && (!IsShooter || !m_StillCollidesShooter)) {
            if (Ent->GetType() == CHARACTER_ENTITY) {
                auto ShootableCharacter = (Character*)Ent;
                ShootableCharacter->Damage(m_Damage, Ent);
                ShootableCharacter->Accelerate(m_Core.Vel * 0.05);
            } else {
                HealthEntity->Damage(m_Damage, nullptr);
            }

            // The projectile has served its purpose (clear immediately on impact)
            m_Alive = false;
            break;
        }
    }
}

void Projectile::TickWallCollision() {
    // Deletes the bullet if it hits the border
    if (m_Core.Pos.x < 0 || m_Core.Pos.x > m_World->GetWidth() ||
        m_Core.Pos.y < 0 || m_Core.Pos.y > m_World->GetHeight())
        m_Alive = false;
}

void Projectile::Tick() {
    TickVelocity();
    TickCollision();
    TickWallCollision();
    TickUpdateLastCore();
}

void Projectile::Draw() {
    Drawing* Render = m_World->GameWindow()->Render();
    double Angle = std::atan2(m_Core.Vel.y, m_Core.Vel.x) / M_PI * 180.0 + 90.0;
    SDL_Rect BulletRect = { int(m_Core.Pos.x - m_Core.Size.x / 2.0),
                            int(m_Core.Pos.y - m_Core.Size.y / 2.0),
                            int(m_Core.Size.x),
                            int(m_Core.Size.y) };
    Render->RenderTextureExCamera(m_Texture->SDLTexture(), nullptr, BulletRect, Angle, nullptr, SDL_FLIP_NONE);
}
