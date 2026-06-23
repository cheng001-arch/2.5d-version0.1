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
	bool TryStartLedgeClimb();
	void UpdateLedgeClimb();

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
		Run,
		Jump,
		Fall,
		LedgeClimb,
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
	std::shared_ptr<KdSquarePolygon> m_runSprite;
	std::shared_ptr<KdSquarePolygon> m_jumpSprite;
	std::shared_ptr<KdSquarePolygon> m_ledgeClimbSprite;
	float m_animationTimer = 0.0f;
	int m_animationFrame = 0;
	float m_moveInput = 0.0f;
	float m_runSpeedMultiplier = 2.0f;
	float m_doubleTapWindow = 0.28f;
	float m_leftTapTimer = 0.0f;
	float m_rightTapTimer = 0.0f;
	float m_runningDirection = 0.0f;
	bool m_wasLeftHeld = false;
	bool m_wasRightHeld = false;
	bool m_isRunning = false;
	bool m_isLedgeClimbing = false;
	Math::Vector3 m_ledgeClimbStart = Math::Vector3::Zero;
	Math::Vector3 m_ledgeClimbTarget = Math::Vector3::Zero;
	float m_ledgeClimbElapsed = 0.0f;
	float m_ledgeClimbFrameDuration = 0.1f;
	float m_ledgeMinHeight = 0.15f;
	float m_ledgeMaxHeight = 1.75f;
	float m_ledgeWallCheckDistance = 1.15f;
	float m_ledgeInputBufferDuration = 0.30f;
	float m_ledgeInputBufferTimer = 0.0f;
	AnimationState m_animationState = AnimationState::Idle;
	float m_collisionRadius = 0.4f;
	bool m_controlEnabled = true;
};
