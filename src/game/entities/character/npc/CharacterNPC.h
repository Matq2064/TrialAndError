//
// Created by 11dpmbozko on 18.05.2023.
//

#ifndef TRIALANDERROR_CHARACTERNPC_H
#define TRIALANDERROR_CHARACTERNPC_H

#include "../Character.h"

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
                 double max_health,
                 const Vec2d& start_pos,
                 const Vec2d& start_vel,
                 NPCType ai_type,
                 bool is_boss);
    ~CharacterNPC();

};

#endif //TRIALANDERROR_CHARACTERNPC_H
