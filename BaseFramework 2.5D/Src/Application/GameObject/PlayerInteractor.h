#pragma once

class ColorBall;
class BallStorageDevice;
class PlayerCarrySlot;

class PlayerInteractor
{
public:
	void SetOwner(KdGameObject* owner) { m_owner = owner; }
	void SetCarrySlot(PlayerCarrySlot* slot) { m_carrySlot = slot; }
	void SetInteractRange(float range) { m_interactRange = range; }

	void DetectInteractable();
	void Interact();
	std::shared_ptr<ColorBall> GetCurrentInteractable() const { return m_currentInteractable.lock(); }

private:
	float m_interactRange = 1.5f;
	std::weak_ptr<ColorBall> m_currentInteractable;
	std::weak_ptr<BallStorageDevice> m_currentStorageDevice;
	KdGameObject* m_owner = nullptr;
	PlayerCarrySlot* m_carrySlot = nullptr;
};
