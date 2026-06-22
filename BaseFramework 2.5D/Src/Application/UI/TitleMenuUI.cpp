#include "TitleMenuUI.h"
#include "../GameSystem/GameManager.h"
#include "../GameSystem/LevelManager.h"
#include "../main.h"

void TitleMenuUI::Init()
{
	m_background = std::make_shared<KdTexture>("Asset/Textures/title/beijing.png");
	m_title = std::make_shared<KdTexture>("Asset/Textures/title/title.png");
	m_startButton.Load("Asset/Textures/title/start.png");
	m_settingButton.Load("Asset/Textures/title/setting.png");
	m_exitButton.Load("Asset/Textures/title/exit.png");

	m_startButton.SetLayout({ 0.0f, 20.0f }, { 330.0f, 150.0f });
	m_settingButton.SetLayout({ 0.0f, -125.0f }, { 360.0f, 120.0f });
	m_exitButton.SetLayout({ 0.0f, -255.0f }, { 360.0f, 120.0f });
}

void TitleMenuUI::Update()
{
	const Math::Vector2 mousePosition = GetMousePosition();
	const bool mousePressed = (GetAsyncKeyState(VK_LBUTTON) & 0x0001) != 0;
	const float deltaSeconds = Application::Instance().GetDeltaSeconds();

	if (m_startButton.Update(mousePosition, mousePressed, deltaSeconds))
	{
		LevelManager::Instance().SetCurrentLevelId(0);
		GameManager::Instance().StartGame();
	}

	m_settingButton.Update(mousePosition, mousePressed, deltaSeconds);

	if (m_exitButton.Update(mousePosition, mousePressed, deltaSeconds))
	{
		Application::Instance().End();
	}
}

void TitleMenuUI::DrawSprite()
{
	auto& spriteShader = KdShaderManager::Instance().m_spriteShader;
	if (m_background) { spriteShader.DrawTex(m_background.get(), 0, 0, 1280, 720); }
	if (m_title) { spriteShader.DrawTex(m_title.get(), 0, 235, 820, 273); }
	m_startButton.Draw();
	m_settingButton.Draw();
	m_exitButton.Draw();
}

Math::Vector2 TitleMenuUI::GetMousePosition() const
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
