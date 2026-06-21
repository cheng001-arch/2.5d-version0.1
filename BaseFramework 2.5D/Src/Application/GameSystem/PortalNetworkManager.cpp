#include "PortalNetworkManager.h"
#include "../GameObject/PortalDoor.h"

void PortalNetworkManager::RegisterDoor(const std::shared_ptr<PortalDoor>& door)
{
	if (!door) { return; }

	for (auto it = m_allDoors.begin(); it != m_allDoors.end();)
	{
		auto registeredDoor = it->lock();
		if (!registeredDoor)
		{
			it = m_allDoors.erase(it);
			continue;
		}
		if (registeredDoor == door) { return; }
		++it;
	}

	m_allDoors.emplace_back(door);
}

void PortalNetworkManager::ClearDoors()
{
	m_allDoors.clear();
}

std::shared_ptr<PortalDoor> PortalNetworkManager::GetLinkedDoor(
	const std::shared_ptr<PortalDoor>& sourceDoor) const
{
	if (!sourceDoor ||
		!sourceDoor->IsActivated() ||
		sourceDoor->GetCurrentColor() == GameColor::None)
	{
		return nullptr;
	}

	auto sourceArea = sourceDoor->GetOwnerArea();
	if (!sourceArea) { return nullptr; }

	std::vector<std::shared_ptr<PortalDoor>> matchingDoors;
	for (const auto& weakDoor : m_allDoors)
	{
		auto door = weakDoor.lock();
		if (!door ||
			!door->IsActivated() ||
			door->GetCurrentColor() != sourceDoor->GetCurrentColor() ||
			door->GetOwnerArea() != sourceArea)
		{
			continue;
		}
		matchingDoors.emplace_back(door);
	}

	// A color is linked only when exactly two active doors exist in this Area.
	if (matchingDoors.size() != 2) { return nullptr; }
	return matchingDoors[0] == sourceDoor ? matchingDoors[1] : matchingDoors[0];
}
