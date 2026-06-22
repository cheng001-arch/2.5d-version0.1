#pragma once

class MenuButton
{
public:
	bool Load(std::string_view texturePath);
	void SetLayout(const Math::Vector2& center, const Math::Vector2& size);
	bool Update(const Math::Vector2& mousePosition, bool mousePressed, float deltaSeconds);
	void Draw() const;

private:
	void PlayHoverSound();

	std::shared_ptr<KdTexture> m_texture;
	Math::Vector2 m_center = Math::Vector2::Zero;
	Math::Vector2 m_size = { 320.0f, 100.0f };
	float m_animationTime = 0.0f;
	bool m_isAnimating = false;
	bool m_isHovered = false;
};
