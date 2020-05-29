#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "ShootEmUp.h"
#include <string>
#include <vector>
#include <array>
#include <algorithm>


// Todo:
// - Make a function to fade in a pixel
//    - Fix all issues that need to use this function
// - Homing bullets still a little janky
// - Add lists of upgrades
//    - Homing, Larger shots, more damage, reduced tear delay, piercing, more upgrade options, etc


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

		nCurState = State::Level_Complete;

		// Populating stars
		for (auto& s : arrStars)
			s = { (float)(rand() % ScreenWidth()), (float)(rand() % ScreenHeight()) };

		// Creating and placing enemies initially
		for (int i = 0; i < 5; i++)
			vEnemies.push_back(std::make_unique<Enemy>(rand() % ScreenWidth(), rand() % ScreenHeight(), (rand() % 20) + 10.0f, 2));

		// Populate cards info
		GenerateRandCards(vpssCards, nCards);

		return true;
	}

	

	bool IsOffScreen(olc::vf2d& pos)
	{
		return (pos.x > ScreenWidth() || pos.x < 0 || pos.y > ScreenHeight() || pos.y < 0);
	}

	olc::vf2d CenterTextPosistion(size_t size, uint32_t scale = 1, olc::vf2d offset = { 0.0f, 0.0f })
	{
		return (olc::vf2d((ScreenWidth() / 2.0f) - (size * 8 * scale) / 2.0f, (ScreenHeight() / 2.0f) - 8.0f * scale) + offset);
	}

	olc::vf2d CenterTextCard(std::string& s, uint32_t dx, uint32_t cards, uint32_t scale = 1U, olc::vf2d offset = { 0.0f, 0.0f })
	{
		return (olc::vf2d(dx + (.5f * ScreenWidth() / cards) - (scale * 4U * s.size()), 1.5f * ScreenHeight()/3 - 4U * scale) + offset);
	}

	std::array<olc::vf2d, 1000> arrStars;

	// Starting player pos and velocity and other things
	olc::vf2d pos = { ScreenWidth() / 2, ScreenHeight() / 2 };
	olc::vf2d rotPosRight = {};
	olc::vf2d rotPosLeft = {};
	olc::vf2d rotPosNose = {};

	std::vector<Upgrade> vUpgrades = {};

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
	bool bHomingShots = false;
	float fAccumulatedTime = 0.0f;
	int nSelected = 0;

	// Fix these
	olc::Pixel whiteFadeIn = olc::PixelF(1.0f, 1.0f, 1.0f, 0.0f);
	olc::Pixel whiteFadeIn2 = olc::PixelF(1.0f, 1.0f, 1.0f, 0.0f);

	// Game states
	enum class State 
	{
		Main_Screen, 
		Help_Screen, 
		Level, 
		Level_Complete, 
		Pause
	} nCurState;

	std::vector<std::string> arrMainScreen = 
	{ 
		"Shoot 'em UP!", 
		"PLAY", 
		"HELP", 
		"EXIT" 
	};
	std::vector<std::string> arrHelpScreen = 
	{ 
		"W A S D - Move and turn", 
		"SHIFT - Hold to increase speed", 
		"SPACE - Fire", 
		"B - Toggle Auto Firing",
		"G - Toggle Power Up"
	};

	// Determine closest enemy to bullet
	olc::vf2d closestEnemyPos = { 0.0f, 0.0f };

	// Card variables
	int nCards = 2;
	std::vector<std::pair<std::string, std::string>> vpssCards = {};



