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
	if (m_isConsumed) { return; }
	if (!m_isHeld && GetPos().y < LevelManager::Instance().GetVoidY())
	{
		ResetState();
		return;
	}

	const float deltaSeconds = Application::Instance().GetDeltaSeconds();

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

	if (!m_isFlying) { return; }

	Math::Vector3 position = GetPos();
	m_flightTime += deltaSeconds;
	m_flyVelocity.y -= m_flightGravity * deltaSeconds;
	const Math::Vector3 nextPosition = position + m_flyVelocity * deltaSeconds;

	if (m_flightTime >= 0.12f)
	{
		const KdCollider::SphereInfo flyingSphere(
			KdCollider::TypeBump, nextPosition, m_collisionRadius);

		// Doors take priority over the surrounding map so a ball touching both
		// on the same frame still colors the door.
		for (auto& object : SceneManager::Instance().GetObjList())
		{
			auto door = std::dynamic_pointer_cast<PortalDoor>(object);
			if (door && door->Intersects(flyingSphere, nullptr))
			{
				door->OnBallHit(m_ballColor);
				OnHitPortalDoor(door);
				return;
			}
		}

		for (auto& object : SceneManager::Instance().GetObjList())
		{
			if (!object || object.get() == this) { continue; }
			if (std::dynamic_pointer_cast<PlayerController2_5D>(object)) { continue; }
			if (std::dynamic_pointer_cast<PortalDoor>(object)) { continue; }
			if (object->Intersects(flyingSphere, nullptr))
			{
				m_isFlying = false;
				m_flyVelocity = Math::Vector3::Zero;
				return;
			}
		}
	}

	SetPos(nextPosition);

	if ((nextPosition - m_throwStartPosition).Length() >= m_maxThrowDistance)
	{
		m_isFlying = false;
		m_flyVelocity = Math::Vector3::Zero;
	}
}

void ColorBall::GenerateDepthMapFromLight()
{
	if (!m_model || m_isConsumed) { return; }
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void ColorBall::DrawLit()
{
	if (!m_model || m_isConsumed) { return; }
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
	m_isHeld = true;
	m_isFlying = false;
	m_heldTargetPosition = GetPos();
	m_carryVelocity = Math::Vector3::Zero;
}

void ColorBall::OnThrown(const Math::Vector3& direction, float speed, float maxDistance)
{
	m_isHeld = false;
	m_isFlying = speed > 0.0f && maxDistance > 0.0f;
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
	m_isFlying = launchVelocity.LengthSquared() > 0.0f && maxDistance > 0.0f;
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
	m_isHeld = false;
	m_isConsumed = true;
	m_flyVelocity = Math::Vector3::Zero;
	m_carryVelocity = Math::Vector3::Zero;
}

void ColorBall::ResetState()
{
	m_ownerArea = m_initialArea;
	m_isHeld = false;
	m_isFlying = false;
	m_isConsumed = false;
	m_flyDirection = Math::Vector3::Zero;
	m_flyVelocity = Math::Vector3::Zero;
	m_carryVelocity = Math::Vector3::Zero;
	SetPos(m_initialPosition);
}

void ColorBall::SetHeldPosition(const Math::Vector3& position)
{
	if (m_isHeld) { m_heldTargetPosition = position; }
}

void ColorBall::DropAt(const Math::Vector3& position)
{
	m_isHeld = false;
	m_isFlying = false;
	m_flyVelocity = Math::Vector3::Zero;
	m_carryVelocity = Math::Vector3::Zero;
	SetPos(position);
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

	for (auto& material : m_model->WorkMaterials())
	{
		if (material.m_name == "M_BallTintable")
		{
			material.m_baseColorRate = visualColor;
			break;
		}
	}
}
