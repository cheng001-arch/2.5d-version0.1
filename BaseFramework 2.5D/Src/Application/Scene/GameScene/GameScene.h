#pragma once

#include"../BaseScene/BaseScene.h"

class PlayerController2_5D;

class GameScene : public BaseScene
{
public :

	GameScene()  { Init(); }
	~GameScene() {}

private:

	void Event() override;
	void Init()  override;
	void UpdateFollowCamera();

	std::weak_ptr<PlayerController2_5D> m_player;
	Math::Vector3 m_cameraTarget = Math::Vector3::Zero;
};
