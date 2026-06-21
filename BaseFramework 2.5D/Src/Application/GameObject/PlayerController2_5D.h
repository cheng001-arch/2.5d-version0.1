#pragma once

#include "PlayerGroundChecker.h"
#include "PlayerCarrySlot.h"
#include "PlayerInteractor.h"
#include "BallThrowController.h"

class MapArea;

class PlayerController2_5D : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void DrawUnLit() override;

	void MoveLeftRight();
	void Jump();
	void Crouch();
	void ApplyGravity();
	void LockZToCurrentArea();
	void UpdateAnimation();
	void ResolveHorizontalCollisions();

	void SetCurrentArea(const std::shared_ptr<MapArea>& area) { m_currentArea = area; }
	std::shared_ptr<MapArea> GetCurrentArea() const { return m_currentArea.lock(); }

	void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
	void SetJumpForce(float force) { m_jumpForce = force; }
	void SetGravity(float gravity) { m_gravity = gravity; }
	float GetFacingDirection() const { return m_facingDirection; }
	float GetDistanceToGround() const { return m_groundChecker.GetDistanceToGround(); }
	void ApplyTeleport(const Math::Vector3& position, float facingDirection);
	void SetControlEnabled(bool enabled) { m_controlEnabled = enabled; }
	bool IsControlEnabled() const { return m_controlEnabled; }

	PlayerCarrySlot& WorkCarrySlot() { return m_carrySlot; }
	PlayerInteractor& WorkInteractor() { return m_interactor; }
	PlayerGroundChecker& WorkGroundChecker() { return m_groundChecker; }

private:
	enum class AnimationState
	{
		Idle,
		Walk,
		Jump,
		Fall,
	};

	float m_moveSpeed = 5.0f;
	float m_jumpForce = 7.0f;
	float m_gravity = 18.0f;
	float m_facingDirection = 1.0f;
	float m_verticalVelocity = 0.0f;
	bool m_isCrouching = false;
	std::weak_ptr<MapArea> m_currentArea;
	PlayerGroundChecker m_groundChecker;
	PlayerCarrySlot m_carrySlot;
	PlayerInteractor m_interactor;
	BallThrowController m_ballThrowController;
	std::shared_ptr<KdSquarePolygon> m_idleSprite;
	std::shared_ptr<KdSquarePolygon> m_walkSprite;
	std::shared_ptr<KdSquarePolygon> m_jumpSprite;
	float m_animationTimer = 0.0f;
	int m_animationFrame = 0;
	float m_moveInput = 0.0f;
	AnimationState m_animationState = AnimationState::Idle;
	float m_collisionRadius = 0.4f;
	bool m_controlEnabled = true;
};
