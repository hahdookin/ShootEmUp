#include "olcPixelGameEngine.h"
#include "Player.h"
#include "ShootEmUp.h"


/* Player Class Implementation */

// Constructors
Player::Player(olc::vf2d _pos)
	: Player::Player(_pos.x, _pos.y) {};

Player::Player(float _x, float _y)
{
	m_pos.x = _x;
	m_pos.y = _y;
	m_vBulletsPerShot.push_back(Bullet(m_rotPosNose, m_fShotSpeed, m_fRotation, 4));
}

// Update player position
void Player::Move(float elapsedTime)
{
	m_pos.y += m_fVelocity * sinf(m_fRotation) * elapsedTime;
	m_pos.x += m_fVelocity * cosf(m_fRotation) * elapsedTime;
}
// Updates the points of the ship on rotation
void Player::UpdateRotPositions()
{
	m_rotPosRight = 
	{
		m_pos.x - m_lineRay * cosf(m_fRotation - m_fInnerShipAngle),
		m_pos.y - m_lineRay * sinf(m_fRotation - m_fInnerShipAngle)
	};
	m_rotPosLeft = 
	{
		m_pos.x - m_lineRay * cosf(m_fRotation + m_fInnerShipAngle),
		m_pos.y - m_lineRay * sinf(m_fRotation + m_fInnerShipAngle)
	};
	m_rotPosNose = 
	{
		m_pos.x + m_lineRay * cosf(m_fRotation),
		m_pos.y + m_lineRay * sinf(m_fRotation)
	};
}
// Draw the lines between points of the ship
void Player::DrawShip(olc::PixelGameEngine* pge)
{
	pge->DrawLine(m_rotPosNose, m_rotPosLeft);
	pge->DrawLine(m_rotPosLeft, m_pos);
	pge->DrawLine(m_pos, m_rotPosRight);
	pge->DrawLine(m_rotPosRight, m_rotPosNose);
}

// THE ISSUE: bullets are being created in two vectors:
// one in the game class and one in this class, 
// however, we are not updating the position of 
// the bullets in this class. 
// SOLUTION: completely remove the bullets vector
// in the game loop, and handle updating the position
// in this class. Include getters and setters for 
// the m_vBullets vector. (maybe)
// MORE: Provide working move constructor on bullet class

void Player::AddBullet(Bullet& b)
{
	m_vBulletsPerShot.push_back(b);
	//m_vBulletsPerShot.push_back(Bullet(m_rotPosNose, m_fShotSpeed, m_fRotation + PI/18));
	//m_vBulletsPerShot.push_back(Bullet(m_rotPosNose, m_fShotSpeed, m_fRotation + PI / 18));
	//vBullets.push_back(std::make_unique<Bullet>(rotPosNose, player.GetShotSpeed(), player.GetRotation() + PI / 18, 4));
	//vBullets.push_back(std::make_unique<Bullet>(rotPosNose, player.GetShotSpeed(), player.GetRotation() - PI / 18, 4));

}
void Player::CreateBullets()
{
	for (auto& b : m_vBulletsPerShot)
	{
		b.pos = m_rotPosNose;
		b.angle = m_fRotation;
		b.speed = m_fShotSpeed;
		m_vBullets.push_back(b);
	}
}

// TODO THIS (may not need this after all)
void Player::Shoot(std::vector<Bullet>& gameLoopBullets)
{
	for (auto& bullet : m_vBullets)
	{
		gameLoopBullets.push_back(bullet);
	}
}
void Player::EraseBullets()
{
	m_vBullets.clear();
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

void Player::ApplyUpgrade(uint32_t upgrade)
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

		case 6: // Fire rate up
		{
			Player::SetShotDelay(Player::GetShotDelay() / 2.0f);
		}
		break;

		case 9: // Fly faster;
		{
			Player::SetMaxVelocity(Player::GetVelocity() * 1.2f);
		}
		break;

		case 11: // Grow
		{
			Player::SetLineRay(Player::GetLineRay() * 1.2f);
		}
		break;

		case 12: // Shrink
		{
			Player::SetLineRay(Player::GetLineRay() * 0.8f);
		}
		break;
	}
}