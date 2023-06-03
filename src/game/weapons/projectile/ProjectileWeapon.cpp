//
// Created by Matq on 30/03/2023.
//

#include "ProjectileWeapon.h"
#include "../../entities/character/Character.h"
#include "../../entities/Projectile.h"
#include <cmath>

Sound* ProjectileWeapon::ms_ReloadSound = nullptr;
Sound* ProjectileWeapon::ms_NoAmmo = nullptr;

double ProjectileWeapon::GenerateSpreadAngle() const {
    return (double(rand() % m_FullRandomSpread) - m_HalfRandomSpread) / m_RandomSpreadDivisor;
}

double ProjectileWeapon::GenerateRandomProjectileSpeed() const {
    return m_ProjectileSpeed + ((rand() % m_FullRandomProjectileSpeed)
        - m_NegativeRandomProjectileSpeed) / m_RandomProjectileSpeedDivisor;
}

ProjectileWeapon::ProjectileWeapon(Character* owner,
                                   WeaponType type,
                                   int tick_cooldown,
                                   int ammo_capacity,
                                   int total_ammo_capacity,
                                   double projectile_speed,
                                   bool automatic) {
    m_Shooter = owner;
    m_Type = type;
    m_LastShotAt = 0;
    m_TickCooldown = tick_cooldown;
    m_AmmoCapacity = ammo_capacity;
    m_Ammo = m_AmmoCapacity;
    m_TrueAmmoCapacity = total_ammo_capacity;
    m_TrueAmmo = m_TrueAmmoCapacity;
    m_ProjectileSpeed = projectile_speed;
    m_Automatic = automatic;
    m_Triggered = false;
    m_LastShot = false;

    m_RandomSpreadDivisor = 0.0;
    m_HalfRandomSpread = 0.0;
    m_FullRandomSpread = 0;
    m_RandomProjectileSpeedDivisor = 0.0;
    m_NegativeRandomProjectileSpeed = 0.0;
    m_FullRandomProjectileSpeed = 0;
}

void ProjectileWeapon::TickTrigger() {
    if (m_Shooter) {
        bool Shoot = (Character*) (m_Shooter)->m_Input.m_Shooting;
        bool LastShoot = (Character*) (m_Shooter)->m_LastInput.m_Shooting;

        m_Triggered = Shoot && !LastShoot;  // Always trigger on semi
        if (!m_Triggered) {
            bool Auto = Shoot && m_Automatic;
            m_Triggered = Auto && (m_Ammo || m_LastShot);
        }
    }
}

unsigned int ProjectileWeapon::NeededAmmo() const {
    return m_TrueAmmoCapacity - m_TrueAmmo;
}

void ProjectileWeapon::AddTrueAmmo(unsigned int count) {
    m_TrueAmmo += count;
}

void ProjectileWeapon::SetSpread(double degrees, int decimal_places) {
    m_RandomSpreadDivisor = std::pow(10, decimal_places);
    m_HalfRandomSpread = degrees / 180.0 * M_PI * m_RandomSpreadDivisor;
    m_FullRandomSpread = int(m_HalfRandomSpread * 2.0) + 1;
}

void ProjectileWeapon::SetRandomProjectileSpeed(double delta_speed,
                                                double delta_percentage_negative,
                                                int delta_decimal_places) {
    m_RandomProjectileSpeedDivisor = std::pow(10, delta_decimal_places);
    double FullSpeed = delta_speed * m_RandomProjectileSpeedDivisor;
    m_NegativeRandomProjectileSpeed = FullSpeed * delta_percentage_negative;
    m_FullRandomProjectileSpeed = int(FullSpeed) + 1;
}

void ProjectileWeapon::Reload() {
    if (m_TrueAmmo != 0) {
        m_Shooter->World()->GameWindow()->Assets()->SoundHandler()->PlaySound(ms_ReloadSound);
    } else {
        m_Shooter->World()->GameWindow()->Assets()->SoundHandler()->PlaySound(ms_NoAmmo);
    }
    unsigned int AmmoNeeded = m_AmmoCapacity - m_Ammo;
    if (m_TrueAmmo >= AmmoNeeded) {
        m_TrueAmmo -= AmmoNeeded;
        m_Ammo = m_AmmoCapacity;
    } else {
        m_Ammo += m_TrueAmmo;
        m_TrueAmmo = 0;
    }
}

void ProjectileWeapon::Tick() {
    TickTrigger();
}
