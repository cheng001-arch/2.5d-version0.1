#pragma once

#include "MenuButton.h"

class PauseMenuUI : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

	void SetOpen(bool open) { m_isOpen = open; }
	bool IsOpen() const { return m_isOpen; }

private:
	Math::Vector2 GetMousePosition() const;

	std::shared_ptr<KdTexture> m_title;
	MenuButton m_mainButton;
	MenuButton m_exitButton;
	bool m_isOpen = false;
};
