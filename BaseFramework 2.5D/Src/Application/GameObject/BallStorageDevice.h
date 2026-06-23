#pragma once

class ColorBall;
class PlayerController2_5D;
class MapArea;

class BallStorageDevice : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void GenerateDepthMapFromLight() override;
	void DrawLit() override;

	void SetPlacement(const Math::Vector3& position);
	void SetOwnerArea(const std::shared_ptr<MapArea>& area) { m_ownerArea = area; }
	std::shared_ptr<MapArea> GetOwnerArea() const { return m_ownerArea.lock(); }

	void Interact(PlayerController2_5D& player);
	bool HasStoredBall() const { return !m_storedBall.expired(); }

private:
	std::shared_ptr<KdModelData> m_model;
	std::weak_ptr<MapArea> m_ownerArea;
	std::weak_ptr<ColorBall> m_storedBall;
	Math::Vector3 m_centerPosition = Math::Vector3::Zero;
	Math::Vector3 m_ballRestPosition = Math::Vector3::Zero;
	float m_modelScale = 2.2f;
};
