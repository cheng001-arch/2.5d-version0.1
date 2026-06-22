#include "PlayerController2_5D.h"
#include "ColorBall.h"
#include "PortalDoor.h"
#include "../GameSystem/AreaManager.h"
#include "../GameSystem/GameManager.h"
#include "../GameSystem/MapArea.h"
#include "../Scene/SceneManager.h"
#include "../main.h"

void PlayerController2_5D::Init()
{
	m_groundChecker.SetOwner(this);
	m_interactor.SetOwner(this);
	m_interactor.SetCarrySlot(&m_carrySlot);

	m_idleSprite = std::make_shared<KdSquarePolygon>(
		"Asset/Textures/player/Player Idle.png");
	m_idleSprite->SetSplit(10, 1);
	m_idleSprite->SetUVRect(0);
	m_idleSprite->SetScale({ 1.08f, 1.8f });
	m_idleSprite->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	m_walkSprite = std::make_shared<KdSquarePolygon>(
		"Asset/Textures/player/PlayerWalk.png");
	m_walkSprite->SetSplit(8, 1);
	m_walkSprite->SetUVRect(0);
	// Keep the same world-space size as Idle even though this sheet is 48x48.
	m_walkSprite->SetScale({ 1.08f, 1.8f });
	m_walkSprite->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	m_jumpSprite = std::make_shared<KdSquarePolygon>(
		"Asset/Textures/player/player new jump.png");
	m_jumpSprite->SetSplit(6, 1);
	m_jumpSprite->SetUVRect(0);
	m_jumpSprite->SetScale({ 1.08f, 1.8f });
	m_jumpSprite->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape(
		"Player", Math::Vector3(0.0f, 0.9f, 0.0f), 0.4f,
		KdCollider::TypeBump | KdCollider::TypeEvent);
}

void PlayerController2_5D::Update()
{
	if (GameManager::Instance().IsPaused()) { return; }

	auto managerArea = AreaManager::Instance().GetCurrentArea();
	if (managerArea) { m_currentArea = managerArea; }

	if (!m_controlEnabled)
	{
		m_carrySlot.UpdateCarryPosition(GetPos(), m_facingDirection);
		return;
	}

	MoveLeftRight();
	ResolveHorizontalCollisions();
	m_groundChecker.CheckGround();
	Jump();
	Crouch();
	ApplyGravity();
	LockZToCurrentArea();
	m_groundChecker.CheckGround();
	UpdateAnimation();

	m_interactor.DetectInteractable();
	if (GetAsyncKeyState('E') & 0x0001)
	{
		m_interactor.Interact();
	}

	auto currentBall = m_carrySlot.GetCurrentBall();
	auto currentArea = m_currentArea.lock();
	if (currentBall && currentArea) { currentBall->SetOwnerArea(currentArea); }
	m_carrySlot.UpdateCarryPosition(GetPos(), m_facingDirection);
	m_ballThrowController.UpdateInput(m_carrySlot, m_facingDirection);
	m_ballThrowController.MoveFlyingBall();
}

void PlayerController2_5D::DrawUnLit()
{
	std::shared_ptr<KdSquarePolygon> currentSprite;
	switch (m_animationState)
	{
	case AnimationState::Walk:
		currentSprite = m_walkSprite;
		break;
	case AnimationState::Jump:
	case AnimationState::Fall:
		currentSprite = m_jumpSprite;
		break;
	case AnimationState::Idle:
	default:
		currentSprite = m_idleSprite;
		break;
	}
	if (!currentSprite) { return; }

	Math::Matrix drawMatrix =
		Math::Matrix::CreateScale(m_facingDirection, 1.0f, 1.0f) * m_mWorld;
	Math::Vector3 drawPosition = drawMatrix.Translation();
	drawPosition.z -= 1.1f;
	drawMatrix.Translation(drawPosition);
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*currentSprite, drawMatrix);
}

void PlayerController2_5D::MoveLeftRight()
{
	m_moveInput = 0.0f;
	if (GetAsyncKeyState('A') & 0x8000) { m_moveInput -= 1.0f; }
	if (GetAsyncKeyState('D') & 0x8000) { m_moveInput += 1.0f; }

	if (m_moveInput != 0.0f) { m_facingDirection = m_moveInput; }

	Math::Vector3 position = GetPos();
	position.x += m_moveInput * m_moveSpeed * Application::Instance().GetDeltaSeconds();
	SetPos(position);
}

void PlayerController2_5D::ResolveHorizontalCollisions()
{
	if (m_moveInput == 0.0f) { return; }

	// The player's origin is at the feet. Use a sphere around the body to
	// collide against the map mesh and apply only horizontal pushback.
	for (int pass = 0; pass < 3; ++pass)
	{
		const Math::Vector3 bodyCenter = GetPos() + Math::Vector3(0.0f, 0.9f, 0.0f);
		const KdCollider::SphereInfo bodySphere(
			KdCollider::TypeBump, bodyCenter, m_collisionRadius);

		float horizontalCorrection = 0.0f;
		for (auto& object : SceneManager::Instance().GetObjList())
		{
			if (!object || object.get() == this) { continue; }
			if (std::dynamic_pointer_cast<PortalDoor>(object)) { continue; }

			std::list<KdCollider::CollisionResult> results;
			if (!object->Intersects(bodySphere, &results)) { continue; }

			for (const auto& result : results)
			{
				// Ground and ceiling contacts are handled by vertical movement.
				if (std::abs(result.m_hitDir.x) <= std::abs(result.m_hitDir.y))
				{
					continue;
				}
				horizontalCorrection += result.m_hitDir.x * result.m_overlapDistance;
			}
		}

		if (std::abs(horizontalCorrection) < 0.0001f) { break; }

		Math::Vector3 position = GetPos();
		position.x += horizontalCorrection;
		SetPos(position);
	}
}

