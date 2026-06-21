#pragma once

class ColorBall;

struct AreaCameraSetting
{
	Math::Vector3 position = { 0.0f, 0.0f, -20.0f };
	Math::Vector3 target = Math::Vector3::Zero;
	float fieldOfView = 60.0f;
};

class MapArea : public KdGameObject
{
public:
	void SetAreaId(int id) { m_areaId = id; }
	int GetAreaId() const { return m_areaId; }

	void SetAreaName(std::string_view name) { m_areaName = name; }
	const std::string& GetAreaName() const { return m_areaName; }

	void SetGameplayZ(float z) { m_gameplayZ = z; }
	float GetGameplayZ() const { return m_gameplayZ; }

	void SetCameraSetting(const AreaCameraSetting& setting) { m_cameraSetting = setting; }
	const AreaCameraSetting& GetCameraSetting() const { return m_cameraSetting; }

	void SetCameraBounds(const DirectX::BoundingBox& bounds) { m_cameraBounds = bounds; }
	const DirectX::BoundingBox& GetCameraBounds() const { return m_cameraBounds; }

	void SetAreaRoot(const std::shared_ptr<KdGameObject>& root) { m_areaRoot = root; }
	std::shared_ptr<KdGameObject> GetAreaRoot() const { return m_areaRoot.lock(); }

	void RegisterDoor(const std::shared_ptr<KdGameObject>& door);
	void RegisterBall(const std::shared_ptr<ColorBall>& ball);

	const std::vector<std::weak_ptr<KdGameObject>>& GetDoors() const { return m_doorsInArea; }
	const std::vector<std::weak_ptr<ColorBall>>& GetBalls() const { return m_ballsInArea; }

	void SetCurrentArea(bool current) { m_isCurrentArea = current; }
	bool IsCurrentArea() const { return m_isCurrentArea; }

private:
	int m_areaId = 0;
	std::string m_areaName;
	float m_gameplayZ = 0.0f;
	AreaCameraSetting m_cameraSetting;
	DirectX::BoundingBox m_cameraBounds;
	std::weak_ptr<KdGameObject> m_areaRoot;
	std::vector<std::weak_ptr<KdGameObject>> m_doorsInArea;
	std::vector<std::weak_ptr<ColorBall>> m_ballsInArea;
	bool m_isCurrentArea = false;
};
