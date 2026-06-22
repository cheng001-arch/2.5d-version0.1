#pragma once

#include"../BaseScene/BaseScene.h"

class PlayerController2_5D;
class PauseMenuUI;

class GameScene : public BaseScene
{
public :

	GameScene()  { Init(); }
	~GameScene() {}

private:

	void Event() override;
	void Init()  override;
	void PreDraw() override;
	void UpdateFollowCamera();

	std::weak_ptr<PlayerController2_5D> m_player;
	std::shared_ptr<PauseMenuUI> m_pauseMenu;
	Math::Vector3 m_cameraTarget = Math::Vector3::Zero;
	Math::Vector3 m_mouseCameraOffset = Math::Vector3::Zero;
	std::shared_ptr<KdTexture> m_levelBackground;
	float m_backgroundOffsetX = 0.0f;
	float m_backgroundOriginPlayerX = 0.0f;
};
