#include "GameManager.h"
#include "LevelManager.h"
#include "../Scene/SceneManager.h"

void GameManager::StartGame()
{
	m_isGameStarted = true;
	m_isLevelComplete = false;
	m_isPaused = false;
	SceneManager::Instance().SetNextScene(SceneManager::SceneType::Game);
}

void GameManager::RestartLevel()
{
	m_isLevelComplete = false;
	m_isPaused = false;
	LevelManager::Instance().ResetLevel();
}

void GameManager::CompleteLevel()
{
	m_isLevelComplete = true;
}

void GameManager::LoadNextLevel()
{
	auto& level = LevelManager::Instance();
	constexpr int LevelCount = 5;
	const int nextLevelId =
		(level.GetCurrentLevelId() + 1) % LevelCount;
	level.SetCurrentLevelId(nextLevelId);
	m_isLevelComplete = false;
	m_isPaused = false;
	SceneManager::Instance().ReloadCurrentScene();
}

void GameManager::PauseGame()
{
	m_isPaused = true;
}

void GameManager::ResumeGame()
{
	m_isPaused = false;
}
