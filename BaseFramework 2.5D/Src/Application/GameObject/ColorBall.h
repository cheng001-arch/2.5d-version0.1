#pragma once

#include "../GameSystem/GameColor.h"

class MapArea;

class ColorBall : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void GenerateDepthMapFromLight() override;
	void DrawLit() override;

	void SetBallColor(GameColor color);
	GameColor GetBallColor() const { return m_ballColor; }

	void SetOwnerArea(const std::shared_ptr<MapArea>& area) { m_ownerArea = area; }
	std::shared_ptr<MapArea> GetOwnerArea() const { return m_ownerArea.lock(); }

	void SaveInitialState();
	void OnPickedUp();
	void OnThrown(const Math::Vector3& direction, float speed, float maxDistance);
	void OnThrownArc(const Math::Vector3& launchVelocity, float gravity, float maxDistance);
	void OnHitPortalDoor(const std::shared_ptr<KdGameObject>& targetDoor);
	void ResetState();

	bool IsHeld() const { return m_isHeld; }
	bool IsFlying() const { return m_isFlying; }
	bool IsConsumed() const { return m_isConsumed; }
	bool IsModelLoaded() const { return m_model != nullptr; }
	void SetHeldPosition(const Math::Vector3& position);
	void DropAt(const Math::Vector3& position);

private:
	std::shared_ptr<KdModelData> m_model;
	GameColor m_ballColor = GameColor::None;
	std::weak_ptr<MapArea> m_ownerArea;
	bool m_isHeld = false;
	bool m_isFlying = false;
	bool m_isConsumed = false;
	Math::Vector3 m_initialPosition = Math::Vector3::Zero;
	std::weak_ptr<MapArea> m_initialArea;
	Math::Vector3 m_flyDirection = Math::Vector3::Zero;
	Math::Vector3 m_flyVelocity = Math::Vector3::Zero;
	Math::Vector3 m_throwStartPosition = Math::Vector3::Zero;
	Math::Vector3 m_heldTargetPosition = Math::Vector3::Zero;
	Math::Vector3 m_carryVelocity = Math::Vector3::Zero;
	float m_throwSpeed = 12.0f;
	float m_flightGravity = 18.0f;
	float m_maxThrowDistance = 8.0f;
	float m_flightTime = 0.0f;
	float m_collisionRadius = 0.633f;
	float m_carrySpring = 42.0f;
	float m_carryDamping = 9.0f;
};