#define DEBUGMODE 1
	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);

		// Update position only if not in Pause state
		DrawStars(this, arrStars, fElapsedTime, (nCurState != State::Pause));


		// Handling game states
		switch (nCurState)
		{
			case State::Main_Screen:
			{

				// This WHOLE THING is a hack !!! edit: IDK i kinda like it
				olc::vf2d vTitlePos = CenterTextPosistion(arrMainScreen[0].size(), 3U, { 0.0f, -75.0f });
				olc::vf2d vPlayPos = CenterTextPosistion(arrMainScreen[1].size(), 2U, { 0.0f, 0.0f });
				olc::vf2d vHelpPos = CenterTextPosistion(arrMainScreen[2].size(), 2U, { 0.0f, 50.0f });
				olc::vf2d vExitPos = CenterTextPosistion(arrMainScreen[3].size(), 2U, { 0.0f, 100.0f });

				if (GetKey(olc::W).bPressed) nSelected--;
				if (GetKey(olc::S).bPressed) nSelected++;

				if (nSelected < 0) nSelected = 2;
				if (nSelected > 2) nSelected = 0;
				
				if (GetKey(olc::ENTER).bPressed || GetKey(olc::SPACE).bPressed)
				{
					if (nSelected == 0) nCurState = State::Level;
					if (nSelected == 1) nCurState = State::Help_Screen;
					if (nSelected == 2) return false; // Exit game
				}

				DrawString(vTitlePos, arrMainScreen[0], olc::WHITE, 3U);
				DrawString(vPlayPos, arrMainScreen[1], (nSelected == 0 ? olc::RED : olc::WHITE), 2U);
				DrawString(vHelpPos, arrMainScreen[2], (nSelected == 1 ? olc::RED : olc::WHITE), 2U);
				DrawString(vExitPos, arrMainScreen[3], (nSelected == 2 ? olc::RED : olc::WHITE), 2U);
				

			}
			break;

			case State::Help_Screen:
			{
				float offset = 10.0f;
				float dy = (float)ScreenHeight() / (float)arrHelpScreen.size();
				for (size_t i = 0; i < arrHelpScreen.size(); i++)
				{
					DrawString(0, offset, arrHelpScreen[i], olc::WHITE, 2U);
					offset += dy;
				}

				if (GetKey(olc::SPACE).bPressed || GetKey(olc::ENTER).bPressed || GetKey(olc::ESCAPE).bPressed)
					nCurState = State::Main_Screen;

			}
			break;

			case State::Level:
			{
				// Level stuff

			#if DEBUGMODE
				if (GetKey(olc::V).bPressed) fAcceleration += 10.0f;
				if (GetKey(olc::C).bPressed) fRotVelocity += 10.0f;
				if (GetKey(olc::X).bPressed) fRotVelocity -= 10.0f;
				if (GetKey(olc::N).bPressed) fShotDelay /= 2.0f;
				if (GetKey(olc::M).bHeld)
				{
					vEnemies.push_back(std::make_unique<Enemy>(rand() % ScreenWidth(), rand() % ScreenHeight(), /*(rand() % 100) +*/ 10, 10));
				}
			#endif

				if (GetKey(olc::ESCAPE).bPressed) { nCurState = State::Pause; return true; }

				// Speed modifier
				if (GetKey(olc::SHIFT).bHeld) fMaxVelocity = 250.0f;
				else fMaxVelocity = 150.0f;

				// Rotation and movement Keys
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
					fVelocity = (fVelocity - fAcceleration * fElapsedTime > 0) ? fVelocity - fAcceleration * fElapsedTime : 0.0f;

				if (!GetKey(olc::S).bHeld && fVelocity < 0)
					fVelocity = (fVelocity + fAcceleration * fElapsedTime < 0) ? fVelocity + fAcceleration * fElapsedTime : 0.0f;

				if (!GetKey(olc::A).bHeld && fRotVelocity < 0)
					fRotVelocity = (fRotVelocity + fRotAcceleration * fElapsedTime < 0) ? fRotVelocity + fRotAcceleration * fElapsedTime : 0.0f;

				if (!GetKey(olc::D).bHeld && fRotVelocity > 0)
					fRotVelocity = (fRotVelocity - fRotAcceleration * fElapsedTime > 0) ? fRotVelocity - fRotAcceleration * fElapsedTime : 0.0f;

				// Shoot bullets
				if (GetKey(olc::B).bPressed) bSingleMode = !bSingleMode;
				if (GetKey(olc::G).bPressed) bPowerUp = !bPowerUp;
				if (GetKey(olc::K).bPressed) bHomingShots = !bHomingShots;
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
							vBullets.push_back(std::make_unique<Bullet>(rotPosNose.x, rotPosNose.y, 300.0f, fRotation, 4));

							if (bPowerUp) 
							{
								vBullets.push_back(std::make_unique<Bullet>(rotPosNose, 300.0f, fRotation + PI / 18, 4));
								vBullets.push_back(std::make_unique<Bullet>(rotPosNose, 300.0f, fRotation - PI / 18, 4));
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

				

				// Draw bullets shot out also update position
				if (vBullets.size() > 0) RemoveLambda(vBullets, [this](std::unique_ptr<Bullet>& b) { return IsOffScreen(b->pos); });
				for (auto& bullet : vBullets)
				{
					
					MoveEntity(*bullet, fElapsedTime);
					DrawCircle(bullet->pos, bullet->radius);

					// Check if bullet has come into contact with an enemy
					for (auto& enemy : vEnemies)
					{
						if (Between(bullet->pos.x, enemy->pos.x - enemy->radius, enemy->pos.x + enemy->radius) &&
							Between(bullet->pos.y, enemy->pos.y - enemy->radius, enemy->pos.y + enemy->radius))
						{
							bullet->pos.x = ScreenWidth() + 1;
							bullet->pos.y = ScreenHeight() + 1;
							enemy->ReduceHP(bullet->damage);
							enemy->isHit = true;
						}

						// Determine closest enemy to home in to
						if (!bHomingShots) continue;
						if (absmag(enemy->pos, bullet->pos) < absmag(closestEnemyPos, bullet->pos))
						{
							closestEnemyPos = enemy->pos;
							if (abs(atan2f(enemy->pos.y - bullet->pos.y, enemy->pos.x - bullet->pos.x)) < PI/2.0f)
								bullet->SetAngleToEntity(enemy->pos);
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
							vParticles.push_back(std::make_unique<Particle>(enemy->pos));
						continue;
					}

					enemy->SetAngleToEntity(pos);
					MoveEntity(*enemy, fElapsedTime);
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
					MoveEntity(*p, fElapsedTime);
					Draw(p->pos);
				}


				// Update points of triangle ship and draw new position of player
				rotPosRight = { pos.x - lineRay * cosf(fRotation - PI / 6), pos.y - lineRay * sinf(fRotation - PI / 6) };
				rotPosLeft = { pos.x - lineRay * cosf(fRotation + PI / 6), pos.y - lineRay * sinf(fRotation + PI / 6) };
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
						std::string s = "Press Enter to continue";
						FadeInPixel(whiteFadeIn2, 100.0f, fElapsedTime);
						DrawString(CenterTextPosistion(s.size(), 2, { 0.0f, 50.0f }), s, whiteFadeIn2, 2U);

						if (GetKey(olc::ENTER).bPressed) 
						{ 
							whiteFadeIn2.a = 0; 
							GenerateRandCards(vpssCards, nCards);
							nCurState = State::Level_Complete; 
						} 
					};

				}
				else whiteFadeIn.a = 0;

			#if DEBUGMODE
				DrawString(5, 5, "x: " + std::to_string(pos.x));
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

			}
			break;

			case State::Level_Complete:
			{
				// Maybe show something for next level
				// Possibly level statistics etc

				std::string su = "SELECT UPGRADE", pe = "PRESS ENTER";
				DrawString(CenterTextPosistion(su.size(), 3U, { 0.0f, (ScreenHeight()/-3.0f) }), su, olc::WHITE, 3U);
				DrawString(CenterTextPosistion(pe.size(), 3U, { 0.0f, (ScreenHeight() /3.0f) + 8.0f }), pe, olc::WHITE, 3U);

				if (GetKey(olc::A).bPressed) nSelected--;
				if (GetKey(olc::D).bPressed) nSelected++;

				if (nSelected < 0) nSelected = nCards - 1;
				if (nSelected > nCards - 1) nSelected = 0;

				if (GetKey(olc::ENTER).bPressed || GetKey(olc::SPACE).bPressed)
				{
					std::cout << "Selected: " << vpssCards[nSelected].first << std::endl;
				}



				
				int offset = 0;

				for (int i = 0; i < nCards; i++)
				{
					if (i == nSelected) FillRect(offset, ScreenHeight() / 3, ScreenWidth() / nCards, ScreenHeight() / 3);
					else DrawRect(offset, ScreenHeight() / 3, ScreenWidth() / nCards, ScreenHeight() / 3);

					DrawString(CenterTextCard(vpssCards[i].first, offset, nCards, 2U, { 0.0f, -10.0f }), vpssCards[i].first, (i == nSelected) ? olc::BLACK : olc::WHITE, 2U);
					DrawString(CenterTextCard(vpssCards[i].second, offset, nCards, 1U, { 0.0f, 20.0f }), vpssCards[i].second, (i == nSelected) ? olc::BLACK : olc::WHITE, 1U);

					offset += ScreenWidth()/nCards;
				}




				if (GetKey(olc::ESCAPE).bPressed) nCurState = State::Main_Screen;
			}
			break;

			case State::Pause:
			{
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

				std::string ps = "PAUSE";
				DrawString(CenterTextPosistion(ps.size(), 2U), ps, olc::WHITE, 2U);

				if (GetKey(olc::ESCAPE).bPressed) nCurState = State::Level;

			}
			break;
		}


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