#pragma once

class MapArea;

class AreaManager
{
public:
	static AreaManager& Instance()
	{
		static AreaManager instance;
		return instance;
	}

	void RegisterArea(const std::shared_ptr<MapArea>& area);
	void ClearAreas();
	void SwitchToArea(const std::shared_ptr<MapArea>& targetArea);
	float GetCurrentGameplayZ() const;
	void EnableAreaInteraction(const std::shared_ptr<MapArea>& area);
	void DisableOtherAreaInteraction();

	std::shared_ptr<MapArea> GetCurrentArea() const { return m_currentArea.lock(); }
	const std::vector<std::weak_ptr<MapArea>>& GetAllAreas() const { return m_allAreas; }

private:
	AreaManager() = default;

	std::weak_ptr<MapArea> m_currentArea;
	std::vector<std::weak_ptr<MapArea>> m_allAreas;
};
