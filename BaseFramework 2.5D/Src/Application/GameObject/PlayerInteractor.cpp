#include "PlayerInteractor.h"
#include "ColorBall.h"
#include "BallStorageDevice.h"
#include "PlayerController2_5D.h"
#include "PlayerCarrySlot.h"
#include "../Scene/SceneManager.h"

void PlayerInteractor::DetectInteractable()
{
	m_currentInteractable.reset();
	m_currentStorageDevice.reset();
	if (!m_owner) { return; }

	float nearestDistanceSqr = m_interactRange * m_interactRange;
	for (auto& object : SceneManager::Instance().GetObjList())
	{
		auto storage = std::dynamic_pointer_cast<BallStorageDevice>(object);
		if (storage)
		{
			const float distanceSqr =
				(storage->GetPos() - m_owner->GetPos()).LengthSquared();
			if (distanceSqr <= nearestDistanceSqr)
			{
				nearestDistanceSqr = distanceSqr;
				m_currentStorageDevice = storage;
				m_currentInteractable.reset();
			}
			continue;
		}

		auto ball = std::dynamic_pointer_cast<ColorBall>(object);
		if (!ball || ball->IsHeld() || ball->IsStored() || ball->IsConsumed()) { continue; }

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
	auto storage = m_currentStorageDevice.lock();
	auto player = dynamic_cast<PlayerController2_5D*>(m_owner);
	if (storage && player)
	{
		storage->Interact(*player);
		return;
	}

	auto ball = m_currentInteractable.lock();
	if (ball && m_carrySlot) { m_carrySlot->PickUpBall(ball); }
}
