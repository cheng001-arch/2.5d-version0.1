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
	level.SetCurrentLevelId(level.GetCurrentLevelId() + 1);
	m_isLevelComplete = false;
	m_isPaused = false;
	SceneManager::Instance().SetNextScene(SceneManager::SceneType::Game);
}

void GameManager::PauseGame()
{
	m_isPaused = true;
}

void GameManager::ResumeGame()
{
	m_isPaused = false;
}
