#include "MapArea.h"

void MapArea::RegisterDoor(const std::shared_ptr<KdGameObject>& door)
{
	if (!door) { return; }
	m_doorsInArea.emplace_back(door);
}

void MapArea::RegisterBall(const std::shared_ptr<ColorBall>& ball)
{
	if (!ball) { return; }
	m_ballsInArea.emplace_back(ball);
}
