#include "RainbowGoalDoor.h"
#include "LevelTransitionController.h"
#include "PlayerController2_5D.h"
#include "../Scene/SceneManager.h"
#include "../main.h"

void RainbowGoalDoor::Init()
{
	SetModelAssetPath("Asset/Data/door2/door2.gltf");
	PortalDoor::Init();
}

void RainbowGoalDoor::Update()
{
	const float deltaSeconds = Application::Instance().GetDeltaSeconds();
	m_visualTime += deltaSeconds;
	m_triggerCooldown = std::max(0.0f, m_triggerCooldown - deltaSeconds);

	if (!IsActivated()) { return; }

	const auto rainbowChannel = [this](float phase)
	{
		return 0.55f + 0.45f * std::sin(m_visualTime * 2.8f + phase);
	};
	SetPortalSurfaceVisualColor({
		rainbowChannel(0.0f),
		rainbowChannel(2.094395f),
		rainbowChannel(4.188790f),
		1.0f
	});

	if (m_triggerCooldown > 0.0f) { return; }

	const Math::Vector3 center = GetCenterPosition();
	for (auto& object : SceneManager::Instance().GetObjList())
	{
		auto player = std::dynamic_pointer_cast<PlayerController2_5D>(object);
		if (!player) { continue; }

		const Math::Vector3 position = player->GetPos();
		if (std::abs(position.x - center.x) <= m_triggerHalfWidth &&
			position.y >= center.y - 1.25f &&
			position.y <= center.y + 1.25f &&
			std::abs(position.z - center.z) <= 1.45f)
		{
			TriggerGoal(player);
			return;
		}
	}
}

bool RainbowGoalDoor::OnBallHit(GameColor ballColor)
{
	if (ballColor != GameColor::Rainbow || !CanBeActivated()) { return false; }
	return SetColor(GameColor::Rainbow);
}

void RainbowGoalDoor::TriggerGoal(
	const std::shared_ptr<PlayerController2_5D>& player)
{
	if (!player) { return; }
	if (LevelTransitionController::StartNextLevelTransition(player))
	{
		m_triggerCooldown = 1.0f;
	}
}
