//
// Created by janis on 3/22/2023.
//
#include <cmath>
#include <iostream>
#include "Bullets.h"
Bullets::Bullets(SDL_Renderer* Renderer, double start_x, double start_y)
        : Entity(Renderer, start_x, start_y, 100, 100){
    m_xvel = 10;
    m_yvel = 0;
    TickBullets();
}

void Bullets::TickBullets() {
    if((m_x < 900-25) and (m_x > 25)) m_x += m_xvel; // if on screen
    else if (m_x >= 900-25) m_x -= -m_xvel; // if going to the right
    else if (m_x <= 25)m_x += -m_xvel; // if going to the left
    if((m_y < 700-25) and (m_y > 25)) m_y += m_yvel; // if on screen
    else if(m_y >= 700-25) m_y -= -m_yvel; // if going below screen
    else if (m_y <= 25)m_y += -m_yvel; // if going above screen

}