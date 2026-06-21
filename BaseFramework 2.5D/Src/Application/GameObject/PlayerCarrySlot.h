#pragma once

class ColorBall;

class PlayerCarrySlot
{
public:
	bool HasBall() const;
	void PickUpBall(const std::shared_ptr<ColorBall>& ball);
	void ReplaceBall(const std::shared_ptr<ColorBall>& newBall);
	void DropCurrentBall();
	void ThrowCurrentBall(const Math::Vector3& direction, float speed, float maxDistance);
	void UpdateCarryPosition(const Math::Vector3& playerPosition, float facingDirection);
	std::shared_ptr<ColorBall> ReleaseCurrentBall();

	void SetCarryPoint(const Math::Vector3& point) { m_carryPoint = point; }
	std::shared_ptr<ColorBall> GetCurrentBall() const { return m_currentBall; }

private:
	std::shared_ptr<ColorBall> m_currentBall;
	Math::Vector3 m_carryPoint = { 0.0f, 2.6f, 0.0f };
	Math::Vector3 m_lastPlayerPosition = Math::Vector3::Zero;
	float m_lastFacingDirection = 1.0f;
};
