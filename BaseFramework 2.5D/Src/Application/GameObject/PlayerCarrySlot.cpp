#include "PlayerCarrySlot.h"
#include "ColorBall.h"

bool PlayerCarrySlot::HasBall() const
{
	return m_currentBall != nullptr;
}

void PlayerCarrySlot::PickUpBall(const std::shared_ptr<ColorBall>& ball)
{
	if (!ball) { return; }
	if (HasBall()) { ReplaceBall(ball); return; }
	m_currentBall = ball;
	m_currentBall->OnPickedUp();
}

void PlayerCarrySlot::ReplaceBall(const std::shared_ptr<ColorBall>& newBall)
{
	DropCurrentBall();
	PickUpBall(newBall);
}

void PlayerCarrySlot::DropCurrentBall()
{
	if (!m_currentBall) { return; }
	const Math::Vector3 dropPosition =
		m_lastPlayerPosition + Math::Vector3(-m_lastFacingDirection * 0.9f, 0.633f, 0.0f);
	m_currentBall->DropAt(dropPosition);
	m_currentBall = nullptr;
}

void PlayerCarrySlot::ThrowCurrentBall(
	const Math::Vector3& direction, float speed, float maxDistance)
{
	if (!m_currentBall) { return; }
	m_currentBall->OnThrown(direction, speed, maxDistance);
	m_currentBall = nullptr;
}

std::shared_ptr<ColorBall> PlayerCarrySlot::ReleaseCurrentBall()
{
	auto ball = m_currentBall;
	m_currentBall = nullptr;
	return ball;
}

void PlayerCarrySlot::UpdateCarryPosition(
	const Math::Vector3& playerPosition, float facingDirection)
{
	m_lastPlayerPosition = playerPosition;
	m_lastFacingDirection = facingDirection;
	if (!m_currentBall) { return; }
	Math::Vector3 offset = m_carryPoint;
	offset.x *= facingDirection;
	m_currentBall->SetHeldPosition(playerPosition + offset);
}

void PlayerCarrySlot::SnapCarryPosition(
	const Math::Vector3& playerPosition, float facingDirection)
{
	m_lastPlayerPosition = playerPosition;
	m_lastFacingDirection = facingDirection;
	if (!m_currentBall) { return; }

	Math::Vector3 offset = m_carryPoint;
	offset.x *= facingDirection;
	m_currentBall->SnapHeldPosition(playerPosition + offset);
}
