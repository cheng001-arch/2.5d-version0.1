#pragma once

class PlayerController2_5D;
class PortalDoor;

class PortalTeleportService
{
public:
	static PortalTeleportService& Instance()
	{
		static PortalTeleportService instance;
		return instance;
	}

	bool TeleportPlayer(
		const std::shared_ptr<PlayerController2_5D>& player,
		const std::shared_ptr<PortalDoor>& fromDoor,
		const std::shared_ptr<PortalDoor>& toDoor,
		float entrySide);

private:
	PortalTeleportService() = default;
};
