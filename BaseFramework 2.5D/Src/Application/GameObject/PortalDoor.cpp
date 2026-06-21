#include "PortalDoor.h"
#include "PlayerController2_5D.h"
#include "../GameSystem/MapArea.h"
#include "../GameSystem/PortalNetworkManager.h"
#include "../GameSystem/PortalTeleportService.h"
#include "../Scene/SceneManager.h"
#include "../main.h"

namespace
{
	constexpr float DoorModelCenterX = 0.794055f;
	constexpr float DoorModelCenterZ = 1.017439f;
}

void PortalDoor::Init()
{
	m_model = std::make_shared<KdModelData>();
	if (!m_model->Load("Asset/Data/door/door.gltf"))
	{
		m_model.reset();
		m_isExpired = true;
		return;
	}

	m_hasPortalSurface = m_model->FindNode("Portal_Surface") != nullptr;
	m_visualController.SetModel(m_model);
	ResetToInitialState();

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape(
		"PortalDoor", m_model, KdCollider::TypeBump | KdCollider::TypeEvent);
}

void PortalDoor::GenerateDepthMapFromLight()
{
	if (!m_model) { return; }
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void PortalDoor::Update()
{
	if (m_teleportCooldownRemaining > 0.0f)
	{
		m_teleportCooldownRemaining = std::max(
			0.0f,
			m_teleportCooldownRemaining - Application::Instance().GetDeltaSeconds());
	}

	if (!m_isActivated || IsTeleportCoolingDown()) { return; }
	DetectPlayerEntry();
}

void PortalDoor::DrawLit()
{
	if (!m_model) { return; }
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void PortalDoor::SetPlacement(const Math::Vector3& centerPosition)
{
	m_centerPosition = centerPosition;
	const Math::Matrix centerCorrection =
		Math::Matrix::CreateTranslation(-DoorModelCenterX, 0.0f, -DoorModelCenterZ);

	m_mWorld =
		centerCorrection *
		Math::Matrix::CreateTranslation(centerPosition);
}

void PortalDoor::SetExitPoint(
	const Math::Vector3& position, float facingDirectionAfterExit)
{
	m_exitPoint.SetPosition(position);
	m_exitPoint.SetFacingDirectionAfterExit(facingDirectionAfterExit);
}

Math::Vector3 PortalDoor::GetExitPosition(float side) const
{
	const float normalizedSide = side < 0.0f ? -1.0f : 1.0f;
	auto area = m_ownerArea.lock();
	const float gameplayZ = area ? area->GetGameplayZ() : m_centerPosition.z;

	return {
		m_centerPosition.x + normalizedSide * m_exitDistance,
		m_centerPosition.y - 1.0f,
		gameplayZ
	};
}

void PortalDoor::BeginTeleportCooldown(float seconds)
{
	m_teleportCooldownRemaining = std::max(
		m_teleportCooldownRemaining,
		std::max(0.0f, seconds));
}

bool PortalDoor::SetColor(GameColor color)
{
	if (color != GameColor::None &&
		color != GameColor::Black &&
		color != GameColor::White)
	{
		return false;
	}

	m_currentColor = color;
	m_isActivated = color != GameColor::None;
	m_visualController.UpdateDoorColorVisual(m_currentColor, m_isActivated);
	return true;
}

bool PortalDoor::OnBallHit(GameColor ballColor)
{
	if (!m_canBeActivated || !m_canBeColored || m_isFixedColor) { return false; }
	if (ballColor != GameColor::Black && ballColor != GameColor::White) { return false; }
	return SetColor(ballColor);
}

void PortalDoor::ResetToInitialState()
{
	m_teleportCooldownRemaining = 0.0f;
	SetColor(m_initialColor);
}

void PortalDoor::SetInitialColor(GameColor color)
{
	m_initialColor = color;
	SetColor(m_initialColor);
}

void PortalDoor::DetectPlayerEntry()
{
	auto self = std::dynamic_pointer_cast<PortalDoor>(shared_from_this());
	auto linkedDoor = PortalNetworkManager::Instance().GetLinkedDoor(self);
	if (!linkedDoor) { return; }

	const Math::Vector3 doorCenter = m_centerPosition;
	for (auto& object : SceneManager::Instance().GetObjList())
	{
		auto player = std::dynamic_pointer_cast<PlayerController2_5D>(object);
		if (!player || player->GetCurrentArea() != GetOwnerArea()) { continue; }

		const Math::Vector3 playerPosition = player->GetPos();
		const bool insideTrigger =
			std::abs(playerPosition.x - doorCenter.x) <= m_playerTriggerHalfWidth &&
			playerPosition.y >= doorCenter.y - 1.25f &&
			playerPosition.y <= doorCenter.y + 1.25f &&
			std::abs(playerPosition.z - doorCenter.z) <= m_playerTriggerHalfDepth;

		if (insideTrigger)
		{
			float entrySide = playerPosition.x < doorCenter.x ? -1.0f : 1.0f;
			if (std::abs(playerPosition.x - doorCenter.x) < 0.01f)
			{
				entrySide = player->GetFacingDirection() < 0.0f ? 1.0f : -1.0f;
			}
			PortalTeleportService::Instance().TeleportPlayer(
				player, self, linkedDoor, entrySide);
			return;
		}
	}
}
