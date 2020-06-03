#include "ShootEmUp.h"
#include "olcPixelGameEngine.h"

// Base struct which most inherit from

// -> Entity
float Entity::GetAngleToEntity(const olc::vf2d& entityPos)
{
	return atan2f(entityPos.y - this->pos.y, entityPos.x - this->pos.x);
}
void Entity::SetAngleToEntity(const olc::vf2d& entityPos)
{
	this->angle = Entity::GetAngleToEntity(entityPos);
}
void Entity::Move(float elapsedTime)
{
	pos.x += speed * cosf(angle) * elapsedTime;
	pos.y += speed * sinf(angle) * elapsedTime;
}

// -> Bullet
Bullet::Bullet(olc::vf2d pos, float speed, float angle, int radius, int damage)
	: Bullet::Bullet(pos.x, pos.y, speed, angle, radius, damage) {}

Bullet::Bullet(const Bullet& b)
	: Bullet::Bullet(b.pos.x, b.pos.y, b.speed, b.angle, b.radius, b.damage) 
{
	std::cout << "Bullet copied!\n";
}

Bullet::Bullet(float x, float y, float speed, float angle, int radius, int damage)
{
	this->pos.x = x;
	this->pos.y = y;
	this->speed = speed;
	this->angle = angle;
	this->radius = radius;
	this->damage = damage;
}
void Bullet::DrawYourself(olc::PixelGameEngine* pge, bool friendly)
{
	pge->DrawCircle(pos, radius, (friendly ? olc::WHITE : olc::RED));
}

// -> Enemy
void Enemy::ReduceHP(int points)
{
	this->healthPoints = (this->healthPoints - points > 0) ? this->healthPoints - points : 0;
}
bool Enemy::WillFire()  
{
	return false;
}
void Enemy::DrawYourself(olc::PixelGameEngine* pge)
{
	pge->FillCircle(pos, radius, (isHit ? olc::YELLOW : color));
	pge->DrawCircle(pos, radius, RandColor());
}

// -> Shooting Enemy
ShootingEnemy::ShootingEnemy(olc::vf2d pos, float speed, int hp, olc::vf2d offset)
	: ShootingEnemy::ShootingEnemy(pos.x, pos.y, speed, hp, offset) {};

ShootingEnemy::ShootingEnemy(float x, float y, float speed, int hp, olc::vf2d offset)
{
	this->pos.x = x + offset.x;
	this->pos.y = y + offset.y;
	this->speed = speed;
	this->healthPoints = hp;
	this->radius = 20.0f;
	this->color = olc::DARK_GREEN;
};
bool ShootingEnemy::WillFire()
{
	return rand() % 500 == 1;
}

// -> Brute Enemy
BruteEnemy::BruteEnemy(olc::vf2d pos, float speed, int hp, olc::vf2d offset)
	: BruteEnemy::BruteEnemy(pos.x, pos.y, speed, hp, offset) {}

BruteEnemy::BruteEnemy(float x, float y, float speed, int hp, olc::vf2d offset)
{
	this->pos.x = x + offset.x;
	this->pos.y = y + offset.y;
	this->speed = speed;
	this->healthPoints = hp;
	this->radius = 20.0f;
	this->color = olc::DARK_RED;
}

// -> Particle
Particle::Particle(olc::vf2d pos)
	: Particle::Particle(pos.x, pos.y) {};
Particle::Particle(float x, float y)
{
	this->pos.x = x;
	this->pos.y = y;
	this->speed = rand() % 200 + 30.0f;
	this->angle = (rand() % 360) * (PI / 180);
}
void Particle::DrawYourself(olc::PixelGameEngine* pge)
{
	pge->Draw(pos);
}


/* Player Class Implementation */
Player::Player(olc::vf2d _pos)
	: Player::Player(_pos.x, _pos.y) {};

