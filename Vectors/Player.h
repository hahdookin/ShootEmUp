#ifndef PLAYER_H
#define PLAYER_H

#include "olcPixelGameEngine.h"
#include "ShootEmUp.h"

class Player
{
private:
	// Starting player pos and velocity and other things
	olc::vf2d m_pos = {};
	olc::vf2d m_rotPosRight = {};
	olc::vf2d m_rotPosLeft = {};
	olc::vf2d m_rotPosNose = {};

	std::vector<Upgrade> m_vUpgrades = {};
	std::vector<Bullet> m_vBullets = {};
	std::vector<Bullet> m_vBulletsPerShot = {};

	float m_fVelocity = 0.0f;
	float m_fAcceleration = 150.0f;
	float m_fMaxVelocity = 150.0f;

	float m_fRotation = 0.0f;
	float m_fRotAcceleration = 45.0f;
	float m_fRotVelocity = 0.0f;
	float m_fRotMaxVelocity = 7.0f;

	float m_lineRay = 15.0f; // Magnitude of triangle edges
	float m_fInnerShipAngle = PI / 6.0f;
	float m_fShotDelay = 0.3f;
	float m_fShotSpeed = 300.0f;

public:
	Player(olc::vf2d pos);
	Player(float x, float y);

	void Move(float elapsedTime);
	void UpdateRotPositions();
	void DrawShip(olc::PixelGameEngine* pge);

	void AddBullet(Bullet& b);
	void CreateBullets();
	void Shoot(std::vector<Bullet>& gameLoopBullets);
	void EraseBullets();
	void ApplyUpgrade(uint32_t upgrade);

	const olc::vf2d& GetPos();
	void SetPos(olc::vf2d pos);
	void SetPos(float x, float y);

	float GetPosX();
	float GetPosY();

	void SetPosX(float x);
	void SetPosY(float y);

	const olc::vf2d& GetPosNose();

	float GetVelocity();
	void SetVelocity(float velocity);

	float GetMaxVelocity();
	void SetMaxVelocity(float velocity);

	float GetAcceleration();
	void SetAcceleration(float acceleration);

	float GetRotation();
	void SetRotation(float angle);

	float GetRotAcceleration();
	void SetRotAcceleration(float acceleration);

	float GetRotVelocity();
	void SetRotVelocity(float velocity);

	float GetRotMaxVelocity();
	void SetRotMaxVelocity(float velocity);

	float GetLineRay();
	void SetLineRay(float length);

	float GetInnerShipAngle();
	void SetInnerShipAngle(float angle);

	float GetShotDelay();
	void SetShotDelay(float delay);

	float GetShotSpeed();
	void SetShotSpeed(float speed);
};


#endif
