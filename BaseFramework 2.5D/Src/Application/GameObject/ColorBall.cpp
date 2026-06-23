#include "ColorBall.h"
#include "PlayerController2_5D.h"
#include "PortalDoor.h"
#include "../GameSystem/GameManager.h"
#include "../GameSystem/LevelManager.h"
#include "../Scene/SceneManager.h"
#include "../main.h"

void ColorBall::Init()
{
	m_model = std::make_shared<KdModelData>();
	if (!m_model->Load("Asset/Textures/ball/ball.gltf"))
	{
		m_model.reset();
		m_isExpired = true;
		return;
	}

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape(
		"ColorBall", Math::Vector3::Zero, m_collisionRadius,
		KdCollider::TypeEvent | KdCollider::TypeDamage);
	SetBallColor(m_ballColor);
	SaveInitialState();
}

void ColorBall::Update()
{
	if (GameManager::Instance().IsPaused()) { return; }
	const float deltaSeconds = Application::Instance().GetDeltaSeconds();
	m_visualTime += deltaSeconds;
	if (m_ballColor == GameColor::Rainbow && m_model)
	{
		const auto channel = [this](float phase)
		{
			return 0.55f + 0.45f * std::sin(m_visualTime * 3.2f + phase);
		};
		for (auto& material : m_model->WorkMaterials())
		{
			if (material.m_name == "M_BallTintable")
			{
				material.m_baseColorRate = {
					channel(0.0f),
					channel(2.094395f),
					channel(4.188790f),
					1.0f
				};
				break;
			}
		}
	}
	if (m_isConsumed)
	{
		m_portalRespawnTimer = std::max(
			0.0f, m_portalRespawnTimer - deltaSeconds);
		if (m_portalRespawnTimer <= 0.0f)
		{
			ResetState();
		}
		return;
	}
	if (m_isAbandonedGone) { return; }
	if (m_isStored) { return; }

	if (m_isAbandonCountdownActive && !m_isHeld)
	{
		m_abandonTimer += deltaSeconds;
		if (m_abandonTimer >= m_abandonDisappearTime)
		{
			// An abandoned ball returns to its level-defined spawn point
			// instead of remaining permanently removed from the level.
			ResetState();
			return;
		}
	}
	if (!m_isHeld && GetPos().y < LevelManager::Instance().GetVoidY())
	{
		ResetState();
		return;
	}

	if (m_isHeld)
	{
		Math::Vector3 position = GetPos();
		const Math::Vector3 displacement = m_heldTargetPosition - position;
		const Math::Vector3 acceleration =
			displacement * m_carrySpring - m_carryVelocity * m_carryDamping;

		m_carryVelocity += acceleration * deltaSeconds;
		if (m_carryVelocity.LengthSquared() > 144.0f)
		{
			m_carryVelocity.Normalize();
			m_carryVelocity *= 12.0f;
		}

		position += m_carryVelocity * deltaSeconds;

		// A carried ball belongs to the current Area plane and never drifts on Z.
		position.z = m_heldTargetPosition.z;
		m_carryVelocity.z = 0.0f;
		SetPos(position);
		return;
	}

	if (!m_isPhysicsActive) { return; }

	Math::Vector3 position = GetPos();
	m_flightTime += deltaSeconds;
	m_flyVelocity.y -= m_flightGravity * deltaSeconds;
	Math::Vector3 nextPosition = position + m_flyVelocity * deltaSeconds;

	if (m_flightTime >= 0.12f)
	{
		const KdCollider::SphereInfo flyingSphere(
			KdCollider::TypeBump, nextPosition, m_collisionRadius);

		// A thrown ball may still reach a door after landing and rolling.
		// Stationary balls do not color doors automatically.
		if (m_isFlying || m_isRolling)
		{
			for (auto& object : SceneManager::Instance().GetObjList())
			{
				auto door = std::dynamic_pointer_cast<PortalDoor>(object);
				if (door && door->Intersects(flyingSphere, nullptr))
				{
					if (door->OnBallHit(m_ballColor))
					{
						OnHitPortalDoor(door);
						return;
					}
				}
			}
		}

		bool hitGround = false;
		bool hitWall = false;
		for (auto& object : SceneManager::Instance().GetObjList())
		{
			if (!object || object.get() == this) { continue; }
			if (std::dynamic_pointer_cast<PlayerController2_5D>(object)) { continue; }
			if (std::dynamic_pointer_cast<PortalDoor>(object)) { continue; }

			std::list<KdCollider::CollisionResult> results;
			if (!object->Intersects(flyingSphere, &results)) { continue; }

			for (const auto& result : results)
			{
				Math::Vector3 pushDirection = result.m_hitDir;
				if (pushDirection.LengthSquared() <= 0.000001f) { continue; }
				pushDirection.Normalize();
				nextPosition += pushDirection * result.m_overlapDistance;

				const float velocityIntoSurface = m_flyVelocity.Dot(pushDirection);
				if (velocityIntoSurface < 0.0f)
				{
					m_flyVelocity -= pushDirection * velocityIntoSurface;
				}

				if (pushDirection.y > 0.55f)
				{
					hitGround = true;
				}
				else if (std::abs(pushDirection.x) > 0.45f)
				{
					hitWall = true;
				}
			}
		}

		if (hitWall)
		{
			m_isFlying = false;
			m_isRolling = false;
			m_flyVelocity.x = 0.0f;
		}

		if (hitGround && m_flyVelocity.y <= m_restSpeedThreshold)
		{
			m_isFlying = false;
			m_flyVelocity.y = 0.0f;

			if (!m_isRolling)
			{
				// Keep a small amount of the throw's horizontal momentum on
				// first landing so the ball rolls forward instead of freezing.
				m_isRolling = true;
				m_flyVelocity.x *= m_landingHorizontalRetention;
			}

			const float frictionStep = m_groundRollingFriction * deltaSeconds;
			if (std::abs(m_flyVelocity.x) <=
				std::max(m_restSpeedThreshold, frictionStep))
			{
				m_isRolling = false;
				m_isPhysicsActive = false;
				m_flyVelocity = Math::Vector3::Zero;
				if (m_hasBeenMoved)
				{
					m_isAbandonCountdownActive = true;
					m_abandonTimer = 0.0f;
				}
			}
			else
			{
				m_flyVelocity.x +=
					m_flyVelocity.x > 0.0f ? -frictionStep : frictionStep;
			}
		}
	}

	SetPos(nextPosition);

	if (m_isFlying &&
		(nextPosition - m_throwStartPosition).Length() >= m_maxThrowDistance)
	{
		m_isFlying = false;
		m_flyVelocity.x = 0.0f;
	}
}

