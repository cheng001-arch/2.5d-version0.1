#include "LevelManager.h"
#include "AreaManager.h"
#include "MapArea.h"
#include "PortalNetworkManager.h"
#include "../GameObject/ColorBall.h"
#include "../GameObject/PortalDoor.h"

void LevelManager::InitializeLevel()
{
	AreaManager::Instance().ClearAreas();
	for (auto& weakArea : m_allAreas)
	{
		auto area = weakArea.lock();
		if (area) { AreaManager::Instance().RegisterArea(area); }
	}

	if (!m_allAreas.empty())
	{
		AreaManager::Instance().SwitchToArea(m_allAreas.front().lock());
	}
}

void LevelManager::ResetLevel()
{
	for (auto& weakBall : m_allBalls)
	{
		auto ball = weakBall.lock();
		if (ball) { ball->ResetState(); }
	}

	for (auto& weakDoor : m_allDoors)
	{
		auto door = std::dynamic_pointer_cast<PortalDoor>(weakDoor.lock());
		if (door) { door->ResetToInitialState(); }
	}

	auto player = m_player.lock();
	if (player) { player->SetPos(m_playerSpawnPoint); }

	InitializeLevel();
}

void LevelManager::ClearLevel()
{
	m_allAreas.clear();
	m_allDoors.clear();
	m_allBalls.clear();
	m_goalDoor.reset();
	m_player.reset();
	AreaManager::Instance().ClearAreas();
	PortalNetworkManager::Instance().ClearDoors();
}

void LevelManager::RegisterDoor(const std::shared_ptr<KdGameObject>& door)
{
	if (!door) { return; }
	m_allDoors.emplace_back(door);

	auto portalDoor = std::dynamic_pointer_cast<PortalDoor>(door);
	if (portalDoor) { PortalNetworkManager::Instance().RegisterDoor(portalDoor); }
}

void LevelManager::RegisterBall(const std::shared_ptr<ColorBall>& ball)
{
	if (ball) { m_allBalls.emplace_back(ball); }
}

void LevelManager::RegisterArea(const std::shared_ptr<MapArea>& area)
{
	if (area) { m_allAreas.emplace_back(area); }
}
