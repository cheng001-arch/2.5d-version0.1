#include "PortalTeleportService.h"
#include "AreaManager.h"
#include "../GameObject/PlayerController2_5D.h"
#include "../GameObject/PortalDoor.h"

bool PortalTeleportService::TeleportPlayer(
	const std::shared_ptr<PlayerController2_5D>& player,
	const std::shared_ptr<PortalDoor>& fromDoor,
	const std::shared_ptr<PortalDoor>& toDoor,
	float entrySide)
{
	if (!player || !fromDoor || !toDoor || fromDoor == toDoor) { return false; }
	if (fromDoor->IsTeleportCoolingDown() || toDoor->IsTeleportCoolingDown())
	{
		return false;
	}

	auto targetArea = toDoor->GetOwnerArea();
	if (!targetArea) { return false; }

	// Entering from the left exits on the linked door's right, and vice versa.
	const float exitSide = entrySide < 0.0f ? 1.0f : -1.0f;
	const Math::Vector3 exitPosition = toDoor->GetExitPosition(exitSide);
	AreaManager::Instance().SwitchToArea(targetArea);
	player->SetCurrentArea(targetArea);
	player->ApplyTeleport(
		exitPosition,
		exitSide);

	const float cooldownSeconds = std::max(
		fromDoor->GetTeleportCooldownDuration(),
		toDoor->GetTeleportCooldownDuration());
	fromDoor->BeginTeleportCooldown(cooldownSeconds);
	toDoor->BeginTeleportCooldown(cooldownSeconds);
	return true;
}
