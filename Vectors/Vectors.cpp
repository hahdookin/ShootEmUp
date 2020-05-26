#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <string>
#include <vector>
//#include <cmath>
#include <array>
#include <algorithm>

constexpr double PI = 3.14159265359;
constexpr double TWO_PI = PI * 2;

struct Bullet
{
	olc::vf2d pos;
	int radius;
	float shotSpeed;
	float angle;
	Bullet(olc::vf2d pos, float shotSpeed, float angle, int radius = 2)
	{
		this->pos = pos;
		this->shotSpeed = shotSpeed;
		this->angle = angle;
		this->radius = radius;
	};
	Bullet(float x, float y, float shotSpeed, float angle, int radius = 2)
	{
		this->pos.x = x;
		this->pos.y = y;
		this->shotSpeed = shotSpeed;
		this->angle = angle;
		this->radius = radius;
	};
};

struct Enemy
{
	olc::vf2d pos;
	olc::vf2d offset;
	float speed;
	float angle;
	float radius;
	int healthPoints;
	bool isHit = false;
	Enemy(olc::vf2d pos, float speed, int hp, olc::vf2d offset = { 0.0f, 0.0f })
	{
		this->pos = pos;
		this->offset = offset;
		this->speed = speed;
		this->healthPoints = hp;
		this->radius = 20.0f;
	};
	Enemy(float x, float y, float speed, int hp, olc::vf2d offset = { 0.0f, 0.0f })
	{
		this->pos.x = x;
		this->pos.y = y;
		this->offset = offset;
		this->speed = speed;
		this->healthPoints = hp;
		this->radius = 20.0f;
	};
	void SetAngleToEntity(olc::vf2d& pPos)
	{
		this->angle = atan2f(pPos.y - this->pos.y, pPos.x - this->pos.x);
	}
	void ReduceHP(int points)
	{
		this->healthPoints = (this->healthPoints - points > 0) ? this->healthPoints - points : 0;
	}
};

struct Particle 
{
	olc::vf2d pos;
	float speed;
	float angle;
	Particle(olc::vf2d pos)
	{
		this->pos = pos;
		this->speed = rand() % 200 + 30.0f;
		this->angle = (rand() % 360) * (PI/180);
	}
	Particle(float x, float y)
	{
		this->pos.x = x;
		this->pos.y = y;
		this->speed = rand() % 200 + 30.0f;
		this->angle = (rand() % 360) * (PI / 180);
	}
};

class Rays : public olc::PixelGameEngine
{
public:
	Rays()
	{
		sAppName = "Rays";
	}

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here

		// Populating stars
		for (auto& s : arrStars)
			s = { (float)(rand() % ScreenWidth()), (float)(rand() % ScreenHeight()) };

		// Creating and placing enemies initially
		for (int i = 0; i < 10; i++)
			//vEnemies.push_back(std::make_unique<Enemy>(rand() % ScreenWidth(), rand() % ScreenHeight(), (rand() % 20) + 10.0f,  3));

