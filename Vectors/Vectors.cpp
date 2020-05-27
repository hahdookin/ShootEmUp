#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <string>
#include <vector>
#include <array>
#include <algorithm>

// Todo:
// - Make a function to fade in a pixel
//    - Fix all issues that need to use this function
// - Fix rotation acceleration if I even want that
// - Add Help Screen

constexpr float PI = 3.14159265359f;
constexpr float TWO_PI = PI * 2.0f;

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

		nCurState = STATE::MAIN_SCREEN;

		// Populating stars
		for (auto& s : arrStars)
			s = { (float)(rand() % ScreenWidth()), (float)(rand() % ScreenHeight()) };

		// Creating and placing enemies initially
		for (int i = 0; i < 5; i++)
			vEnemies.push_back(std::make_unique<Enemy>(rand() % ScreenWidth(), rand() % ScreenHeight(), (rand() % 20) + 10.0f,  2));

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
	
	olc::vf2d CenterTextPosistion(size_t size, uint32_t scale = 1, olc::vf2d offset = { 0.0f, 0.0f })
	{
		return (olc::vf2d((ScreenWidth() / 2.0f) - (size * 8 * scale) / 2.0f, (ScreenHeight() / 2.0f) - 8.0f * scale) + offset);
	}

	void FadeInPixel(olc::Pixel& p, float speed, float elapsedTime)
	{
		if (p.a == 255) return;
		if (p.a < 255) p.a += ceil(speed * elapsedTime);
		else p.a = 255;
	}

	std::array<olc::vf2d, 1000> arrStars;

	// Starting player pos and velocity and other things
	olc::vf2d pos = { ScreenWidth() / 2, ScreenHeight() / 2 };
	olc::vf2d rotPosRight = {};
	olc::vf2d rotPosLeft = {};
	olc::vf2d rotPosNose = {};

	float fVelocity = 0.0f;
	float fAcceleration = 150.0f;
	float fMaxVelocity = 150.0f;

	float fRotation = 0.0f;
	float fRotAcceleration = 45.0f;
	float fRotVelocity = 0.0f;
	float fRotMaxVelocity = 7.0f;

	float lineRay = 15.0f; // Magnitude of triangle edges
	float fShotDelay = 0.1f;

	std::vector<std::unique_ptr<Bullet>> vBullets = {};
	std::vector<std::unique_ptr<Enemy>> vEnemies = {};
	std::vector<std::unique_ptr<Particle>> vParticles = {};

	bool bSingleMode = true;
	bool bPowerUp = false;
	float fAccumulatedTime = 0.0f;
	int nSelected = 0;

	
	olc::Pixel whiteFadeIn = olc::PixelF(1.0f, 1.0f, 1.0f, 0.0f);
	olc::Pixel whiteFadeIn2 = olc::PixelF(1.0f, 1.0f, 1.0f, 0.0f);

	// Game states
	enum class STATE {
		MAIN_SCREEN, LEVEL, LEVEL_COMPLETE, PAUSE
	} nCurState;

	std::string gameName[4] = { "Shoot 'em UP!", "Play", "Help", "Exit" };

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Handling game states
		switch (nCurState)
		{
			case STATE::MAIN_SCREEN:
			{
				Clear(olc::BLACK);

				// Drawing stars
				for (int i = 0; i < arrStars.size(); i++)
				{
					arrStars[i].y += ((i < 150) ? 7.0f : (i < 500) ? 8.5f : 10.0f) * fElapsedTime;
					if (arrStars[i].y > ScreenHeight()) arrStars[i] = { (float)(rand() % ScreenWidth()), 0.0f };
					Draw(arrStars[i], ((i < 150) ? olc::VERY_DARK_GREY : (i < 500) ? olc::DARK_GREY : olc::WHITE));
				}

				// This WHOLE THING is a hack !!! edit: IDK i kinda like it
				olc::vf2d vTitlePos = CenterTextPosistion(gameName[0].size(), 3U, { 0.0f, -75.0f });
				olc::vf2d vPlayPos = CenterTextPosistion(gameName[1].size(), 2U, { 0.0f, 0.0f });
				olc::vf2d vHelpPos = CenterTextPosistion(gameName[2].size(), 2U, { 0.0f, 50.0f });
				olc::vf2d vExitPos = CenterTextPosistion(gameName[3].size(), 2U, { 0.0f, 100.0f });

				if (GetKey(olc::W).bPressed) nSelected--;
				if (GetKey(olc::S).bPressed) nSelected++;

				if (nSelected < 0) nSelected = 2;
				if (nSelected > 2) nSelected = 0;
				
				if (GetKey(olc::ENTER).bPressed || GetKey(olc::SPACE).bPressed)
				{
					if (nSelected == 0) nCurState = STATE::LEVEL;
					if (nSelected == 1) {} // Help Screen
					if (nSelected == 2) return false; // Exit game
				}

				DrawString(vTitlePos, gameName[0], olc::WHITE, 3U);
				DrawString(vPlayPos, gameName[1], (nSelected == 0 ? olc::RED : olc::WHITE), 2U);
				DrawString(vHelpPos, gameName[2], (nSelected == 1 ? olc::RED : olc::WHITE), 2U);
				DrawString(vExitPos, gameName[3], (nSelected == 2 ? olc::RED : olc::WHITE), 2U);

				return true; // for now
			}
			break;
			case STATE::LEVEL:
			{
				// Level stuff
				// Could Just Squeeze all level mechanics into here
			}
			break;
			case STATE::LEVEL_COMPLETE:
			{
				// Maybe show something for next level
				// Possibly level statistics etc
			}
			break;
			case STATE::PAUSE:
			{
				// Display
				Clear(olc::BLACK);

				// Drawing stars
				for (int i = 0; i < arrStars.size(); i++)
					Draw(arrStars[i], ((i < 150) ? olc::VERY_DARK_GREY : (i < 500) ? olc::DARK_GREY : olc::WHITE));

				// Draw bullets shot out
				for (auto& bullet : vBullets)
					DrawCircle(bullet->pos, bullet->radius);

				// Draw enemies on screen
				for (auto& enemy : vEnemies)
				{
					FillCircle(enemy->pos, enemy->radius, (enemy->isHit ? olc::YELLOW : olc::RED));
					DrawCircle(enemy->pos, enemy->radius, RandColor());
				}

				// Draw particles and remove 
				for (auto& p : vParticles)
					Draw(p->pos);


				// Draw player's ship
				DrawLine(rotPosNose, rotPosLeft);
				DrawLine(rotPosLeft, pos);
				DrawLine(pos, rotPosRight);
				DrawLine(rotPosRight, rotPosNose);

				std::string ps = "Pause";
				DrawString(CenterTextPosistion(ps.size(), 2U), ps, olc::WHITE, 2U);
				if (GetKey(olc::ESCAPE).bPressed) nCurState = STATE::LEVEL;
				return true;
			}
			break;
		}

