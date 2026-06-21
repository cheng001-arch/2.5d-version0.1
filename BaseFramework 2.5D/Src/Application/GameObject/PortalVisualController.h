#pragma once

#include "../GameSystem/GameColor.h"

class PortalVisualController
{
public:
	void SetModel(const std::shared_ptr<KdModelData>& model) { m_model = model; }
	void UpdateDoorColorVisual(GameColor color, bool isActivated);

private:
	std::weak_ptr<KdModelData> m_model;
};
