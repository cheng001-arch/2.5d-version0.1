#pragma once

#include "../GameSystem/GameColor.h"
#include "DoorExitPoint.h"
#include "PortalVisualController.h"

class MapArea;
class PlayerController2_5D;

class PortalDoor : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void GenerateDepthMapFromLight() override;
	void DrawLit() override;
	void DrawUnLit() override;
	void SetModelAssetPath(std::string_view assetPath)
	{
		m_modelAssetPath = assetPath;
	}

	// The imported door is kept parallel to the ZY plane. This places its
	// visible center at the requested gameplay position.
	void SetPlacement(const Math::Vector3& centerPosition);
	void SetOwnerArea(const std::shared_ptr<MapArea>& area) { m_ownerArea = area; }
	std::shared_ptr<MapArea> GetOwnerArea() const { return m_ownerArea.lock(); }

	void SetDoorId(std::string_view doorId) { m_doorId = doorId; }
	const std::string& GetDoorId() const { return m_doorId; }

	void SetExitPoint(const Math::Vector3& position, float facingDirectionAfterExit);
	const DoorExitPoint& GetExitPoint() const { return m_exitPoint; }
	Math::Vector3 GetExitPosition(float side) const;
	void SetExitDistance(float distance) { m_exitDistance = std::max(0.5f, distance); }
	void SetAllowedEntrySide(float side)
	{
		m_allowedEntrySide = side == 0.0f ? 0.0f : (side < 0.0f ? -1.0f : 1.0f);
	}
	void SetForcedExitSide(float side)
	{
		m_forcedExitSide = side == 0.0f ? 0.0f : (side < 0.0f ? -1.0f : 1.0f);
	}
	float GetForcedExitSide() const { return m_forcedExitSide; }

	void SetTeleportCooldownDuration(float seconds)
	{
		m_teleportCooldownDuration = std::max(0.0f, seconds);
	}
	float GetTeleportCooldownDuration() const { return m_teleportCooldownDuration; }
	void BeginTeleportCooldown(float seconds);
	bool IsTeleportCoolingDown() const { return m_teleportCooldownRemaining > 0.0f; }

	bool SetColor(GameColor color);
	virtual bool OnBallHit(GameColor ballColor);
	void ResetToInitialState();

	void SetInitialColor(GameColor color);
	void SetCanBeColored(bool canBeColored) { m_canBeColored = canBeColored; }
	void SetFixedColor(bool isFixedColor) { m_isFixedColor = isFixedColor; }
	void SetCanBeActivated(bool canBeActivated) { m_canBeActivated = canBeActivated; }
	void SetBlockedBallColor(GameColor color)
	{
		m_blockedBallColors.clear();
		if (color != GameColor::None) { m_blockedBallColors.push_back(color); }
	}
	void AddBlockedBallColor(GameColor color)
	{
		if (color == GameColor::None) { return; }
		if (std::find(
			m_blockedBallColors.begin(),
			m_blockedBallColors.end(),
			color) == m_blockedBallColors.end())
		{
			m_blockedBallColors.push_back(color);
		}
	}
	void SetSolidForPlayer(bool solid) { m_isSolidForPlayer = solid; }
	bool IsSolidForPlayer() const { return m_isSolidForPlayer; }
	void SetLockedVisual(bool locked);

	GameColor GetInitialColor() const { return m_initialColor; }
	GameColor GetCurrentColor() const { return m_currentColor; }
	bool CanBeColored() const { return m_canBeColored; }
	bool IsFixedColor() const { return m_isFixedColor; }
	bool CanBeActivated() const { return m_canBeActivated; }
	bool IsActivated() const { return m_isActivated; }
	const Math::Vector3& GetCenterPosition() const { return m_centerPosition; }
	void SetPortalSurfaceVisualColor(const Math::Vector4& color)
	{
		m_visualController.SetSurfaceColor(color);
	}

	bool IsModelLoaded() const { return m_model != nullptr; }
	bool HasPortalSurface() const { return m_hasPortalSurface; }

private:
	void DetectPlayerEntry();

	std::shared_ptr<KdModelData> m_model;
	std::shared_ptr<KdSquarePolygon> m_lockedCross;
	std::string m_modelAssetPath = "Asset/Data/door/door.gltf";
	std::weak_ptr<MapArea> m_ownerArea;
	std::string m_doorId;
	Math::Vector3 m_centerPosition = Math::Vector3::Zero;
	DoorExitPoint m_exitPoint;
	PortalVisualController m_visualController;
	GameColor m_initialColor = GameColor::None;
	GameColor m_currentColor = GameColor::None;
	std::vector<GameColor> m_blockedBallColors;
	bool m_canBeColored = true;
	bool m_isFixedColor = false;
	bool m_canBeActivated = true;
	bool m_isActivated = false;
	bool m_isSolidForPlayer = false;
	bool m_hasLockedVisual = false;
	bool m_hasPortalSurface = false;
	float m_teleportCooldownDuration = 0.75f;
	float m_teleportCooldownRemaining = 0.0f;
	float m_exitDistance = 1.5f;
	float m_allowedEntrySide = 0.0f;
	float m_forcedExitSide = 0.0f;
	float m_playerTriggerHalfWidth = 0.72f;
	float m_playerTriggerHalfDepth = 1.45f;
};
