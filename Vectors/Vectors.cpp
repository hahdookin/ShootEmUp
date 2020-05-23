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
	Particle(float x, float y)
	{
		this->pos.x = x;
		this->pos.y = y;
		this->speed = rand() % 100 + 30.0f;
		this->angle = (rand() % 360) * (PI/180);
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
		{
			Enemy e = Enemy(rand() % ScreenWidth(), rand() % ScreenHeight(), (rand() % 20) + 10.0f, (rand() % 10) + 3);
			vEnemies.push_back(e);
		}

		return true;
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
	olc::vf2d rotPos = {};
	float fVelocity = 150.0f;
	float fRotation = 0.0f;
	float fRotationSpeed = 7.0f;
	float fFOV = 90.0f;
	float lineRay = 20.0f; // Single main ray casted out on circle magnitude
	float fShotDelay = 0.1f;

	std::vector<Bullet> vBullets = {};
	std::vector<Enemy> vEnemies = {};
	std::vector<Particle> vParticles = {};

	bool bSingleMode = true;
	float fAccumulatedTime = 0.0f;

	bool bBulletsCanBeCleared = true;
	bool bParticlesCanBeCleared = true;

	bool OnUserUpdate(float fElapsedTime) override
	{
#define DEBUGMODE 1
#if DEBUGMODE
		if (GetKey(olc::V).bPressed) fVelocity += 10;
		if (GetKey(olc::B).bPressed) bSingleMode = !bSingleMode;
		if (GetKey(olc::C).bPressed) fRotationSpeed += 10;
		if (GetKey(olc::N).bPressed) fShotDelay /= 2;
#endif
		// Shoot bullets
		if (bSingleMode) 
		{
			if (GetKey(olc::SPACE).bPressed)
			{
				Bullet bullet = Bullet(pos.x, pos.y, 200.0f, fRotation);
				vBullets.push_back(bullet);
			}
		} 
		else 
		{
			if (GetKey(olc::SPACE).bHeld)
			{
				fAccumulatedTime += fElapsedTime;
				if (fAccumulatedTime >= fShotDelay)
				{
					Bullet bullet = Bullet(pos.x, pos.y, 200.0f, fRotation);
					vBullets.push_back(bullet);
					fAccumulatedTime = 0.0f;
				}
				
			}
		}

		// Rotation and Movement Keys
		if (GetKey(olc::A).bHeld) fRotation -= fRotationSpeed * fElapsedTime;
		if (GetKey(olc::D).bHeld) fRotation += fRotationSpeed * fElapsedTime;
		if (GetKey(olc::W).bHeld) 
		{
			pos.y += fVelocity * sinf(fRotation) * fElapsedTime;
			pos.x += fVelocity * cosf(fRotation) * fElapsedTime;
		}
		if (GetKey(olc::S).bHeld)
		{
			pos.y -= fVelocity * sinf(fRotation) * fElapsedTime;
			pos.x -= fVelocity * cosf(fRotation) * fElapsedTime;
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
			arrStars[i].y += ((i < 300) ? 7.0f : 10.0f) * fElapsedTime;
			if (arrStars[i].y > ScreenHeight()) arrStars[i] = { (float)(rand() % ScreenWidth()), 0.0f };
			Draw(arrStars[i], ((i < 100) ? olc::VERY_DARK_GREY : olc::WHITE));
		}

		// Draw bullets shot out
		if (vBullets.size() > 0)
			vBullets.erase(std::remove_if(vBullets.begin(), vBullets.end(), [this](Bullet& b) { return IsOffScreen(b.pos); }), vBullets.end());
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
				{
					Particle p = Particle(enemy.pos.x, enemy.pos.y);
					vParticles.push_back(p);
				}
			    continue; 
			}

			enemy.SetAngleToPlayer(pos);
			enemy.pos.x += enemy.speed * cosf(enemy.angle) * fElapsedTime;
			enemy.pos.y += enemy.speed * sinf(enemy.angle) * fElapsedTime;
			FillCircle(enemy.pos, enemy.radius, (enemy.isHit ? olc::YELLOW : olc::RED));
			DrawCircle(enemy.pos, enemy.radius, RandColor());
			if (enemy.isHit) enemy.isHit = false;
		}
		if (vEnemies.size() > 0)
			vEnemies.erase(std::remove_if(vEnemies.begin(), vEnemies.end(), [](Enemy& e){ return e.healthPoints == 0; }), vEnemies.end());



		// Draw particles
		for (auto& p : vParticles)
		{
			if (IsOffScreen(p.pos)) continue;

			bParticlesCanBeCleared = false;
			p.pos.x += p.speed * cosf(p.angle) * fElapsedTime;
			p.pos.y += p.speed * sinf(p.angle) * fElapsedTime;
			Draw(p.pos);
		}
		if (vParticles.size() > 0)
			vParticles.erase(std::remove_if(vParticles.begin(), vParticles.end(), [this](Particle& p) { return IsOffScreen(p.pos); }), vParticles.end());

		// Draw new position of player
		rotPos = { pos.x + lineRay * cosf(fRotation), pos.y + lineRay * sinf(fRotation) };
		DrawCircle(pos, 10);
		DrawLine(pos, rotPos, olc::RED);
		

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