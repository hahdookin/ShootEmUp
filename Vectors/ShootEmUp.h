//#pragma once
#ifndef SHOOTEMUP_H
#define SHOOTEMUP_H

#include "olcPixelGameEngine.h"
//#include "Player.h"

constexpr float PI = 3.14159265359f;
constexpr float TWO_PI = PI * 2.0f;

enum class Upgrade : uint32_t
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

//extern std::array<std::pair<std::string, std::string>, (uint32_t)(Upgrade::Total_Upgrades)> arrUpgradeNameDesc;
extern std::array<std::tuple<std::string, std::string, uint32_t>, (uint32_t)(Upgrade::Total_Upgrades)> arrUpgradeNameDesc;


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

	Bullet(const Bullet& b);
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



//void GenerateRandCards(std::vector<std::pair<std::string, std::string>>& vCards, int nCards);
void GenerateRandCards(std::vector<std::tuple<std::string, std::string, uint32_t>>& vCards, int nCards);

bool Between(float n, float min, float max);
olc::Pixel RandColor();
void FadeInPixel(olc::Pixel& p, float speed, float elapsedTime);
bool Between(float n, float min, float max);
void MoveEntity(Entity& e, float elapsedTime);
float absmag(const olc::vf2d& v1, const olc::vf2d& v2);
void DrawStars(olc::PixelGameEngine* pge, std::array<olc::vf2d, 1000>& arr, float elapsedTime, bool update = true);



// More concise erase remove idiom
template<class T, typename Lambda>
bool RemoveLambda(std::vector<T>& v, Lambda&& func)
{
	v.erase(std::remove_if(v.begin(), v.end(), func), v.end());
	return false;
}


#endif