#pragma once

#include "PortalDoor.h"

class RainbowGoalDoor : public PortalDoor
{
public:
	void Init() override;
	void Update() override;
	bool OnBallHit(GameColor ballColor) override;

private:
	void TriggerGoal(const std::shared_ptr<PlayerController2_5D>& player);

	float m_visualTime = 0.0f;
	float m_triggerCooldown = 0.0f;
	float m_triggerHalfWidth = 0.8f;
};
