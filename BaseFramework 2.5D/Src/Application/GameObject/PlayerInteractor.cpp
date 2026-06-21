#include "PlayerInteractor.h"
#include "ColorBall.h"
#include "PlayerCarrySlot.h"
#include "../Scene/SceneManager.h"

void PlayerInteractor::DetectInteractable()
{
	m_currentInteractable.reset();
	if (!m_owner) { return; }

	float nearestDistanceSqr = m_interactRange * m_interactRange;
	for (auto& object : SceneManager::Instance().GetObjList())
	{
		auto ball = std::dynamic_pointer_cast<ColorBall>(object);
		if (!ball || ball->IsHeld() || ball->IsConsumed()) { continue; }

		const float distanceSqr = (ball->GetPos() - m_owner->GetPos()).LengthSquared();
		if (distanceSqr <= nearestDistanceSqr)
		{
			nearestDistanceSqr = distanceSqr;
			m_currentInteractable = ball;
		}
	}
}

void PlayerInteractor::Interact()
{
	auto ball = m_currentInteractable.lock();
	if (ball && m_carrySlot) { m_carrySlot->PickUpBall(ball); }
}
