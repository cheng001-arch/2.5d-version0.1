#pragma once

class ColorBall;
class MapArea;

class LevelManager
{
public:
	static LevelManager& Instance()
	{
		static LevelManager instance;
		return instance;
	}

	void SetCurrentLevelId(int id) { m_currentLevelId = id; }
	int GetCurrentLevelId() const { return m_currentLevelId; }

	void SetPlayerSpawnPoint(const Math::Vector3& point) { m_playerSpawnPoint = point; }
	const Math::Vector3& GetPlayerSpawnPoint() const { return m_playerSpawnPoint; }
	void SetPlayer(const std::shared_ptr<KdGameObject>& player) { m_player = player; }

	void SetVoidBaseY(float y) { m_voidBaseY = y; }
	float GetVoidBaseY() const { return m_voidBaseY; }
	void SetVoidFallDistance(float distance) { m_voidFallDistance = std::max(0.0f, distance); }
	float GetVoidFallDistance() const { return m_voidFallDistance; }
	float GetVoidY() const { return m_voidBaseY - m_voidFallDistance; }

	void SetGoalDoor(const std::shared_ptr<KdGameObject>& door) { m_goalDoor = door; }
	std::shared_ptr<KdGameObject> GetGoalDoor() const { return m_goalDoor.lock(); }

	void InitializeLevel();
	void ResetLevel();
	void ClearLevel();
	void RegisterDoor(const std::shared_ptr<KdGameObject>& door);
	void RegisterBall(const std::shared_ptr<ColorBall>& ball);
	void RegisterArea(const std::shared_ptr<MapArea>& area);

	const std::vector<std::weak_ptr<MapArea>>& GetAllAreas() const { return m_allAreas; }
	const std::vector<std::weak_ptr<KdGameObject>>& GetAllDoors() const { return m_allDoors; }
	const std::vector<std::weak_ptr<ColorBall>>& GetAllBalls() const { return m_allBalls; }

private:
	LevelManager() = default;

	int m_currentLevelId = 0;
	Math::Vector3 m_playerSpawnPoint = Math::Vector3::Zero;
	std::vector<std::weak_ptr<MapArea>> m_allAreas;
	std::vector<std::weak_ptr<KdGameObject>> m_allDoors;
	std::vector<std::weak_ptr<ColorBall>> m_allBalls;
	std::weak_ptr<KdGameObject> m_goalDoor;
	std::weak_ptr<KdGameObject> m_player;
	float m_voidBaseY = 0.0f;
	float m_voidFallDistance = 8.0f;
};
