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

	m_runSprite = std::make_shared<KdSquarePolygon>(
		"Asset/Textures/player/player run.png");
	m_runSprite->SetSplit(8, 1);
	m_runSprite->SetUVRect(0);
	m_runSprite->SetScale({ 1.08f, 1.8f });
	m_runSprite->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	m_jumpSprite = std::make_shared<KdSquarePolygon>(
		"Asset/Textures/player/player new jump.png");
	m_jumpSprite->SetSplit(6, 1);
	m_jumpSprite->SetUVRect(0);
	m_jumpSprite->SetScale({ 1.08f, 1.8f });
	m_jumpSprite->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	m_ledgeClimbSprite = std::make_shared<KdSquarePolygon>(
		"Asset/Textures/player/player ledge climb.png");
	m_ledgeClimbSprite->SetSplit(5, 1);
	m_ledgeClimbSprite->SetUVRect(0);
	m_ledgeClimbSprite->SetScale({ 1.08f, 1.8f });
	m_ledgeClimbSprite->SetPivot(
		KdSquarePolygon::PivotType::Center_Bottom);

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

	if (m_isLedgeClimbing)
	{
		UpdateLedgeClimb();
		m_carrySlot.UpdateCarryPosition(GetPos(), m_facingDirection);
		return;
	}

	if (!m_controlEnabled)
	{
		m_carrySlot.UpdateCarryPosition(GetPos(), m_facingDirection);
		return;
	}

	const float deltaSeconds = Application::Instance().GetDeltaSeconds();
	if (GetAsyncKeyState('F') & 0x0001)
	{
		m_ledgeInputBufferTimer = m_ledgeInputBufferDuration;
	}
	else
	{
		m_ledgeInputBufferTimer = std::max(
			0.0f, m_ledgeInputBufferTimer - deltaSeconds);
	}

	MoveLeftRight();
	ResolveHorizontalCollisions();
	m_groundChecker.CheckGround();
	if (TryStartLedgeClimb())
	{
		m_carrySlot.UpdateCarryPosition(GetPos(), m_facingDirection);
		return;
	}
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
	case AnimationState::Run:
		currentSprite = m_runSprite;
		break;
	case AnimationState::Jump:
	case AnimationState::Fall:
		currentSprite = m_jumpSprite;
		break;
	case AnimationState::LedgeClimb:
		currentSprite = m_ledgeClimbSprite;
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
	const float deltaSeconds = Application::Instance().GetDeltaSeconds();
	m_leftTapTimer = std::max(0.0f, m_leftTapTimer - deltaSeconds);
	m_rightTapTimer = std::max(0.0f, m_rightTapTimer - deltaSeconds);

	const bool leftHeld = (GetAsyncKeyState('A') & 0x8000) != 0;
	const bool rightHeld = (GetAsyncKeyState('D') & 0x8000) != 0;
	const bool leftPressed = leftHeld && !m_wasLeftHeld;
	const bool rightPressed = rightHeld && !m_wasRightHeld;

	if (leftPressed)
	{
		if (m_leftTapTimer > 0.0f)
		{
			m_isRunning = true;
			m_runningDirection = -1.0f;
			m_leftTapTimer = 0.0f;
		}
		else
		{
			m_leftTapTimer = m_doubleTapWindow;
		}
	}

	if (rightPressed)
	{
		if (m_rightTapTimer > 0.0f)
		{
			m_isRunning = true;
			m_runningDirection = 1.0f;
			m_rightTapTimer = 0.0f;
		}
		else
		{
			m_rightTapTimer = m_doubleTapWindow;
		}
	}

	if (m_isRunning)
	{
		const bool runningKeyHeld =
			m_runningDirection < 0.0f ? leftHeld : rightHeld;
		const bool oppositeKeyHeld =
			m_runningDirection < 0.0f ? rightHeld : leftHeld;
		if (!runningKeyHeld || oppositeKeyHeld)
		{
			m_isRunning = false;
			m_runningDirection = 0.0f;
		}
	}

	m_moveInput = 0.0f;
	if (leftHeld) { m_moveInput -= 1.0f; }
	if (rightHeld) { m_moveInput += 1.0f; }

	if (m_moveInput != 0.0f) { m_facingDirection = m_moveInput; }

	const float movementSpeed =
		m_moveSpeed * (m_isRunning ? m_runSpeedMultiplier : 1.0f);
	const float horizontalDistance =
		m_moveInput * movementSpeed * deltaSeconds;
	const int movementSteps = std::max(
		1,
		static_cast<int>(
			std::ceil(std::abs(horizontalDistance) / 0.12f)));
	const float stepDistance = horizontalDistance / movementSteps;

	// Resolve collision for every short movement segment. This prevents a
	// running player from crossing a thin wall/corner between two frames.
	for (int step = 0; step < movementSteps; ++step)
	{
		Math::Vector3 position = GetPos();
		position.x += stepDistance;
		SetPos(position);
		ResolveHorizontalCollisions();
	}

	m_wasLeftHeld = leftHeld;
	m_wasRightHeld = rightHeld;
}

