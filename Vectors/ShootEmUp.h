//#pragma once
#ifndef SHOOTEMUP_H
#define SHOOTEMUP_H

#include "olcPixelGameEngine.h"

constexpr float PI = 3.14159265359f;
constexpr float TWO_PI = PI * 2.0f;

enum class Upgrade
{
	Double_Shot,
	Triple_Shot,
	Quad_Shot,
	Homing_Shot,
	Piercing_Shot,
	Shrink_Shot,
	Decrease_Shot_Delay,
	Increase_Shot_Size,
	More_Options,
	Increase_Speed,
	Increase_Mod_Speed,
	Incrase_Size,
	Decrease_Size,
	Total_Upgrades
};

// Declarations
class Player
{
private:
	// Starting player pos and velocity and other things
	olc::vf2d m_pos = {};
	olc::vf2d m_rotPosRight = {};
	olc::vf2d m_rotPosLeft = {};
	olc::vf2d m_rotPosNose = {};

	std::vector<Upgrade> m_vUpgrades = {};

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

	olc::vf2d GetPos();
	void SetPos(olc::vf2d pos);
	void SetPos(float x, float y);

	float GetPosX();
	float GetPosY();

	void SetPosX(float x);
	void SetPosY(float y);

	olc::vf2d GetPosNose();

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
struct Entity
{
	olc::vf2d pos;
	float speed;
	float angle;

	float GetAngleToEntity(const olc::vf2d& entityPos);
	virtual void SetAngleToEntity(const olc::vf2d& entityPos);
	void Move(float elapsedTime);
};
struct Bullet : Entity
{
	int radius;
	int damage;

	Bullet(olc::vf2d pos, float speed, float angle, int radius = 2, int damage = 1);
	Bullet(float x, float y, float speed, float angle, int radius = 2, int damage = 1);
	void DrawYourself(olc::PixelGameEngine* pge, bool friendly);
};
struct Enemy : Entity
{
	float radius;
	int healthPoints;
	bool isHit = false;
	olc::Pixel color;

	void ReduceHP(int points);
	virtual bool WillFire();
	void DrawYourself(olc::PixelGameEngine* pge);
};
struct ShootingEnemy : Enemy
{
	ShootingEnemy(olc::vf2d pos, float speed, int hp, olc::vf2d offset = { 0.0f, 0.0f });

	ShootingEnemy(float x, float y, float speed, int hp, olc::vf2d offset = { 0.0f, 0.0f });
	bool WillFire() override;
};
struct BruteEnemy : Enemy
{
	BruteEnemy(olc::vf2d pos, float speed, int hp, olc::vf2d offset = { 0.0f, 0.0f });

	BruteEnemy(float x, float y, float speed, int hp, olc::vf2d offset = { 0.0f, 0.0f });
};

struct Particle : Entity
{
	Particle(olc::vf2d pos);

	Particle(float x, float y);
	void DrawYourself(olc::PixelGameEngine* pge);
};



void GenerateRandCards(std::vector<std::pair<std::string, std::string>>& vCards, int nCards);
bool Between(float n, float min, float max);
olc::Pixel RandColor();
void FadeInPixel(olc::Pixel& p, float speed, float elapsedTime);
bool Between(float n, float min, float max);
void MoveEntity(Entity& e, float elapsedTime);
float absmag(const olc::vf2d& v1, const olc::vf2d& v2);
void DrawStars(olc::PixelGameEngine* pge, std::array<olc::vf2d, 1000>& arr, float elapsedTime, bool update);


// More concise erase remove idiom
template<class T, typename Lambda>
bool RemoveLambda(std::vector<T>& v, Lambda&& func)
{
	v.erase(std::remove_if(v.begin(), v.end(), func), v.end());
	return false;
}


#endif