void ColorBall::GenerateDepthMapFromLight()
{
	if (!m_model || IsConsumed()) { return; }
	if (m_isAbandonCountdownActive &&
		m_abandonTimer >= m_abandonBlinkStartTime &&
		static_cast<int>(m_abandonTimer / m_abandonBlinkInterval) % 2 != 0)
	{
		return;
	}
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void ColorBall::DrawLit()
{
	if (!m_model || IsConsumed()) { return; }
	if (m_isAbandonCountdownActive &&
		m_abandonTimer >= m_abandonBlinkStartTime &&
		static_cast<int>(m_abandonTimer / m_abandonBlinkInterval) % 2 != 0)
	{
		return;
	}
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void ColorBall::SaveInitialState()
{
	m_initialPosition = GetPos();
	m_initialArea = m_ownerArea;
}

void ColorBall::OnPickedUp()
{
	if (m_isConsumed) { return; }
	m_isStored = false;
	m_isHeld = true;
	m_isFlying = false;
	m_isPhysicsActive = false;
	m_isRolling = false;
	m_hasBeenMoved = true;
	m_isAbandonCountdownActive = false;
	m_abandonTimer = 0.0f;
	m_heldTargetPosition = GetPos();
	m_carryVelocity = Math::Vector3::Zero;
}

void ColorBall::OnThrown(const Math::Vector3& direction, float speed, float maxDistance)
{
	m_isHeld = false;
	m_isStored = false;
	m_isFlying = speed > 0.0f && maxDistance > 0.0f;
	m_isPhysicsActive = m_isFlying;
	m_isRolling = false;
	m_hasBeenMoved = true;
	m_isAbandonCountdownActive = false;
	m_abandonTimer = 0.0f;
	m_carryVelocity = Math::Vector3::Zero;
	m_flyDirection = direction;
	if (m_flyDirection.LengthSquared() > 0.0f) { m_flyDirection.Normalize(); }
	m_throwSpeed = speed;
	m_maxThrowDistance = maxDistance;
	m_throwStartPosition = GetPos();
	m_flyVelocity = m_flyDirection * m_throwSpeed;
	m_flightGravity = 0.0f;
	m_flightTime = 0.0f;
}

void ColorBall::OnThrownArc(
	const Math::Vector3& launchVelocity, float gravity, float maxDistance)
{
	m_isHeld = false;
	m_isStored = false;
	m_isFlying = launchVelocity.LengthSquared() > 0.0f && maxDistance > 0.0f;
	m_isPhysicsActive = m_isFlying;
	m_isRolling = false;
	m_hasBeenMoved = true;
	m_isAbandonCountdownActive = false;
	m_abandonTimer = 0.0f;
	m_carryVelocity = Math::Vector3::Zero;
	m_flyVelocity = launchVelocity;
	m_flightGravity = std::max(0.0f, gravity);
	m_maxThrowDistance = maxDistance;
	m_throwStartPosition = GetPos();
	m_flightTime = 0.0f;
}

void ColorBall::OnHitPortalDoor(const std::shared_ptr<KdGameObject>&)
{
	m_isFlying = false;
	m_isPhysicsActive = false;
	m_isRolling = false;
	m_isHeld = false;
	m_isStored = false;
	m_isConsumed = true;
	m_isAbandonCountdownActive = false;
	m_abandonTimer = 0.0f;
	m_portalRespawnTimer = m_portalRespawnDelay;
	m_flyVelocity = Math::Vector3::Zero;
	m_carryVelocity = Math::Vector3::Zero;
}

void ColorBall::ResetState()
{
	m_ownerArea = m_initialArea;
	m_isHeld = false;
	m_isStored = false;
	m_isFlying = false;
	m_isPhysicsActive = false;
	m_isRolling = false;
	m_isConsumed = false;
	m_isAbandonedGone = false;
	m_hasBeenMoved = false;
	m_isAbandonCountdownActive = false;
	m_portalRespawnTimer = 0.0f;
	m_abandonTimer = 0.0f;
	m_flyDirection = Math::Vector3::Zero;
	m_flyVelocity = Math::Vector3::Zero;
	m_carryVelocity = Math::Vector3::Zero;
	if (m_pCollider) { m_pCollider->SetEnable("ColorBall", true); }
	SetPos(m_initialPosition);
}

void ColorBall::SetHeldPosition(const Math::Vector3& position)
{
	if (m_isHeld) { m_heldTargetPosition = position; }
}

void ColorBall::SnapHeldPosition(const Math::Vector3& position)
{
	if (!m_isHeld) { return; }
	m_heldTargetPosition = position;
	m_carryVelocity = Math::Vector3::Zero;
	m_flyVelocity = Math::Vector3::Zero;
	m_isFlying = false;
	m_isPhysicsActive = false;
	m_isRolling = false;
	SetPos(position);
}

void ColorBall::DropAt(const Math::Vector3& position)
{
	m_isStored = false;
	m_isHeld = false;
	m_isFlying = false;
	m_isPhysicsActive = true;
	m_isRolling = false;
	m_hasBeenMoved = true;
	m_isAbandonCountdownActive = false;
	m_abandonTimer = 0.0f;
	m_flyVelocity = Math::Vector3::Zero;
	m_flightGravity = 18.0f;
	m_flightTime = 0.12f;
	m_carryVelocity = Math::Vector3::Zero;
	SetPos(position);
}

void ColorBall::StoreAt(const Math::Vector3& position)
{
	m_isStored = true;
	m_isHeld = false;
	m_isFlying = false;
	m_isPhysicsActive = false;
	m_isRolling = false;
	m_isConsumed = false;
	m_isAbandonedGone = false;
	m_hasBeenMoved = true;
	m_isAbandonCountdownActive = false;
	m_abandonTimer = 0.0f;
	m_flyVelocity = Math::Vector3::Zero;
	m_carryVelocity = Math::Vector3::Zero;
	if (m_pCollider) { m_pCollider->SetEnable("ColorBall", false); }
	SetPos(position);
}

void ColorBall::ReleaseFromStorage()
{
	m_isStored = false;
	if (m_pCollider) { m_pCollider->SetEnable("ColorBall", true); }
}

void ColorBall::SetBallColor(GameColor color)
{
	m_ballColor = color;
	if (!m_model) { return; }

	Math::Vector4 visualColor = { 0.8f, 0.8f, 0.8f, 1.0f };
	if (color == GameColor::Black)
	{
		visualColor = { 0.025f, 0.025f, 0.035f, 1.0f };
	}
	else if (color == GameColor::White)
	{
		visualColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	}
	else if (color == GameColor::Red)
	{
		visualColor = { 1.0f, 0.035f, 0.025f, 1.0f };
	}
	else if (color == GameColor::Blue)
	{
		visualColor = { 0.03f, 0.22f, 1.0f, 1.0f };
	}
	else if (color == GameColor::Yellow)
	{
		visualColor = { 1.0f, 0.82f, 0.04f, 1.0f };
	}
	else if (color == GameColor::Rainbow)
	{
		visualColor = { 1.0f, 0.25f, 0.75f, 1.0f };
	}

	for (auto& material : m_model->WorkMaterials())
	{
		if (material.m_name == "M_BallTintable")
		{
			material.m_baseColorRate = visualColor;
			break;
		}
	}
}
