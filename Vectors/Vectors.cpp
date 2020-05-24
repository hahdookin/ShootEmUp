#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <string>
#include <vector>
#include <cmath>
#include <array>
#include <algorithm>

constexpr double PI = 3.14159265359;
constexpr double TWO_PI = PI * 2;

template<class T>
struct Vector
{
	T i;
	T j;
	Vector() { this->i = 0.0f; this->j = 0.0f; }
	Vector(T i, T j) : i(i), j(j) {};

	inline double mag() { return sqrt(this->i * this->i + this->j * this->j); }

	inline void print()
	{
		std::cout << this->i << 'i' << (this->j < 0 ? " - " : " + ") << abs(this->j) << 'j' << std::endl;
	}

	inline double angle_r()
	{
		double ang = atan2(this->j, this->i);
		return (ang >= 0.0f ? ang : ang + 2 * PI);
	}

	inline double angle_d()
	{
		double ang = atan2(this->j, this->i) * (180 / PI);
		return (ang >= 0.0f ? ang : ang + 360);
	}

	inline T dot_product(Vector<T> &other)	{ return (this->i * other.i + this->j * other.j); }

	inline Vector operator + (const Vector& rhs) const { return Vector(this->i + rhs.i, this->j + rhs.j); }
	inline Vector operator - (const Vector& rhs) const { return Vector(this->i - rhs.i, this->j - rhs.j); }
	inline Vector operator * (const T& rhs) const { return Vector(this->i * rhs, this->j * rhs); }
	inline Vector operator / (const T& rhs) const { return Vector(this->i / rhs, this->j / rhs); }
	
};