void PlayerController2_5D::ResolveHorizontalCollisions()
{
	if (m_moveInput == 0.0f) { return; }

	// Two spheres approximate a vertical capsule. The old single sphere left
	// the feet/lower body unprotected when approaching a block corner.
	constexpr float bodySphereHeights[] = { 0.45f, 1.25f };
	for (int pass = 0; pass < 4; ++pass)
	{
		float horizontalCorrection = 0.0f;
		for (float sphereHeight : bodySphereHeights)
		{
			const Math::Vector3 bodyCenter =
				GetPos() + Math::Vector3(0.0f, sphereHeight, 0.0f);
			const KdCollider::SphereInfo bodySphere(
				KdCollider::TypeBump, bodyCenter, m_collisionRadius);

			for (auto& object : SceneManager::Instance().GetObjList())
			{
				if (!object || object.get() == this) { continue; }
				auto portalDoor = std::dynamic_pointer_cast<PortalDoor>(object);
				if (portalDoor && !portalDoor->IsSolidForPlayer()) { continue; }

				std::list<KdCollider::CollisionResult> results;
				if (!object->Intersects(bodySphere, &results)) { continue; }

				for (const auto& result : results)
				{
					const float correctionX =
						result.m_hitDir.x * result.m_overlapDistance;

					// Flat ground has no horizontal correction. At an edge the
					// push direction is diagonal; keep its X component when it
					// opposes movement instead of misclassifying it as ground.
					if (std::abs(correctionX) < 0.00001f ||
						correctionX * m_moveInput >= 0.0f)
					{
						continue;
					}
					horizontalCorrection += correctionX;
				}
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
	if (m_isLedgeClimbing)
	{
		m_animationState = AnimationState::LedgeClimb;
		return;
	}

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
		desiredState = m_isRunning
			? AnimationState::Run
			: AnimationState::Walk;
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
		else if (m_animationState == AnimationState::Run && m_runSprite)
		{
			m_runSprite->SetUVRect(0);
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
		m_animationState == AnimationState::Idle ? 0.1f :
		m_animationState == AnimationState::Run ? 0.06f : 0.08f;
	const int frameCount =
		m_animationState == AnimationState::Idle ? 10 :
		(m_animationState == AnimationState::Walk ||
			m_animationState == AnimationState::Run) ? 8 : 6;
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
		else if (m_animationState == AnimationState::Run && m_runSprite)
		{
			m_runSprite->SetUVRect(m_animationFrame);
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

bool PlayerController2_5D::TryStartLedgeClimb()
{
	if (m_groundChecker.IsGrounded() ||
		m_ledgeInputBufferTimer <= 0.0f)
	{
		return false;
	}

	const float facing = m_facingDirection < 0.0f ? -1.0f : 1.0f;
	const Math::Vector3 playerPosition = GetPos();

	std::shared_ptr<KdGameObject> wallObject;
	KdCollider::CollisionResult nearestWallHit;
	float nearestWallDistance = FLT_MAX;

	// Probe at two body heights. This makes grabbing reliable near the apex
	// and while beginning to fall, instead of requiring one exact pose/frame.
	constexpr float wallProbeHeights[] = { 0.55f, 1.05f };
	for (float probeHeight : wallProbeHeights)
	{
		const Math::Vector3 wallRayStart =
			playerPosition + Math::Vector3(0.0f, probeHeight, 0.0f);
		const KdCollider::RayInfo wallRay(
			KdCollider::TypeBump,
			wallRayStart,
			Math::Vector3(facing, 0.0f, 0.0f),
			m_ledgeWallCheckDistance);

		for (auto& object : SceneManager::Instance().GetObjList())
		{
			if (!object || object.get() == this) { continue; }
			if (std::dynamic_pointer_cast<PortalDoor>(object)) { continue; }

			std::list<KdCollider::CollisionResult> results;
			if (!object->Intersects(wallRay, &results)) { continue; }

			for (const auto& result : results)
			{
				const float distance =
					(result.m_hitPos - wallRayStart).Length();
				if (distance < nearestWallDistance &&
					std::abs(result.m_hitNDir.x) > 0.35f)
				{
					nearestWallDistance = distance;
					nearestWallHit = result;
					wallObject = object;
				}
			}
		}
	}

	if (!wallObject) { return false; }

	// Cast down just beyond the wall face to find the walkable top.
	const float topProbeX =
		nearestWallHit.m_hitPos.x + facing * (m_collisionRadius + 0.12f);
	const Math::Vector3 topRayStart(
		topProbeX,
		playerPosition.y + m_ledgeMaxHeight + 0.35f,
		playerPosition.z);
	const KdCollider::RayInfo topRay(
		KdCollider::TypeGround,
		topRayStart,
		Math::Vector3::Down,
		m_ledgeMaxHeight + 0.7f);

	std::list<KdCollider::CollisionResult> topResults;
	if (!wallObject->Intersects(topRay, &topResults)) { return false; }

	float platformTopY = -FLT_MAX;
	for (const auto& result : topResults)
	{
		if (result.m_hitPos.y > platformTopY)
		{
			platformTopY = result.m_hitPos.y;
		}
	}
	if (platformTopY == -FLT_MAX) { return false; }

	const float climbHeight = platformTopY - playerPosition.y;
	if (climbHeight < m_ledgeMinHeight ||
		climbHeight > m_ledgeMaxHeight)
	{
		return false;
	}

	const Math::Vector3 targetPosition(
		topProbeX,
		platformTopY,
		playerPosition.z);

	// Ensure the upper body has room at the destination.
	const KdCollider::SphereInfo clearanceSphere(
		KdCollider::TypeBump,
		targetPosition + Math::Vector3(0.0f, 0.95f, 0.0f),
		m_collisionRadius * 0.9f);
	for (auto& object : SceneManager::Instance().GetObjList())
	{
		if (!object || object.get() == this || object == wallObject) { continue; }
		if (std::dynamic_pointer_cast<PortalDoor>(object)) { continue; }
		if (object->Intersects(clearanceSphere, nullptr)) { return false; }
	}

	m_isLedgeClimbing = true;
	m_ledgeInputBufferTimer = 0.0f;
	m_isRunning = false;
	m_runningDirection = 0.0f;
	m_moveInput = 0.0f;
	m_verticalVelocity = 0.0f;
	m_ledgeClimbElapsed = 0.0f;
	m_ledgeClimbStart = {
		nearestWallHit.m_hitPos.x -
			facing * (m_collisionRadius + 0.04f),
		std::min(playerPosition.y, platformTopY - 0.85f),
		playerPosition.z
	};
	m_ledgeClimbTarget = targetPosition;
	SetPos(m_ledgeClimbStart);
	m_animationState = AnimationState::LedgeClimb;
	m_animationFrame = 0;
	m_animationTimer = 0.0f;
	if (m_ledgeClimbSprite) { m_ledgeClimbSprite->SetUVRect(0); }
	return true;
}

void PlayerController2_5D::UpdateLedgeClimb()
{
	constexpr int frameCount = 5;
	const float totalDuration =
		frameCount * m_ledgeClimbFrameDuration;
	m_ledgeClimbElapsed += Application::Instance().GetDeltaSeconds();

	m_animationFrame = std::min(
		static_cast<int>(
			m_ledgeClimbElapsed / m_ledgeClimbFrameDuration),
		frameCount - 1);
	if (m_ledgeClimbSprite)
	{
		m_ledgeClimbSprite->SetUVRect(m_animationFrame);
	}

	const float normalizedTime = std::clamp(
		m_ledgeClimbElapsed / totalDuration, 0.0f, 1.0f);
	const float smoothTime =
		normalizedTime * normalizedTime *
		(3.0f - 2.0f * normalizedTime);
	SetPos(Math::Vector3::Lerp(
		m_ledgeClimbStart,
		m_ledgeClimbTarget,
		smoothTime));
	LockZToCurrentArea();

	if (m_ledgeClimbElapsed < totalDuration) { return; }

	SetPos(m_ledgeClimbTarget);
	m_isLedgeClimbing = false;
	m_animationState = AnimationState::Idle;
	m_animationFrame = 0;
	m_animationTimer = 0.0f;
	m_groundChecker.CheckGround();
	if (m_idleSprite) { m_idleSprite->SetUVRect(0); }
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
	m_isLedgeClimbing = false;
	m_ledgeInputBufferTimer = 0.0f;
	m_isRunning = false;
	m_runningDirection = 0.0f;
	m_leftTapTimer = 0.0f;
	m_rightTapTimer = 0.0f;
	m_groundChecker.CheckGround();

	auto currentBall = m_carrySlot.GetCurrentBall();
	auto currentArea = m_currentArea.lock();
	if (currentBall && currentArea)
	{
		currentBall->SetOwnerArea(currentArea);
	}
	m_carrySlot.SnapCarryPosition(GetPos(), m_facingDirection);
}
