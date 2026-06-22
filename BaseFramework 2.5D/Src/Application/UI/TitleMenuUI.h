#pragma once

#include "MenuButton.h"

class TitleMenuUI : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

private:
	Math::Vector2 GetMousePosition() const;

	std::shared_ptr<KdTexture> m_background;
	std::shared_ptr<KdTexture> m_title;
	MenuButton m_startButton;
	MenuButton m_settingButton;
	MenuButton m_exitButton;
};
