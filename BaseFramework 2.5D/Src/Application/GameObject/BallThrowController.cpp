#include "BallThrowController.h"
#include "ColorBall.h"
#include "PlayerCarrySlot.h"
#include "../main.h"

void BallThrowController::UpdateInput(
	PlayerCarrySlot& carrySlot, float facingDirection)
{
	const bool held = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
	const bool pressed = held && !m_wasThrowButtonHeld;
	const bool released = !held && m_wasThrowButtonHeld;

	if (pressed && carrySlot.HasBall()) { BeginCharge(); }

	if (m_isCharging && held)
	{
		m_chargeTime = std::min(
			m_chargeTime + Application::Instance().GetDeltaSeconds(),
			m_maxChargeTime);
	}

	if (m_isCharging && released)
	{
		ReleaseThrow(carrySlot, facingDirection);
	}

	m_wasThrowButtonHeld = held;
}

void BallThrowController::BeginCharge()
{
	m_isCharging = true;
	m_chargeTime = 0.0f;
}

void BallThrowController::ReleaseThrow(
	PlayerCarrySlot& carrySlot, float facingDirection)
{
	auto ball = carrySlot.ReleaseCurrentBall();
	if (!ball)
	{
		m_isCharging = false;
		m_chargeTime = 0.0f;
		return;
	}

	const float charge = GetChargeRatio();
	const float horizontalSpeed =
		m_minHorizontalSpeed + (m_maxHorizontalSpeed - m_minHorizontalSpeed) * charge;
	float verticalSpeed = 0.0f;
	if (m_chargeTime <= m_tapChargeThreshold)
	{
		// Projectile apex height is proportional to velocity squared.
		// Multiplying by sqrt(0.5) makes a quick tap reach half the old height.
		verticalSpeed = m_minVerticalSpeed * std::sqrt(0.5f);
	}
	else
	{
		const float heldCharge = std::clamp(
			(m_chargeTime - m_tapChargeThreshold) /
			(m_maxChargeTime - m_tapChargeThreshold),
			0.0f,
			1.0f);
		verticalSpeed =
			m_minVerticalSpeed +
			(m_maxVerticalSpeed - m_minVerticalSpeed) * heldCharge;
	}
	const float maxDistance =
		m_minThrowDistance + (m_maxThrowDistance - m_minThrowDistance) * charge;

	const Math::Vector3 launchVelocity(
		facingDirection < 0.0f ? -horizontalSpeed : horizontalSpeed,
		verticalSpeed,
		0.0f);

	m_flyingBall = ball;
	ball->OnThrownArc(launchVelocity, m_ballGravity, maxDistance);
	m_isCharging = false;
	m_chargeTime = 0.0f;
}

void BallThrowController::MoveFlyingBall()
{
	auto ball = m_flyingBall.lock();
	if (!ball || !ball->IsFlying()) { m_flyingBall.reset(); }
}

bool BallThrowController::DetectHit()
{
	auto ball = m_flyingBall.lock();
	return ball && !ball->IsFlying();
}

float BallThrowController::GetChargeRatio() const
{
	if (m_maxChargeTime <= 0.0f) { return 1.0f; }
	return std::clamp(m_chargeTime / m_maxChargeTime, 0.0f, 1.0f);
}
