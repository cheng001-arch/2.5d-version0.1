#include "PauseMenuUI.h"
#include "../GameSystem/GameManager.h"
#include "../Scene/SceneManager.h"
#include "../main.h"

void PauseMenuUI::Init()
{
	m_title = std::make_shared<KdTexture>("Asset/Textures/title/title.png");
	m_mainButton.Load("Asset/Textures/title/main.png");
	m_exitButton.Load("Asset/Textures/title/exit.png");
	m_mainButton.SetLayout({ 0.0f, -15.0f }, { 360.0f, 150.0f });
	m_exitButton.SetLayout({ 0.0f, -185.0f }, { 360.0f, 120.0f });
}

void PauseMenuUI::Update()
{
	if (!m_isOpen) { return; }

	const Math::Vector2 mousePosition = GetMousePosition();
	const bool mousePressed = (GetAsyncKeyState(VK_LBUTTON) & 0x0001) != 0;
	const float deltaSeconds = Application::Instance().GetDeltaSeconds();

	if (m_mainButton.Update(mousePosition, mousePressed, deltaSeconds))
	{
		m_isOpen = false;
		GameManager::Instance().ResumeGame();
		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Title);
	}

	if (m_exitButton.Update(mousePosition, mousePressed, deltaSeconds))
	{
		Application::Instance().End();
	}
}

void PauseMenuUI::DrawSprite()
{
	if (!m_isOpen) { return; }

	auto& spriteShader = KdShaderManager::Instance().m_spriteShader;
	const Math::Color overlayColor(0.02f, 0.01f, 0.07f, 0.78f);
	spriteShader.DrawBox(0, 0, 642, 362, &overlayColor, true);
	if (m_title) { spriteShader.DrawTex(m_title.get(), 0, 205, 760, 253); }
	m_mainButton.Draw();
	m_exitButton.Draw();
}

Math::Vector2 PauseMenuUI::GetMousePosition() const
{
	POINT mousePosition = {};
	RECT clientRect = {};
	const HWND windowHandle = Application::Instance().GetWindowHandle();
	if (!windowHandle ||
		!GetCursorPos(&mousePosition) ||
		!ScreenToClient(windowHandle, &mousePosition) ||
		!GetClientRect(windowHandle, &clientRect))
	{
		return { FLT_MAX, FLT_MAX };
	}

	const float width = static_cast<float>(clientRect.right - clientRect.left);
	const float height = static_cast<float>(clientRect.bottom - clientRect.top);
	return { mousePosition.x - width * 0.5f, height * 0.5f - mousePosition.y };
}