#define DEBUGMODE 1
#if DEBUGMODE
		if (GetKey(olc::V).bPressed) fAcceleration += 10;
		if (GetKey(olc::C).bPressed) fRotVelocity += 10;
		if (GetKey(olc::N).bPressed) fShotDelay /= 2.0f;
		if (GetKey(olc::M).bHeld) vEnemies.push_back(std::make_unique<Enemy>(0.0f, 0.0f, (rand() % 100) + 10, 10));
#endif
		if (GetKey(olc::ESCAPE).bPressed) { nCurState = STATE::PAUSE; return true; }
		
		// Speed modifier
		if (GetKey(olc::SHIFT).bHeld) fMaxVelocity = 250.0f;
		else fMaxVelocity = 150.0f;

		// Rotation and movement Keys
		//if (GetKey(olc::A).bHeld) fRotation -= fRotVelocity * fElapsedTime;
		//if (GetKey(olc::D).bHeld) fRotation += fRotVelocity * fElapsedTime;
		if (GetKey(olc::A).bHeld)
		{
			if (fRotVelocity >= -fRotMaxVelocity) fRotVelocity -= fRotAcceleration * fElapsedTime;
		}
		if (GetKey(olc::D).bHeld)
		{
			if (fRotVelocity <= fRotMaxVelocity) fRotVelocity += fRotAcceleration * fElapsedTime;
		}
		if (GetKey(olc::W).bHeld) 
		{
			if (fVelocity >= fMaxVelocity) fVelocity -= fAcceleration * fElapsedTime; // Decrease smoothly after speed mod
			else if (fVelocity <= fMaxVelocity) fVelocity += fAcceleration * fElapsedTime; // Increase velocity by acceleration
		}
		if (GetKey(olc::S).bHeld)
		{
			if (fVelocity <= -fMaxVelocity / 2.0f) fVelocity += fAcceleration * fElapsedTime; // ... and vice versa when going backwards.
			else if (fVelocity >= -fMaxVelocity / 2.0f) fVelocity -= fAcceleration * fElapsedTime;
		}
		
		// Update player rotation
		if (fRotVelocity != 0.0f)
		{
			fRotation += fRotVelocity * fElapsedTime;
		}

		// Move player
		if (fVelocity != 0.0f)
		{
			pos.y += fVelocity * sinf(fRotation) * fElapsedTime;
			pos.x += fVelocity * cosf(fRotation) * fElapsedTime;
		}

		// Start decellerating
		if (!GetKey(olc::W).bHeld && fVelocity > 0) 
			fVelocity = (fVelocity - fAcceleration*fElapsedTime > 0) ? fVelocity - fAcceleration*fElapsedTime : 0.0f;
		
		if (!GetKey(olc::S).bHeld && fVelocity < 0)
			fVelocity = (fVelocity + fAcceleration * fElapsedTime < 0) ? fVelocity + fAcceleration * fElapsedTime : 0.0f;

		if (!GetKey(olc::A).bHeld && fRotVelocity < 0)
			fRotVelocity = (fRotVelocity + fRotAcceleration * fElapsedTime < 0) ? fRotVelocity + fRotAcceleration * fElapsedTime : 0.0f;

		if (!GetKey(olc::D).bHeld && fRotVelocity > 0)
			fRotVelocity = (fRotVelocity - fRotAcceleration * fElapsedTime > 0) ? fRotVelocity - fRotAcceleration * fElapsedTime : 0.0f;

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
			arrStars[i].y += ((i < 150) ? 7.0f : (i < 500) ? 8.5f : 10.0f) * fElapsedTime;
			if (arrStars[i].y > ScreenHeight()) arrStars[i] = { (float)(rand() % ScreenWidth()), 0.0f };
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
			std::string lc = "Level Complete";
			FadeInPixel(whiteFadeIn, 100.0f, fElapsedTime);
			DrawString(CenterTextPosistion(lc.size(), 2), lc, whiteFadeIn, 2U);
			if (whiteFadeIn.a == 255)
			{
				std::string s = "Press Enter to continue...";
				FadeInPixel(whiteFadeIn2, 100.0f, fElapsedTime);
				DrawString(CenterTextPosistion(s.size(), 2, { 0.0f, 50.0f }), s, whiteFadeIn2, 2U);

				if (GetKey(olc::ENTER).bPressed) nCurState = STATE::MAIN_SCREEN; // STATE_LEVEL_COMPLETE ?
			};
			
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
		DrawString(5, 85, "da: " + std::to_string(fRotVelocity));
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