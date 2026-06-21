#pragma once

class PortalDoor;

class PortalNetworkManager
{
public:
	static PortalNetworkManager& Instance()
	{
		static PortalNetworkManager instance;
		return instance;
	}

	void RegisterDoor(const std::shared_ptr<PortalDoor>& door);
	void ClearDoors();
	std::shared_ptr<PortalDoor> GetLinkedDoor(
		const std::shared_ptr<PortalDoor>& sourceDoor) const;

	const std::vector<std::weak_ptr<PortalDoor>>& GetAllDoors() const { return m_allDoors; }

private:
	PortalNetworkManager() = default;

	std::vector<std::weak_ptr<PortalDoor>> m_allDoors;
};
