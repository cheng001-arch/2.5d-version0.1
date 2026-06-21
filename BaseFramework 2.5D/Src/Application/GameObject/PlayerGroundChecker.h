#pragma once

class PlayerGroundChecker
{
public:
	void SetOwner(KdGameObject* owner) { m_owner = owner; }
	void SetGroundCheckPoint(const Math::Vector3& point) { m_groundCheckPoint = point; }
	void SetGroundCheckRadius(float radius) { m_groundCheckRadius = radius; }

	bool CheckGround();
	bool IsGrounded() const { return m_isGrounded; }
	float GetGroundHeight() const { return m_groundHeight; }
	float GetDistanceToGround() const { return m_distanceToGround; }

private:
	bool m_isGrounded = false;
	Math::Vector3 m_groundCheckPoint = { 0.0f, 0.05f, 0.0f };
	float m_groundCheckRadius = 0.15f;
	float m_groundHeight = 0.0f;
	float m_distanceToGround = FLT_MAX;
	float m_groundMeasureRange = 200.0f;
	KdGameObject* m_owner = nullptr;
};