Player::Player(float _x, float _y)
{
	m_pos.x = _x;
	m_pos.y = _y;
}
void Player::Move(float elapsedTime)
{
	m_pos.y += m_fVelocity * sinf(m_fRotation) * elapsedTime;
	m_pos.x += m_fVelocity * cosf(m_fRotation) * elapsedTime;
}
void Player::UpdateRotPositions()
{
	m_rotPosRight = {
		m_pos.x - m_lineRay * cosf(m_fRotation - m_fInnerShipAngle),
		m_pos.y - m_lineRay * sinf(m_fRotation - m_fInnerShipAngle)
	};
	m_rotPosLeft = {
		m_pos.x - m_lineRay * cosf(m_fRotation + m_fInnerShipAngle),
		m_pos.y - m_lineRay * sinf(m_fRotation + m_fInnerShipAngle)
	};
	m_rotPosNose = {
		m_pos.x + m_lineRay * cosf(m_fRotation),
		m_pos.y + m_lineRay * sinf(m_fRotation)
	};
}
void Player::DrawShip(olc::PixelGameEngine* pge)
{
	pge->DrawLine(m_rotPosNose, m_rotPosLeft);
	pge->DrawLine(m_rotPosLeft, m_pos);
	pge->DrawLine(m_pos, m_rotPosRight);
	pge->DrawLine(m_rotPosRight, m_rotPosNose);
}

// Getters and setters
const olc::vf2d& Player::GetPos()
{
	return m_pos;
}
void Player::SetPos(olc::vf2d pos)
{
	Player::SetPos(pos.x, pos.y);
}
void Player::SetPos(float x, float y)
{
	m_pos.x = x;
	m_pos.y = y;
}
float Player::GetPosX()
{
	return m_pos.x;
}
float Player::GetPosY()
{
	return m_pos.y;
}
void Player::SetPosX(float x)
{
	m_pos.x = x;
}
void Player::SetPosY(float y)
{
	m_pos.y = y;
}
const olc::vf2d& Player::GetPosNose()
{
	return m_rotPosNose;
}
float Player::GetVelocity()
{
	return m_fVelocity;
}
void Player::SetVelocity(float velocity)
{
	m_fVelocity = velocity;
}
float Player::GetMaxVelocity()
{
	return m_fMaxVelocity;
}
void Player::SetMaxVelocity(float velocity)
{
	m_fMaxVelocity = velocity;
}
float Player::GetAcceleration()
{
	return m_fAcceleration;
}
void Player::SetAcceleration(float acceleration)
{
	m_fAcceleration = acceleration;
}
float Player::GetRotation()
{
	return m_fRotation;
}
void Player::SetRotation(float angle)
{
	m_fRotation = angle;
}
float Player::GetRotAcceleration()
{
	return m_fRotAcceleration;
}
void Player::SetRotAcceleration(float acceleration)
{
	m_fRotAcceleration = acceleration;
}
float Player::GetRotVelocity()
{
	return m_fRotVelocity;
}
void Player::SetRotVelocity(float velocity)
{
	m_fRotVelocity = velocity;
}
float Player::GetRotMaxVelocity()
{
	return m_fRotMaxVelocity;
}
void Player::SetRotMaxVelocity(float velocity)
{
	m_fRotMaxVelocity = velocity;
}
float Player::GetLineRay()
{
	return m_lineRay;
}
void Player::SetLineRay(float length)
{
	m_lineRay = length;
}
float Player::GetInnerShipAngle()
{
	return m_fInnerShipAngle;
}
void Player::SetInnerShipAngle(float angle)
{
	m_fInnerShipAngle = angle;
}
float Player::GetShotDelay()
{
	return m_fShotDelay;
}
void Player::SetShotDelay(float delay)
{
	m_fShotDelay = delay;
}
float Player::GetShotSpeed()
{
	return m_fShotSpeed;
}
void Player::SetShotSpeed(float speed)
{
	m_fShotSpeed = speed;
}
/* End of player class implementation */


void DrawStars(olc::PixelGameEngine* pge, std::array<olc::vf2d, 1000>& arr, float elapsedTime, bool update)
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

float absmag(const olc::vf2d& v1, const olc::vf2d& v2)
{
	// v1's x is larger than v2's x 
	// and v1's y is larger than v2's y
	if (v1.x - v2.x > 0 && v1.y - v2.y > 0)
		return std::sqrtf((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));

	// only v1's x is larger
	if (v1.x - v2.x > 0)
		return std::sqrtf((v1.x - v2.x) * (v1.x - v2.x) + (v2.y - v1.y) * (v2.y - v1.y));

	// only v1's y is larger
	if (v1.y - v2.y > 0)
		return std::sqrtf((v2.x - v1.x) * (v2.x - v1.x) + (v1.y - v2.y) * (v1.y - v2.y));

	return std::sqrtf((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y));
}

