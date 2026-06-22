#include "MenuButton.h"

#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

bool MenuButton::Load(std::string_view texturePath)
{
	m_texture = std::make_shared<KdTexture>();
	return m_texture->Load(texturePath);
}

void MenuButton::SetLayout(
	const Math::Vector2& center, const Math::Vector2& size)
{
	m_center = center;
	m_size = size;
}

bool MenuButton::Update(
	const Math::Vector2& mousePosition, bool mousePressed, float deltaSeconds)
{
	const bool wasHovered = m_isHovered;
	m_isHovered =
		std::abs(mousePosition.x - m_center.x) <= m_size.x * 0.42f &&
		std::abs(mousePosition.y - m_center.y) <= m_size.y * 0.36f;

	if (m_isHovered && !wasHovered)
	{
		m_animationTime = 0.0f;
		m_isAnimating = true;
		PlayHoverSound();
	}

	if (m_isAnimating)
	{
		m_animationTime += deltaSeconds;
		if (m_animationTime >= 0.22f)
		{
			m_animationTime = 0.22f;
			m_isAnimating = false;
		}
	}

	return m_isHovered && mousePressed;
}

void MenuButton::Draw() const
{
	if (!m_texture) { return; }

	Math::Vector2 drawCenter = m_center;
	if (m_isAnimating)
	{
		const float progress = std::clamp(m_animationTime / 0.22f, 0.0f, 1.0f);
		drawCenter.y += std::sin(progress * DirectX::XM_PI) * 5.0f;
	}

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_texture.get(),
		static_cast<int>(drawCenter.x),
		static_cast<int>(drawCenter.y),
		static_cast<int>(m_size.x),
		static_cast<int>(m_size.y));
}

void MenuButton::PlayHoverSound()
{
	mciSendStringA("close menu_hover", nullptr, 0, nullptr);
	mciSendStringA(
		"open \"Asset/Textures/sound/African3.mp3\" type mpegvideo alias menu_hover",
		nullptr, 0, nullptr);
	mciSendStringA("play menu_hover from 0", nullptr, 0, nullptr);
}
