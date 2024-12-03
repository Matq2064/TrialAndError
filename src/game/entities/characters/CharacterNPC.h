//
// Created by 11dpmbozko on 18.05.2023.
//

#pragma once

#include "character/Character.h"

enum NPCType {
    NPC_DUMMY,
    NPC_TURRET
};

class CharacterNPC : public Character {
protected:
    bool m_IsBoss;
    NPCType m_AIType;

    // Listening & Ticking
    void EventDeath() override;
    void TickControls() override;

public:
    CharacterNPC(GameWorld* world,
                 const Vec2d& start_pos,
                 const Vec2d& start_vel,
                 NPCType ai_type,
                 bool is_boss);
    ~CharacterNPC() override;

    // Generating
    [[nodiscard]] const char* toString() const override;
};
