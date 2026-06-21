#pragma once

class GameManager
{
public:
	static GameManager& Instance()
	{
		static GameManager instance;
		return instance;
	}

	void StartGame();
	void RestartLevel();
	void CompleteLevel();
	void LoadNextLevel();
	void PauseGame();
	void ResumeGame();

	bool IsGameStarted() const { return m_isGameStarted; }
	bool IsLevelComplete() const { return m_isLevelComplete; }
	bool IsPaused() const { return m_isPaused; }

private:
	GameManager() = default;

	bool m_isGameStarted = false;
	bool m_isLevelComplete = false;
	bool m_isPaused = false;
};