		return true;
	}

	// More concise erase remove idiom
	template<class T, typename Lambda>
	bool RemoveLambda(std::vector<T>& v, Lambda&& func)
	{
		v.erase(std::remove_if(v.begin(), v.end(), func), v.end());
		return false;
	}

	olc::Pixel RandColor()
	{
		return olc::Pixel(rand() % 256, rand() % 256, rand() % 256);
	}

	bool Between(float n, float min, float max)
	{
		return (n >= min && n <= max);
	}

	bool IsOffScreen(olc::vf2d& pos)
	{
		return (pos.x > ScreenWidth() || pos.x < 0 || pos.y > ScreenHeight() || pos.y < 0);
	}
	
	olc::vf2d CenterTextPosistion(size_t size, uint32_t scale = 1)
	{
		return olc::vf2d((ScreenWidth() / 2.0f) - (size * 8 * scale) / 2.0f, (ScreenHeight() / 2.0f) - 8.0f * scale);
	}

	std::array<olc::vf2d, 1000> arrStars;

	// Starting player pos and velocity and other things
	olc::vf2d pos = { ScreenWidth() / 2, ScreenHeight() / 2 };
	olc::vf2d rotPosRight = {};
	olc::vf2d rotPosLeft = {};
	olc::vf2d rotPosNose = {};

	//float fVelocity = 150.0f;
	float fVelocity = 0.0f;
	float fAcceleration = 150.0f;
	float fMaxVelocity = 150.0f;
	float fMaxVelocityMod = 250.0f;

	float fRotation = 0.0f;
	float fRotationSpeed = 7.0f;
	float lineRay = 15.0f; // Magnitude of triangle edges
	float fShotDelay = 0.1f;

	std::vector<std::unique_ptr<Bullet>> vBullets = {};
	std::vector<std::unique_ptr<Enemy>> vEnemies = {};
	std::vector<std::unique_ptr<Particle>> vParticles = {};

	bool bSingleMode = true;
	bool bPowerUp = false;
	float fAccumulatedTime = 0.0f;

	std::string lc = "Level Complete";
	olc::Pixel whiteFadeIn = olc::PixelF(1.0f, 1.0f, 1.0f, 0.0f);

	bool OnUserUpdate(float fElapsedTime) override
	{
#define DEBUGMODE 1
#if DEBUGMODE
		if (GetKey(olc::V).bPressed) fAcceleration += 10;
		if (GetKey(olc::C).bPressed) fRotationSpeed += 10;
		if (GetKey(olc::N).bPressed) fShotDelay /= 2.0f;
		if (GetKey(olc::M).bHeld) vEnemies.push_back(std::make_unique<Enemy>(0.0f, 0.0f, (rand() % 100) + 10, 10));
#endif
		
		
		// Speed modifier
		if (GetKey(olc::SHIFT).bHeld) fMaxVelocity = 250.0f;
		else fMaxVelocity = 150.0f;

		// Rotation and movement Keys
		if (GetKey(olc::A).bHeld) fRotation -= fRotationSpeed * fElapsedTime;
		if (GetKey(olc::D).bHeld) fRotation += fRotationSpeed * fElapsedTime;
		if (GetKey(olc::W).bHeld) 
		{
			if (fVelocity >= fMaxVelocity) fVelocity -= fAcceleration * fElapsedTime; // Decrease smoothly after speed mod
			else if (fVelocity <= fMaxVelocity) fVelocity += fAcceleration * fElapsedTime; // Increase velocity by acceleration
		}
		if (GetKey(olc::S).bHeld)
		{
			if (fVelocity <= -fMaxVelocity / 2.0f) fVelocity += fAcceleration * fElapsedTime;
			else if (fVelocity >= -fMaxVelocity / 2.0f) fVelocity -= fAcceleration * fElapsedTime;
		}
		
		// Move player
		if (fVelocity != 0)
		{
			pos.y += fVelocity * sinf(fRotation) * fElapsedTime;
			pos.x += fVelocity * cosf(fRotation) * fElapsedTime;
		}

		// Start decellerating
		if (!GetKey(olc::W).bHeld && fVelocity > 0) 
			fVelocity = (fVelocity - fAcceleration*fElapsedTime > 0) ? fVelocity - fAcceleration*fElapsedTime : 0.0f;
		
		if (!GetKey(olc::S).bHeld && fVelocity < 0)
			fVelocity = (fVelocity + fAcceleration * fElapsedTime < 0) ? fVelocity + fAcceleration * fElapsedTime : 0.0f;

		// Shoot bullets
		if (GetKey(olc::B).bPressed) bSingleMode = !bSingleMode;
		if (GetKey(olc::G).bPressed) bPowerUp = !bPowerUp;
		if (bSingleMode)
		{
			if (GetKey(olc::SPACE).bPressed)
			{
				rotPosNose = { pos.x + lineRay * cosf(fRotation), pos.y + lineRay * sinf(fRotation) }; // This is just hacked together
				vBullets.push_back(std::make_unique<Bullet>(rotPosNose.x, rotPosNose.y, 300.0f, fRotation));
			}
		}
		else
		{
			if (GetKey(olc::SPACE).bHeld)
			{
				fAccumulatedTime += fElapsedTime;
				if (fAccumulatedTime >= fShotDelay)
				{
					rotPosNose = { pos.x + lineRay * cosf(fRotation), pos.y + lineRay * sinf(fRotation) }; // This is just hacked together
					vBullets.push_back(std::make_unique<Bullet>(rotPosNose.x, rotPosNose.y, 300.0f, fRotation));
					if (bPowerUp) {
						vBullets.push_back(std::make_unique<Bullet>(rotPosNose, 300.0f, fRotation + PI / 18));
						vBullets.push_back(std::make_unique<Bullet>(rotPosNose, 300.0f, fRotation - PI / 18));
					}
					fAccumulatedTime = 0.0f;
				}

			}
		}


		// Keep rotation within 2 PI
		while (fRotation < 0) fRotation += TWO_PI;
		while (fRotation > TWO_PI) fRotation -= TWO_PI;

		// Screen Boundaries
		if (pos.x >= ScreenWidth()) pos.x = ScreenWidth();
		if (pos.x <= 0) pos.x = 0;
		if (pos.y >= ScreenHeight()) pos.y = ScreenHeight();
		if (pos.y <= 0) pos.y = 0;




		// Display
		Clear(olc::BLACK);

		// Drawing stars
		for (int i = 0; i < arrStars.size(); i++)
		{
			//arrStars[i].y += ((i < 300) ? 7.0f : 10.0f) * fElapsedTime;
			arrStars[i].y += ((i < 150) ? 7.0f : (i < 500) ? 8.5f : 10.0f) * fElapsedTime;
			if (arrStars[i].y > ScreenHeight()) arrStars[i] = { (float)(rand() % ScreenWidth()), 0.0f };
			//Draw(arrStars[i], ((i < 300) ? olc::VERY_DARK_GREY : olc::WHITE));
			Draw(arrStars[i], ((i < 150) ? olc::VERY_DARK_GREY : (i < 500) ? olc::DARK_GREY : olc::WHITE));
		}

		// Draw bullets shot out
		if (vBullets.size() > 0) RemoveLambda(vBullets, [this](std::unique_ptr<Bullet>& b) { return IsOffScreen(b->pos); });
		for (auto& bullet : vBullets)
		{
			// Update bullet's posistion and draw it
			bullet->pos.x += bullet->shotSpeed * cosf(bullet->angle) * fElapsedTime;
			bullet->pos.y += bullet->shotSpeed * sinf(bullet->angle) * fElapsedTime;
			DrawCircle(bullet->pos, bullet->radius);
			
			// Check if bullet has come into contact with an enemy
			for (auto& enemy : vEnemies)
			{
				if (Between(bullet->pos.x, enemy->pos.x - enemy->radius, enemy->pos.x + enemy->radius) &&
					Between(bullet->pos.y, enemy->pos.y - enemy->radius, enemy->pos.y + enemy->radius))
				{
					bullet->pos.x = ScreenWidth() + 1;
					bullet->pos.y = ScreenHeight() + 1;
					enemy->ReduceHP(1);
					enemy->isHit = true;
				}
			}
		} 

		// Draw enemies on screen and check if enemy is dead
		for (auto& enemy : vEnemies)
		{
			if (enemy->healthPoints == 0)
			{ 
				// Explosion
				for (int i = 0; i < 100; i++) // 1000 paricles o.O
					vParticles.push_back(std::make_unique<Particle>(enemy->pos.x, enemy->pos.y));
			    continue; 
			}

			enemy->SetAngleToEntity(pos);
			enemy->pos.x += enemy->speed * cosf(enemy->angle) * fElapsedTime;
			enemy->pos.y += enemy->speed * sinf(enemy->angle) * fElapsedTime;
			FillCircle(enemy->pos, enemy->radius, (enemy->isHit ? olc::YELLOW : olc::RED));
			DrawCircle(enemy->pos, enemy->radius, RandColor());
			if (enemy->isHit) for (int i = 0; i < 10; i++) vParticles.push_back(std::make_unique<Particle>(enemy->pos.x, enemy->pos.y));
			if (enemy->isHit) enemy->isHit = false;
		}
		if (vEnemies.size() > 0) RemoveLambda(vEnemies, [](std::unique_ptr<Enemy>& e) { return e->healthPoints == 0; }); // Has to come after because drawing explosion

		// Draw particles and remove 
		if (vParticles.size() > 0) RemoveLambda(vParticles, [this](std::unique_ptr<Particle>& p) { return IsOffScreen(p->pos); });
		for (auto& p : vParticles)
		{
			p->pos.x += p->speed * cosf(p->angle) * fElapsedTime;
			p->pos.y += p->speed * sinf(p->angle) * fElapsedTime;
			Draw(p->pos);
		}
		

		// Update points of triangle ship and draw new position of player
		rotPosRight = { pos.x - lineRay * cosf(fRotation - PI/6), pos.y - lineRay * sinf(fRotation - PI/6) };
		rotPosLeft = { pos.x - lineRay * cosf(fRotation + PI/6), pos.y - lineRay * sinf(fRotation + PI/6) };
		rotPosNose = { pos.x + lineRay * cosf(fRotation), pos.y + lineRay * sinf(fRotation) }; 

		DrawLine(rotPosNose, rotPosLeft);
		DrawLine(rotPosLeft, pos);
		DrawLine(pos, rotPosRight);
		DrawLine(rotPosRight, rotPosNose);

		// Check if player has won
		if (vEnemies.size() == 0)
		{
			// Fade in 'Level Complete' text
			if (whiteFadeIn.a < 255.0f)
				whiteFadeIn.a = (whiteFadeIn.a + 200.0f * fElapsedTime < 255) ? whiteFadeIn.a + 200.0f * fElapsedTime : 255;
			DrawString(CenterTextPosistion(lc.size(), 2), lc, whiteFadeIn, 2);
		}
		else whiteFadeIn.a = 0;

#if DEBUGMODE
		DrawString(5,  5, "x: " + std::to_string(pos.x));
		DrawString(5, 15, "y: " + std::to_string(pos.y));
		DrawString(5, 25, "a: " + std::to_string(fRotation));
		DrawString(5, 35, "v: " + std::to_string(fVelocity));
		std::string msg = bSingleMode ? "Single" : "Burst";
		DrawString(5, 45, "b: " + msg);
		DrawString(5, 55, "Bn: " + std::to_string(vBullets.size()));
		DrawString(5, 65, "En: " + std::to_string(vEnemies.size()));
		DrawString(5, 75, "Pn: " + std::to_string(vParticles.size()));
		DrawString(5, 85, "da: " + std::to_string(fRotationSpeed));
		DrawString(5, 95, "Wa: " + std::to_string(whiteFadeIn.a));
#endif

		return true;
	}
};



int main()
{
	Rays game;
	if (game.Construct(640, 480, 2, 2))
		game.Start();

	return 0;
}