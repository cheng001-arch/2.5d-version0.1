#include "PlayerVoidRespawnController.h"
#include "PlayerController2_5D.h"
#include "../GameSystem/AreaManager.h"
#include "../GameSystem/LevelManager.h"
#include "../main.h"

void PlayerVoidRespawnController::Update()
{
	auto player = m_player.lock();
	if (!player) { return; }

	const float deltaSeconds = Application::Instance().GetDeltaSeconds();
	auto& levelManager = LevelManager::Instance();

	switch (m_fadeState)
	{
	case FadeState::Idle:
		if (player->GetPos().y < levelManager.GetVoidY())
		{
			player->SetControlEnabled(false);
			m_fadeState = FadeState::FadeOut;
			m_fadeAlpha = 0.0f;
		}
		break;

	case FadeState::FadeOut:
		m_fadeAlpha = std::min(
			1.0f,
			m_fadeAlpha + deltaSeconds / std::max(0.01f, m_fadeOutDuration));
		if (m_fadeAlpha >= 1.0f)
		{
			RespawnPlayer();
			m_fadeState = FadeState::FadeIn;
		}
		break;

	case FadeState::FadeIn:
		m_fadeAlpha = std::max(
			0.0f,
			m_fadeAlpha - deltaSeconds / std::max(0.01f, m_fadeInDuration));
		if (m_fadeAlpha <= 0.0f)
		{
			player->SetControlEnabled(true);
			m_fadeState = FadeState::Idle;
		}
		break;
	}
}

void PlayerVoidRespawnController::DrawSprite()
{
	if (m_fadeAlpha <= 0.0f) { return; }

	const auto backBuffer = KdDirect3D::Instance().GetBackBuffer();
	const int width = backBuffer ? backBuffer->GetInfo().Width : 1280;
	const int height = backBuffer ? backBuffer->GetInfo().Height : 720;
	const Math::Color fadeColor(0.0f, 0.0f, 0.0f, m_fadeAlpha);

	KdShaderManager::Instance().m_spriteShader.DrawBox(
		0, 0, width / 2 + 2, height / 2 + 2, &fadeColor, true);
}

void PlayerVoidRespawnController::RespawnPlayer()
{
	auto player = m_player.lock();
	if (!player) { return; }

	auto& levelManager = LevelManager::Instance();
	std::shared_ptr<MapArea> spawnArea;
	if (!levelManager.GetAllAreas().empty())
	{
		spawnArea = levelManager.GetAllAreas().front().lock();
	}

	if (spawnArea)
	{
		AreaManager::Instance().SwitchToArea(spawnArea);
		player->SetCurrentArea(spawnArea);
	}
	player->ApplyTeleport(levelManager.GetPlayerSpawnPoint(), 1.0f);
}
