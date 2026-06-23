#pragma once

class ColorBall;
class PlayerCarrySlot;

class BallThrowController
{
public:
	void UpdateInput(PlayerCarrySlot& carrySlot, float facingDirection);
	void BeginCharge();
	void ReleaseThrow(PlayerCarrySlot& carrySlot, float facingDirection);
	void MoveFlyingBall();
	bool DetectHit();
	bool IsCharging() const { return m_isCharging; }
	float GetChargeRatio() const;

private:
	bool m_isCharging = false;
	bool m_wasThrowButtonHeld = false;
	float m_chargeTime = 0.0f;
	float m_maxChargeTime = 3.0f;
	float m_tapChargeThreshold = 0.18f;
	float m_minHorizontalSpeed = 7.0f;
	float m_maxHorizontalSpeed = 31.18f;
	float m_minVerticalSpeed = 4.5f;
	float m_maxVerticalSpeed = 17.32f;
	float m_minThrowDistance = 8.0f;
	float m_maxThrowDistance = 66.0f;
	float m_ballGravity = 18.0f;
	std::weak_ptr<ColorBall> m_flyingBall;
};
