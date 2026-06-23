#include "LevelTransitionController.h"
#include "PlayerController2_5D.h"
#include "../GameSystem/GameManager.h"
#include "../main.h"

LevelTransitionController::TransitionState
	LevelTransitionController::s_state = TransitionState::Idle;
LevelTransitionController::TransitionAction
	LevelTransitionController::s_action = TransitionAction::None;
std::weak_ptr<PlayerController2_5D> LevelTransitionController::s_player;
float LevelTransitionController::s_fadeAlpha = 0.0f;

void LevelTransitionController::AttachPlayer(
	const std::shared_ptr<PlayerController2_5D>& player)
{
	s_player = player;
	if (player && s_state != TransitionState::Idle)
	{
		player->SetControlEnabled(false);
		if (s_state == TransitionState::WaitingForReload)
		{
			s_state = TransitionState::FadeIn;
			s_fadeAlpha = 1.0f;
		}
	}
}

void LevelTransitionController::Update()
{
	const float deltaSeconds = Application::Instance().GetDeltaSeconds();

	switch (s_state)
	{
	case TransitionState::FadeOut:
		s_fadeAlpha = std::min(
			1.0f,
			s_fadeAlpha + deltaSeconds / std::max(0.01f, m_fadeOutDuration));
		if (s_fadeAlpha >= 1.0f)
		{
			if (s_action == TransitionAction::StartGame)
			{
				s_state = TransitionState::WaitingForReload;
				GameManager::Instance().StartGame();
			}
			else if (s_action == TransitionAction::NextLevel)
			{
				s_state = TransitionState::WaitingForReload;
				GameManager::Instance().CompleteLevel();
				GameManager::Instance().LoadNextLevel();
			}
			else if (s_action == TransitionAction::RestartLevel)
			{
				GameManager::Instance().RestartLevel();
				s_state = TransitionState::FadeIn;
			}
		}
		break;

	case TransitionState::FadeIn:
		s_fadeAlpha = std::max(
			0.0f,
			s_fadeAlpha - deltaSeconds / std::max(0.01f, m_fadeInDuration));
		if (s_fadeAlpha <= 0.0f)
		{
			auto player = s_player.lock();
			if (player) { player->SetControlEnabled(true); }
			s_state = TransitionState::Idle;
			s_action = TransitionAction::None;
		}
		break;

	default:
		break;
	}
}

void LevelTransitionController::DrawSprite()
{
	if (s_fadeAlpha <= 0.0f) { return; }

	const auto backBuffer = KdDirect3D::Instance().GetBackBuffer();
	const int width = backBuffer ? backBuffer->GetInfo().Width : 1280;
	const int height = backBuffer ? backBuffer->GetInfo().Height : 720;
	const Math::Color fadeColor(0.0f, 0.0f, 0.0f, s_fadeAlpha);

	KdShaderManager::Instance().m_spriteShader.DrawBox(
		0, 0, width / 2 + 2, height / 2 + 2, &fadeColor, true);
}

bool LevelTransitionController::StartNextLevelTransition(
	const std::shared_ptr<PlayerController2_5D>& player)
{
	if (!player || s_state != TransitionState::Idle) { return false; }

	s_player = player;
	s_fadeAlpha = 0.0f;
	s_state = TransitionState::FadeOut;
	s_action = TransitionAction::NextLevel;
	player->SetControlEnabled(false);
	return true;
}

bool LevelTransitionController::StartRestartLevelTransition(
	const std::shared_ptr<PlayerController2_5D>& player)
{
	if (!player || s_state != TransitionState::Idle) { return false; }

	s_player = player;
	s_fadeAlpha = 0.0f;
	s_state = TransitionState::FadeOut;
	s_action = TransitionAction::RestartLevel;
	player->SetControlEnabled(false);
	return true;
}

bool LevelTransitionController::StartGameTransition()
{
	if (s_state != TransitionState::Idle) { return false; }

	s_player.reset();
	s_fadeAlpha = 0.0f;
	s_state = TransitionState::FadeOut;
	s_action = TransitionAction::StartGame;
	return true;
}

bool LevelTransitionController::IsTransitioning()
{
	return s_state != TransitionState::Idle;
}