typedef Vector<int> vi;
typedef Vector<float> vf;
typedef Vector<double> vd;

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
	void SetAngleToPlayer(olc::vf2d& pPos)
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
			vEnemies.push_back(Enemy(rand() % ScreenWidth(), rand() % ScreenHeight(), (rand() % 20) + 10.0f, (rand() % 10) + 3));

		return true;
	}

	// More concise erase remove idiom
	template<class T, typename Lambda>
	bool RemoveLambda(std::vector<T>& v, Lambda&& f)
	{
		v.erase(std::remove_if(v.begin(), v.end(), f), v.end());
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

	std::array<olc::vf2d, 1000> arrStars;

	// Starting player pos and velocity and other things
	olc::vf2d pos = { ScreenWidth() / 2, ScreenHeight() / 2 };
	olc::vf2d rotPosA = {};
	olc::vf2d rotPosB = {};
	olc::vf2d rotPosC = {};

	//float fVelocity = 150.0f;
	float fVelocity = 0.0f;
	float fAcceleration = 150.0f;
	float fMaxVelocity = 150.0f;

	float fRotation = 0.0f;
	float fRotationSpeed = 7.0f;
	float lineRay = 20.0f; // Single main ray casted out on circle magnitude
	float fShotDelay = 0.1f;

	std::vector<Bullet> vBullets = {};
	std::vector<Enemy> vEnemies = {};
	std::vector<Particle> vParticles = {};

	bool bSingleMode = true;
	float fAccumulatedTime = 0.0f;

	bool OnUserUpdate(float fElapsedTime) override
	{
#define DEBUGMODE 1
#if DEBUGMODE
		if (GetKey(olc::V).bPressed) fAcceleration += 10;
		if (GetKey(olc::C).bPressed) fRotationSpeed += 10;
		if (GetKey(olc::N).bPressed) fShotDelay /= 2.0f;
		if (GetKey(olc::M).bHeld) vEnemies.push_back(Enemy(0.0f, 0.0f, 40.0f, 10));
#endif

		// Shoot bullets
		if (GetKey(olc::B).bPressed) bSingleMode = !bSingleMode;
		if (bSingleMode) 
		{
			if (GetKey(olc::SPACE).bPressed)
			{
				rotPosC = { pos.x + lineRay * cosf(fRotation), pos.y + lineRay * sinf(fRotation) }; // This is just hacked together
				vBullets.push_back(Bullet(rotPosC.x, rotPosC.y, 200.0f, fRotation));
			}
		} 
		else 
		{
			if (GetKey(olc::SPACE).bHeld)
			{
				fAccumulatedTime += fElapsedTime;
				if (fAccumulatedTime >= fShotDelay)
				{
					rotPosC = { pos.x + lineRay * cosf(fRotation), pos.y + lineRay * sinf(fRotation) }; // This is just hacked together
					vBullets.push_back(Bullet(rotPosC.x, rotPosC.y, 200.0f, fRotation));
					fAccumulatedTime = 0.0f;
				}
				
			}
		}

		// TODO: FIX THIS
		if (GetKey(olc::SHIFT).bHeld) fMaxVelocity = 250.0f;
		else fMaxVelocity = 150.0f;

		// Rotation and Movement Keys
		if (GetKey(olc::A).bHeld) fRotation -= fRotationSpeed * fElapsedTime;
		if (GetKey(olc::D).bHeld) fRotation += fRotationSpeed * fElapsedTime;
		if (GetKey(olc::W).bHeld) 
		{
			if (fVelocity <= fMaxVelocity) fVelocity += fAcceleration * fElapsedTime; // Increase velocity by acceleration
		}
		if (GetKey(olc::S).bHeld)
		{
			pos.y -= fAcceleration/2 * sinf(fRotation) * fElapsedTime;
			pos.x -= fAcceleration/2 * cosf(fRotation) * fElapsedTime;
			//if (fVelocity >= -fMaxVelocity) fVelocity -= fAcceleration * fElapsedTime;
		}
		if (fVelocity > 0.0f) 
		{
			pos.y += fVelocity * sinf(fRotation) * fElapsedTime;
			pos.x += fVelocity * cosf(fRotation) * fElapsedTime;
		}
		// Start decellerating if not pressing forward 
	    // TODO: Fix decceleration
		if(!GetKey(olc::W).bHeld && fVelocity != 0) fVelocity = (fVelocity - fAcceleration*fElapsedTime > 0) ? fVelocity - fAcceleration*fElapsedTime : 0.0f;
		
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
			arrStars[i].y += ((i < 300) ? 7.0f : 10.0f) * fElapsedTime;
			if (arrStars[i].y > ScreenHeight()) arrStars[i] = { (float)(rand() % ScreenWidth()), 0.0f };
			Draw(arrStars[i], ((i < 300) ? olc::VERY_DARK_GREY : olc::WHITE));
		}

		// Draw bullets shot out
		if (vBullets.size() > 0) RemoveLambda(vBullets, [this](Bullet& b) { return IsOffScreen(b.pos); });
		for (auto& bullet : vBullets)
		{
			// Update bullet's posistion and draw it
			bullet.pos.x += bullet.shotSpeed * cosf(bullet.angle) * fElapsedTime;
			bullet.pos.y += bullet.shotSpeed * sinf(bullet.angle) * fElapsedTime;
			DrawCircle(bullet.pos, bullet.radius);
			
			// Check if bullet has come into contact with an enemy
			for (auto& enemy : vEnemies)
			{
				if (Between(bullet.pos.x, enemy.pos.x - enemy.radius, enemy.pos.x + enemy.radius) && 
					Between(bullet.pos.y, enemy.pos.y - enemy.radius, enemy.pos.y + enemy.radius))
				{
					bullet.pos.x = ScreenWidth() + 1;
					bullet.pos.y = ScreenHeight() + 1;
					enemy.ReduceHP(1);
					enemy.isHit = true;
				}
			}
		} 
		
		// Draw enemies on screen and check if enemy is dead
		for (auto& enemy : vEnemies)
		{
			if (enemy.healthPoints == 0) 
			{ 
				// Explosion
				for (int i = 0; i < 1000; i++) // 1000 paricles o.O
					vParticles.push_back(Particle(enemy.pos.x, enemy.pos.y));
			    continue; 
			}

			enemy.SetAngleToPlayer(pos);
			enemy.pos.x += enemy.speed * cosf(enemy.angle) * fElapsedTime;
			enemy.pos.y += enemy.speed * sinf(enemy.angle) * fElapsedTime;
			FillCircle(enemy.pos, enemy.radius, (enemy.isHit ? olc::YELLOW : olc::RED));
			DrawCircle(enemy.pos, enemy.radius, RandColor());
			if (enemy.isHit) for (int i = 0; i < 10; i++) vParticles.push_back(Particle(enemy.pos.x, enemy.pos.y));
			if (enemy.isHit) enemy.isHit = false;
		}
		if (vEnemies.size() > 0) RemoveLambda(vEnemies, [](Enemy& e) { return e.healthPoints == 0; }); // Has to come after because drawing explosion

		// Draw particles and remove 
		if (vParticles.size() > 0) RemoveLambda(vParticles, [this](Particle& p) { return IsOffScreen(p.pos); });
		for (auto& p : vParticles)
		{
			p.pos.x += p.speed * cosf(p.angle) * fElapsedTime;
			p.pos.y += p.speed * sinf(p.angle) * fElapsedTime;
			Draw(p.pos);
		}
		

		// Update points of triangle ship and draw new position of player /\ 
		rotPosA = { pos.x - lineRay * cosf(fRotation - PI/6), pos.y - lineRay * sinf(fRotation - PI/6) };
		rotPosB = { pos.x - lineRay * cosf(fRotation + PI/6), pos.y - lineRay * sinf(fRotation + PI/6) };
		rotPosC = { pos.x + lineRay * cosf(fRotation), pos.y + lineRay * sinf(fRotation) }; 

		DrawLine(rotPosC, rotPosA, olc::RED);
		DrawLine(rotPosC, rotPosB, olc::RED);


		//if (vEnemies.size() == 0) FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::Pixel(0, 0, 0, 255));

#if DEBUGMODE
		DrawString(5, 5, "x: " + std::to_string(pos.x));
		DrawString(5, 15, "y: " + std::to_string(pos.y));
		DrawString(5, 25, "a: " + std::to_string(fRotation));
		DrawString(5, 35, "v: " + std::to_string(fVelocity));
		std::string msg = bSingleMode ? "Single" : "Burst";
		DrawString(5, 45, "b: " + msg );
		DrawString(5, 55, "Bn: " + std::to_string(vBullets.size()));
		DrawString(5, 65, "En: " + std::to_string(vEnemies.size()));
		DrawString(5, 75, "Pn: " + std::to_string(vParticles.size()));
		DrawString(5, 85, "da: " + std::to_string(fRotationSpeed));
#endif

		return true;
	}
};




int main()
{
	Rays demo;
	//if (demo.Construct(256, 240, 4, 4))
	if (demo.Construct(640, 480, 2, 2))
		demo.Start();

	return 0;
}