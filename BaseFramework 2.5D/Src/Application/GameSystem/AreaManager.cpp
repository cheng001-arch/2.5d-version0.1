#include "AreaManager.h"
#include "MapArea.h"

void AreaManager::RegisterArea(const std::shared_ptr<MapArea>& area)
{
	if (!area) { return; }
	m_allAreas.emplace_back(area);
}

void AreaManager::ClearAreas()
{
	m_currentArea.reset();
	m_allAreas.clear();
}

void AreaManager::SwitchToArea(const std::shared_ptr<MapArea>& targetArea)
{
	if (!targetArea) { return; }
	m_currentArea = targetArea;
	DisableOtherAreaInteraction();
	EnableAreaInteraction(targetArea);
}

float AreaManager::GetCurrentGameplayZ() const
{
	auto area = m_currentArea.lock();
	return area ? area->GetGameplayZ() : 0.0f;
}

void AreaManager::EnableAreaInteraction(const std::shared_ptr<MapArea>& area)
{
	if (area) { area->SetCurrentArea(true); }
}

void AreaManager::DisableOtherAreaInteraction()
{
	auto current = m_currentArea.lock();
	for (auto& weakArea : m_allAreas)
	{
		auto area = weakArea.lock();
		if (area) { area->SetCurrentArea(area == current); }
	}
}