void PlayerController2_5D::UpdateAnimation()
{
	AnimationState desiredState = AnimationState::Idle;
	if (m_verticalVelocity > 0.01f)
	{
		desiredState = AnimationState::Jump;
	}
	else if (!m_groundChecker.IsGrounded())
	{
		desiredState = AnimationState::Fall;
	}
	else if (m_moveInput != 0.0f)
	{
		desiredState = AnimationState::Walk;
	}

	if (desiredState != m_animationState)
	{
		m_animationState = desiredState;
		m_animationTimer = 0.0f;
		m_animationFrame = m_animationState == AnimationState::Fall ? 5 : 0;

		if (m_animationState == AnimationState::Walk && m_walkSprite)
		{
			m_walkSprite->SetUVRect(0);
		}
		else if ((m_animationState == AnimationState::Jump ||
			m_animationState == AnimationState::Fall) && m_jumpSprite)
		{
			m_jumpSprite->SetUVRect(m_animationFrame);
		}
		else if (m_idleSprite)
		{
			m_idleSprite->SetUVRect(0);
		}
	}

	if (m_animationState == AnimationState::Fall)
	{
		m_animationFrame = 5;
		if (m_jumpSprite) { m_jumpSprite->SetUVRect(5); }
		return;
	}

	const float frameDuration =
		m_animationState == AnimationState::Idle ? 0.1f : 0.08f;
	const int frameCount =
		m_animationState == AnimationState::Idle ? 10 :
		m_animationState == AnimationState::Walk ? 8 : 6;
	m_animationTimer += Application::Instance().GetDeltaSeconds();

	while (m_animationTimer >= frameDuration)
	{
		m_animationTimer -= frameDuration;
		if (m_animationState == AnimationState::Jump)
		{
			m_animationFrame = std::min(m_animationFrame + 1, frameCount - 1);
		}
		else
		{
			m_animationFrame = (m_animationFrame + 1) % frameCount;
		}

		if (m_animationState == AnimationState::Walk && m_walkSprite)
		{
			m_walkSprite->SetUVRect(m_animationFrame);
		}
		else if (m_animationState == AnimationState::Jump && m_jumpSprite)
		{
			m_jumpSprite->SetUVRect(m_animationFrame);
		}
		else if (m_idleSprite)
		{
			m_idleSprite->SetUVRect(m_animationFrame);
		}
	}
}

void PlayerController2_5D::Jump()
{
	if (!m_groundChecker.IsGrounded()) { return; }

	if (GetAsyncKeyState('W') & 0x0001)
	{
		m_verticalVelocity = m_jumpForce;
	}
	else if (m_verticalVelocity < 0.0f)
	{
		m_verticalVelocity = 0.0f;
	}
}

void PlayerController2_5D::Crouch()
{
	m_isCrouching = (GetAsyncKeyState('S') & 0x8000) != 0;
}

void PlayerController2_5D::ApplyGravity()
{
	if (m_groundChecker.IsGrounded() && m_verticalVelocity <= 0.0f)
	{
		m_verticalVelocity = 0.0f;
		Math::Vector3 groundedPosition = GetPos();
		groundedPosition.y = m_groundChecker.GetGroundHeight();
		SetPos(groundedPosition);
		return;
	}

	const float deltaSeconds = Application::Instance().GetDeltaSeconds();
	m_verticalVelocity -= m_gravity * deltaSeconds;
	m_verticalVelocity = std::max(m_verticalVelocity, -12.0f);

	Math::Vector3 position = GetPos();
	position.y += m_verticalVelocity * deltaSeconds;
	SetPos(position);
}

void PlayerController2_5D::LockZToCurrentArea()
{
	auto area = m_currentArea.lock();
	const float gameplayZ = area
		? area->GetGameplayZ()
		: AreaManager::Instance().GetCurrentGameplayZ();

	Math::Vector3 position = GetPos();
	position.z = gameplayZ;
	SetPos(position);
}

void PlayerController2_5D::ApplyTeleport(
	const Math::Vector3& position, float facingDirection)
{
	SetPos(position);
	m_facingDirection = facingDirection < 0.0f ? -1.0f : 1.0f;
	m_verticalVelocity = 0.0f;
	m_moveInput = 0.0f;
	m_groundChecker.CheckGround();

	auto currentBall = m_carrySlot.GetCurrentBall();
	auto currentArea = m_currentArea.lock();
	if (currentBall && currentArea)
	{
		currentBall->SetOwnerArea(currentArea);
	}
	m_carrySlot.SnapCarryPosition(GetPos(), m_facingDirection);
}
