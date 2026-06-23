#pragma once

class PlayerController2_5D;

class LevelTransitionController : public KdGameObject
{
public:
	void AttachPlayer(const std::shared_ptr<PlayerController2_5D>& player);
	void Update() override;
	void DrawSprite() override;

	static bool StartNextLevelTransition(
		const std::shared_ptr<PlayerController2_5D>& player);
	static bool StartRestartLevelTransition(
		const std::shared_ptr<PlayerController2_5D>& player);
	static bool StartGameTransition();
	static bool IsTransitioning();

private:
	enum class TransitionState
	{
		Idle,
		FadeOut,
		WaitingForReload,
		FadeIn,
	};

	enum class TransitionAction
	{
		None,
		StartGame,
		NextLevel,
		RestartLevel,
	};

	static TransitionState s_state;
	static TransitionAction s_action;
	static std::weak_ptr<PlayerController2_5D> s_player;
	static float s_fadeAlpha;

	float m_fadeOutDuration = 0.55f;
	float m_fadeInDuration = 0.55f;
};
