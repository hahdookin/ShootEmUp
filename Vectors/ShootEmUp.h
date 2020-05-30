#pragma once
#include "olcPixelGameEngine.h"

constexpr float PI = 3.14159265359f;
constexpr float TWO_PI = PI * 2.0f;

// Declarations
struct Entity;
struct Bullet;
struct Enemy;
struct ShootingEnemy;
struct BruteEnemy;
struct Particle;

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
} nUpgrade;

void GenerateRandCards(std::vector<std::pair<std::string, std::string>>& vCards, int nCards);
bool Between(float n, float min, float max);
olc::Pixel RandColor();
void FadeInPixel(olc::Pixel& p, float speed, float elapsedTime);
bool Between(float n, float min, float max);
void MoveEntity(Entity& e, float elapsedTime);
float absmag(olc::vf2d& v1, olc::vf2d& v2);
template<class T, typename Lambda>
bool RemoveLambda(std::vector<T>& v, Lambda&& func);






// Base struct which most inherit from
struct Entity
{
	olc::vf2d pos;
	float speed;
	float angle;
	virtual void SetAngleToEntity(olc::vf2d& pPos)
	{
		this->angle = atan2f(pPos.y - this->pos.y, pPos.x - this->pos.x);
	}
};

struct Bullet : public Entity
{
	int radius;
	int damage;

	Bullet(olc::vf2d pos, float speed, float angle, int radius = 2, int damage = 1)
		: Bullet(pos.x, pos.y, speed, angle, radius, damage) {};

	Bullet(float x, float y, float speed, float angle, int radius = 2, int damage = 1)
	{
		this->pos.x = x;
		this->pos.y = y;
		this->speed = speed;
		this->angle = angle;
		this->radius = radius;
		this->damage = damage;
	};
	
};

struct Enemy : public Entity
{
	float radius;
	int healthPoints;
	bool isHit = false;
	olc::Pixel color;

	Enemy(olc::vf2d pos, float speed, int hp, olc::vf2d offset = { 0.0f, 0.0f })
		: Enemy(pos.x, pos.y, speed, hp, offset) {};

	Enemy(float x, float y, float speed, int hp, olc::vf2d offset = { 0.0f, 0.0f })
	{
		this->pos.x = x + offset.x;
		this->pos.y = y + offset.y;
		this->speed = speed;
		this->healthPoints = hp;
		this->radius = 20.0f;
	};
	void ReduceHP(int points)
	{
		this->healthPoints = (this->healthPoints - points > 0) ? this->healthPoints - points : 0;
	}
};

struct ShootingEnemy : public Enemy
{
	using Enemy::Enemy;
	olc::Pixel color = olc::DARK_GREEN;
	bool WillFire()
	{
		return rand() % 500 == 1;
	}
};

struct BruteEnemy : public Enemy
{

};

struct Particle : public Entity
{
	Particle(olc::vf2d pos)
	{
		this->pos = pos;
		this->speed = rand() % 200 + 30.0f;
		this->angle = (rand() % 360) * (PI / 180);
	}
	Particle(float x, float y)
	{
		this->pos.x = x;
		this->pos.y = y;
		this->speed = rand() % 200 + 30.0f;
		this->angle = (rand() % 360) * (PI / 180);
	}
};

void DrawStars(olc::PixelGameEngine* pge, std::array<olc::vf2d, 1000>& arr, float elapsedTime, bool update = true)
{
	for (int i = 0; i < arr.size(); i++)
	{
		if (update)
		{
			arr[i].y += ((i < 150) ? 7.0f : (i < 500) ? 8.5f : 10.0f) * elapsedTime;
			if (arr[i].y > pge->ScreenHeight()) arr[i] = { (float)(rand() % pge->ScreenWidth()), 0.0f };
		}
		pge->Draw(arr[i], ((i < 150) ? olc::VERY_DARK_GREY : (i < 500) ? olc::DARK_GREY : olc::WHITE));
	}
}

olc::Pixel RandColor()
{
	return olc::Pixel(rand() % 256, rand() % 256, rand() % 256);
}

void FadeInPixel(olc::Pixel& p, float speed, float elapsedTime)
{
	if (p.a == 255) return;
	if (p.a < 255) p.a += ceil(speed * elapsedTime);
	else p.a = 255;
}

bool Between(float n, float min, float max)
{
	return (n >= min && n <= max);
}

void MoveEntity(Entity& e, float elapsedTime)
{
	e.pos.x += e.speed * cosf(e.angle) * elapsedTime;
	e.pos.y += e.speed * sinf(e.angle) * elapsedTime;
}

// More concise erase remove idiom
template<class T, typename Lambda>
bool RemoveLambda(std::vector<T>& v, Lambda&& func)
{
	v.erase(std::remove_if(v.begin(), v.end(), func), v.end());
	return false;
}

/*float absmagX(olc::vf2d& v1, olc::vf2d& v2)
{
	return sqrt((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y));
}*/

float absmag(olc::vf2d& v1, olc::vf2d& v2)
{
	if (v1.x - v2.x > 0 && v1.y - v2.y > 0)
		return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
	if (v1.x - v2.x > 0)
		return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v2.y - v1.y) * (v2.y - v1.y));
	if (v1.y - v2.y > 0)
		return sqrt((v2.x - v1.x) * (v2.x - v1.x) + (v1.y - v2.y) * (v1.y - v2.y));
	return sqrt((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y));
}

std::array<std::pair<std::string, std::string>, (size_t)(Upgrade::Total_Upgrades)> arrUpgradeNameDesc =
{
	std::make_pair("Double Shot", "Twice the fun"),
	std::make_pair("Triple Shot", "More guns!"),
	std::make_pair("Quad Shot", "Lil' overkill"),
	std::make_pair("Homing Shot", "Find your path"),
	std::make_pair("UV Shot", "Piercing shots"),
	std::make_pair("Shrink Ray", "Make em' Small!"),
	std::make_pair("Extra Crew", "Fire rate UP"),
	std::make_pair("Cannonballs", "Bigger shot size"),
	std::make_pair("Extra Card", "More options!"),
	std::make_pair("Carbon Fins", "Fly faster!"),
	std::make_pair("FTL Thrusters", "Boost faster!"),
	std::make_pair("Renovate", "Bigger ship!"),
	std::make_pair("Keto Diet", "Weight loss!")
	
};

void GenerateRandCards(std::vector<std::pair<std::string, std::string>>& vCards, int nCards)
{
	vCards.clear();
	for (int i = 0; i < nCards; i++)
	{
		vCards.push_back(arrUpgradeNameDesc[rand() % (size_t)Upgrade::Total_Upgrades]);
	}
}



