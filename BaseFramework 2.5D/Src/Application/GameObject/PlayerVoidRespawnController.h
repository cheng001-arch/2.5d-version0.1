#pragma once

class PlayerController2_5D;

class PlayerVoidRespawnController : public KdGameObject
{
public:
	void SetPlayer(const std::shared_ptr<PlayerController2_5D>& player) { m_player = player; }
	void Update() override;
	void DrawSprite() override;

private:
	enum class FadeState
	{
		Idle,
		FadeOut,
		FadeIn,
	};

	void RespawnPlayer();

	std::weak_ptr<PlayerController2_5D> m_player;
	FadeState m_fadeState = FadeState::Idle;
	float m_fadeAlpha = 0.0f;
	float m_fadeOutDuration = 0.45f;
	float m_fadeInDuration = 0.45f;
};