// Keep .first to 13 chars and .second to 16 chars
//std::array<std::pair<std::string, std::string>, (uint32_t)(Upgrade::Total_Upgrades)> arrUpgradeNameDesc =
//{
//	std::make_pair("Double Shot", "Twice the fun"),
//	std::make_pair("Triple Shot", "More guns!"),
//	std::make_pair("Quad Shot", "Lil' overkill"),
//	std::make_pair("Homing Shot", "Find your path"),
//	std::make_pair("UV Shot", "Piercing shots"),
//	std::make_pair("Shrink Ray", "Make em' Small!"),
//	std::make_pair("Extra Crew", "Fire rate UP"),
//	std::make_pair("Cannonballs", "Bigger shot size"),
//	std::make_pair("Extra Card", "More options!"),
//	std::make_pair("Carbon Fins", "Fly faster!"),
//	std::make_pair("FTL Thrusters", "Boost faster!"),
//	std::make_pair("Renovate", "Bigger ship!"),
//	std::make_pair("Keto Diet", "Weight loss!")
//
//};
//
//void GenerateRandCards(std::vector<std::pair<std::string, std::string>>& vCards, int nCards)
//{
//	if ((int)Upgrade::Total_Upgrades < nCards)
//		nCards = (int)Upgrade::Total_Upgrades;
//
//	vCards.clear();
//
//	int n = rand() % (int)Upgrade::Total_Upgrades;
//	std::vector<int> found = {};
//
//	for (int i = 0; i < nCards; i++)
//	{
//		while (std::find(found.begin(), found.end(), n) != found.end())
//		{
//			n = rand() % (int)Upgrade::Total_Upgrades;
//		}
//		found.push_back(n);
//		
//		vCards.push_back(arrUpgradeNameDesc[n]);
//	}
//}

std::array<std::tuple<std::string, std::string, uint32_t>, (uint32_t)(Upgrade::Total_Upgrades)> arrUpgradeNameDesc =
{
	std::make_tuple("Double Shot", "Twice the fun",0),
	std::make_tuple("Triple Shot", "More guns!",1),
	std::make_tuple("Quad Shot", "Lil' overkill",2),
	std::make_tuple("Homing Shot", "Find your path",3),
	std::make_tuple("UV Shot", "Piercing shots",4),
	std::make_tuple("Shrink Ray", "Make em' Small!",5),
	std::make_tuple("Extra Crew", "Fire rate UP",6),
	std::make_tuple("Cannonballs", "Bigger shot size",7),
	std::make_tuple("Extra Card", "More options!",8),
	std::make_tuple("Carbon Fins", "Fly faster!",9),
	std::make_tuple("FTL Thrusters", "Boost faster!",10),
	std::make_tuple("Renovate", "Bigger ship!",11),
	std::make_tuple("Keto Diet", "Weight loss!",12)

};

void GenerateRandCards(std::vector<std::tuple<std::string, std::string, uint32_t>>& vCards, int nCards)
{
	if ((uint32_t)Upgrade::Total_Upgrades < nCards)
		nCards = (uint32_t)Upgrade::Total_Upgrades;

	vCards.clear();

	int n = rand() % (uint32_t)Upgrade::Total_Upgrades;
	std::vector<int> found = {};

	for (int i = 0; i < nCards; i++)
	{
		while (std::find(found.begin(), found.end(), n) != found.end())
		{
			n = rand() % (uint32_t)Upgrade::Total_Upgrades;
		}
		found.push_back(n);

		vCards.push_back(arrUpgradeNameDesc[n]);
	}
}

//----------------------------

void ApplyUpgrade(uint32_t upgrade, Player& player)
{
	switch (upgrade)
	{
		case 0://Upgrade::Double_Shot:
		{

		}
		break;

		case 1://Upgrade::Triple_Shot:
		{

		}
		break;
		
		case 2:
		{

		}
		break;
	}
}