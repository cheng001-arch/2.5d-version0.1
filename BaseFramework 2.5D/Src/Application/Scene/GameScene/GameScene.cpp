#include "GameScene.h"
#include"../SceneManager.h"
#include "../../GameSystem/GameManager.h"
#include "../../GameSystem/LevelManager.h"
#include "../../GameSystem/MapArea.h"
#include "../../GameObject/PlayerController2_5D.h"
#include "../../GameObject/ColorBall.h"
#include "../../GameObject/BallStorageDevice.h"
#include "../../GameObject/PortalDoor.h"
#include "../../GameObject/PlayerVoidRespawnController.h"
#include "../../GameObject/RainbowGoalDoor.h"
#include "../../GameObject/LevelTransitionController.h"
#include "../../UI/PauseMenuUI.h"
#include "../../main.h"

namespace
{
	class StageMap : public KdGameObject
	{
	public:
		StageMap(std::string_view assetPath, std::string_view colliderName)
			: m_assetPath(assetPath), m_colliderName(colliderName)
		{
		}

		void Init() override
		{
			m_model = std::make_shared<KdModelData>();
			if (!m_model->Load(m_assetPath))
			{
				m_isExpired = true;
				return;
			}

			// 2.5D coordinate convention:
			// X = horizontal, Y = vertical, Z = depth.
			// The Blender model already faces the XY screen plane.
			m_mWorld = Math::Matrix::Identity;

			m_pCollider = std::make_unique<KdCollider>();
			m_pCollider->RegisterCollisionShape(
				m_colliderName, m_model,
				KdCollider::TypeGround | KdCollider::TypeBump);
		}

		void GenerateDepthMapFromLight() override
		{
			if (!m_model) { return; }
			KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
		}

		void DrawLit() override
		{
			if (!m_model) { return; }
			KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
		}

	private:
		std::string m_assetPath;
		std::string m_colliderName;
		std::shared_ptr<KdModelData> m_model;
	};

	class StagePlatform : public KdGameObject
	{
	public:
		StagePlatform(
			const Math::Vector3& topCenter,
			const Math::Vector3& size)
			: m_topCenter(topCenter), m_size(size)
		{
		}

		void Init() override
		{
			m_polygon = std::make_shared<KdSquarePolygon>(
				"Asset/Data/map1/stone_floor.png");
			m_polygon->SetScale({ m_size.x, m_size.z });
			m_polygon->SetPivot(KdSquarePolygon::PivotType::Center_Middle);

			m_mWorld = Math::Matrix::CreateTranslation(m_topCenter);

			DirectX::BoundingBox collisionBox;
			collisionBox.Center = {
				0.0f,
				-m_size.y * 0.5f,
				0.0f
			};
			collisionBox.Extents = {
				m_size.x * 0.5f,
				m_size.y * 0.5f,
				m_size.z * 0.5f
			};
			m_pCollider = std::make_unique<KdCollider>();
			m_pCollider->RegisterCollisionShape(
				"StagePlatform",
				collisionBox,
				KdCollider::TypeGround | KdCollider::TypeBump);
		}

		void DrawUnLit() override
		{
			if (!m_polygon) { return; }
			const Math::Matrix drawMatrix =
				Math::Matrix::CreateRotationX(DirectX::XM_PIDIV2) *
				m_mWorld;
			KdShaderManager::Instance().m_StandardShader.DrawPolygon(
				*m_polygon, drawMatrix);
		}

	private:
		Math::Vector3 m_topCenter;
		Math::Vector3 m_size;
		std::shared_ptr<KdSquarePolygon> m_polygon;
	};
}

void GameScene::Event()
{
	UpdateFollowCamera();

	if (GetAsyncKeyState(VK_ESCAPE) & 0x0001)
	{
		const bool shouldOpen = !m_pauseMenu || !m_pauseMenu->IsOpen();
		if (m_pauseMenu) { m_pauseMenu->SetOpen(shouldOpen); }
		shouldOpen
			? GameManager::Instance().PauseGame()
			: GameManager::Instance().ResumeGame();
	}

	if (m_pauseMenu && m_pauseMenu->IsOpen()) { return; }

	if (GetAsyncKeyState('P') & 0x0001)
	{
		auto& gameManager = GameManager::Instance();
		gameManager.IsPaused() ? gameManager.ResumeGame() : gameManager.PauseGame();
	}

	if (GetAsyncKeyState('R') & 0x0001)
	{
		auto player = m_player.lock();
		if (player)
		{
			LevelTransitionController::StartRestartLevelTransition(player);
		}
	}

	if (GetAsyncKeyState('T') & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}
}

void GameScene::PreDraw()
{
	BaseScene::PreDraw();

	if (!m_levelBackground) { return; }

	KdShaderManager::Instance().m_spriteShader.Begin();
	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_levelBackground.get(),
		static_cast<int>(m_backgroundOffsetX),
		0,
		1360,
		766);
	KdShaderManager::Instance().m_spriteShader.End();
}

void GameScene::Init()
{
	auto& levelManager = LevelManager::Instance();
	levelManager.ClearLevel();
	const int currentLevelId = levelManager.GetCurrentLevelId();
	const bool isLevel1 = currentLevelId == 0;
	const bool isLevel2 = currentLevelId == 1;
	const bool isLevel3 = currentLevelId == 2;
	const bool isLevel4 = currentLevelId == 3;
	const bool isLevel5 = currentLevelId >= 4;

	const std::string mapAssetPath =
		isLevel1 ? "Asset/Data/map1/map_floor.gltf" :
		(isLevel4 || isLevel5) ? "Asset/Data/map3/map3.gltf" :
		"Asset/Data/map2/map2.gltf";
	const std::string mapColliderName =
		isLevel1 ? "Map1Ground" :
		(isLevel4 || isLevel5) ? "Map3Ground" :
		"Map2Ground";
	const Math::Vector3 spawnPoint =
		isLevel1 ? Math::Vector3(2.0f, 1.0f, 0.0f) :
		(isLevel4 || isLevel5) ? Math::Vector3(-12.0f, 2.979f, 0.0f) :
		Math::Vector3(-48.0f, 1.0f, 0.0f);

	levelManager.SetPlayerSpawnPoint(spawnPoint);
	levelManager.SetVoidBaseY(0.0f);
	levelManager.SetVoidFallDistance(8.0f);

	auto map = std::make_shared<StageMap>(mapAssetPath, mapColliderName);
	map->Init();
	AddObject(map);

	auto startArea = std::make_shared<MapArea>();
	startArea->SetAreaId(0);
	startArea->SetAreaName(
		isLevel1 ? "Area_Start" :
		isLevel2 ? "Area_Level2" :
		isLevel3 ? "Area_Level3_Main" :
		isLevel4 ? "Area_Level4_Hub" :
		"Area_Level5_Alpha");
	startArea->SetGameplayZ(0.0f);
	startArea->SetAreaRoot(map);
	AddObject(startArea);
	levelManager.RegisterArea(startArea);

	std::shared_ptr<MapArea> rainbowRoomArea;
	std::shared_ptr<MapArea> level4WhiteArea;
	std::shared_ptr<MapArea> level4RedArea;
	std::shared_ptr<MapArea> level4BlackArea;
	std::shared_ptr<MapArea> level5BetaArea;
	if (isLevel3)
	{
		auto rainbowRoomMap = std::make_shared<StageMap>(
			"Asset/Data/map2/map2.gltf", "Map2RainbowRoomGround");
		rainbowRoomMap->Init();
		rainbowRoomMap->SetPos({ 0.0f, 0.0f, 20.0f });
		AddObject(rainbowRoomMap);

		rainbowRoomArea = std::make_shared<MapArea>();
		rainbowRoomArea->SetAreaId(1);
		rainbowRoomArea->SetAreaName("Area_Level3_RainbowRoom");
		rainbowRoomArea->SetGameplayZ(20.0f);
		rainbowRoomArea->SetAreaRoot(rainbowRoomMap);
		AddObject(rainbowRoomArea);
		levelManager.RegisterArea(rainbowRoomArea);
	}
	else if (isLevel4)
	{
		const auto createArea =
			[&](int id, std::string_view name)
			{
				auto area = std::make_shared<MapArea>();
				area->SetAreaId(id);
				area->SetAreaName(name);
				area->SetGameplayZ(0.0f);
				area->SetAreaRoot(map);
				AddObject(area);
				levelManager.RegisterArea(area);
				return area;
			};

		level4WhiteArea = createArea(1, "Area_Level4_WhiteRoom");
		level4RedArea = createArea(2, "Area_Level4_RedRoom");
		level4BlackArea = createArea(3, "Area_Level4_BlackRoom");
	}
	else if (isLevel5)
	{
		auto betaMap = std::make_shared<StageMap>(
			"Asset/Data/map2/map2.gltf", "Map2Level5BetaGround");
		betaMap->Init();
		betaMap->SetPos({ 0.0f, 0.0f, 20.0f });
		AddObject(betaMap);

		level5BetaArea = std::make_shared<MapArea>();
		level5BetaArea->SetAreaId(1);
		level5BetaArea->SetAreaName("Area_Level5_Beta_Map2");
		level5BetaArea->SetGameplayZ(20.0f);
		level5BetaArea->SetAreaRoot(betaMap);
		AddObject(level5BetaArea);
		levelManager.RegisterArea(level5BetaArea);
	}
	levelManager.InitializeLevel();

	auto player = std::make_shared<PlayerController2_5D>();
	player->Init();
	player->SetCurrentArea(startArea);
	player->SetPos(levelManager.GetPlayerSpawnPoint());
	AddObject(player);
	levelManager.SetPlayer(player);
	m_player = player;
	m_levelBackground = std::make_shared<KdTexture>(
		"Asset/Textures/screen/beijing2.png");
	m_backgroundOriginPlayerX = player->GetPos().x;
	m_backgroundOffsetX = 0.0f;

	auto levelTransition = std::make_shared<LevelTransitionController>();
	levelTransition->AttachPlayer(player);
	AddObject(levelTransition);

	auto voidRespawnController = std::make_shared<PlayerVoidRespawnController>();
	voidRespawnController->SetPlayer(player);
	AddObject(voidRespawnController);

	m_pauseMenu = std::make_shared<PauseMenuUI>();
	m_pauseMenu->Init();
	AddObject(m_pauseMenu);

	if (isLevel1)
	{
		auto blackBall = std::make_shared<ColorBall>();
		blackBall->Init();
		blackBall->SetBallColor(GameColor::Black);
		blackBall->SetOwnerArea(startArea);
		blackBall->SetPos({ 8.0f, 1.633f, 0.0f });
		blackBall->SaveInitialState();
		AddObject(blackBall);
		startArea->RegisterBall(blackBall);
		levelManager.RegisterBall(blackBall);

		auto whiteBall = std::make_shared<ColorBall>();
		whiteBall->Init();
		whiteBall->SetBallColor(GameColor::White);
		whiteBall->SetOwnerArea(startArea);
		whiteBall->SetPos({ 12.0f, 1.633f, 0.0f });
		whiteBall->SaveInitialState();
		AddObject(whiteBall);
		startArea->RegisterBall(whiteBall);
		levelManager.RegisterBall(whiteBall);

		auto leftDoor = std::make_shared<PortalDoor>();
		leftDoor->Init();
		leftDoor->SetDoorId("Door_A");
		leftDoor->SetOwnerArea(startArea);
		leftDoor->SetPlacement({ 30.0f, 2.0f, 0.0f });
		leftDoor->SetExitPoint({ 31.5f, 1.0f, 0.0f }, 1.0f);
		leftDoor->SetInitialColor(GameColor::None);
		leftDoor->SetCanBeColored(true);
		leftDoor->SetFixedColor(false);
		leftDoor->SetCanBeActivated(true);
		AddObject(leftDoor);
		startArea->RegisterDoor(leftDoor);
		levelManager.RegisterDoor(leftDoor);

		auto rightDoor = std::make_shared<PortalDoor>();
		rightDoor->SetModelAssetPath("Asset/Data/door2/door2.gltf");
		rightDoor->Init();
		rightDoor->SetDoorId("Door_B");
		rightDoor->SetOwnerArea(startArea);
		rightDoor->SetPlacement({ 70.0f, 2.0f, 0.0f });
		rightDoor->SetExitPoint({ 68.5f, 1.0f, 0.0f }, -1.0f);
		rightDoor->SetInitialColor(GameColor::Black);
		rightDoor->SetCanBeColored(false);
		rightDoor->SetFixedColor(true);
		rightDoor->SetCanBeActivated(true);
		AddObject(rightDoor);
		startArea->RegisterDoor(rightDoor);
		levelManager.RegisterDoor(rightDoor);

		auto goalDoor = std::make_shared<RainbowGoalDoor>();
		goalDoor->Init();
		goalDoor->SetDoorId("Rainbow_Goal");
		goalDoor->SetOwnerArea(startArea);
		goalDoor->SetPlacement({ 87.0f, 2.0f, 0.0f });
		goalDoor->SetInitialColor(GameColor::Rainbow);
		goalDoor->SetCanBeColored(false);
		goalDoor->SetFixedColor(true);
		goalDoor->SetCanBeActivated(true);
		AddObject(goalDoor);
		startArea->RegisterDoor(goalDoor);
		levelManager.SetGoalDoor(goalDoor);
	}
	else if (isLevel2)
	{
		// Level 2 puzzle layout:
		// Start/black ball/A are isolated on the left by the central wall.
		// B/white ball are on the right ground.
		// C and the rainbow goal are on the otherwise unreachable upper platform.
		auto blackBall = std::make_shared<ColorBall>();
		blackBall->Init();
		blackBall->SetBallColor(GameColor::Black);
		blackBall->SetOwnerArea(startArea);
		blackBall->SetPos({ -44.0f, 1.633f, 0.0f });
		blackBall->SaveInitialState();
		AddObject(blackBall);
		startArea->RegisterBall(blackBall);
		levelManager.RegisterBall(blackBall);

		auto whiteBall = std::make_shared<ColorBall>();
		whiteBall->Init();
		whiteBall->SetBallColor(GameColor::White);
		whiteBall->SetOwnerArea(startArea);
		whiteBall->SetPos({ 9.0f, 1.633f, 0.0f });
		whiteBall->SaveInitialState();
		AddObject(whiteBall);
		startArea->RegisterBall(whiteBall);
		levelManager.RegisterBall(whiteBall);

		auto doorA = std::make_shared<PortalDoor>();
		doorA->Init();
		doorA->SetDoorId("Level2_Door_A");
		doorA->SetOwnerArea(startArea);
		doorA->SetPlacement({ -38.0f, 2.0f, 0.0f });
		doorA->SetInitialColor(GameColor::None);
		doorA->SetCanBeColored(true);
		doorA->SetFixedColor(false);
		doorA->SetCanBeActivated(true);
		AddObject(doorA);
		startArea->RegisterDoor(doorA);
		levelManager.RegisterDoor(doorA);

		auto fixedBlackDoorB = std::make_shared<PortalDoor>();
		fixedBlackDoorB->SetModelAssetPath("Asset/Data/door2/door2.gltf");
		fixedBlackDoorB->Init();
		fixedBlackDoorB->SetDoorId("Level2_Door_B_Black");
		fixedBlackDoorB->SetOwnerArea(startArea);
		fixedBlackDoorB->SetPlacement({ 5.0f, 2.0f, 0.0f });
		fixedBlackDoorB->SetInitialColor(GameColor::Black);
		fixedBlackDoorB->SetCanBeColored(false);
		fixedBlackDoorB->SetFixedColor(true);
		fixedBlackDoorB->SetCanBeActivated(true);
		AddObject(fixedBlackDoorB);
		startArea->RegisterDoor(fixedBlackDoorB);
		levelManager.RegisterDoor(fixedBlackDoorB);

		auto fixedWhiteDoorC = std::make_shared<PortalDoor>();
		fixedWhiteDoorC->SetModelAssetPath("Asset/Data/door2/door2.gltf");
		fixedWhiteDoorC->Init();
		fixedWhiteDoorC->SetDoorId("Level2_Door_C_White");
		fixedWhiteDoorC->SetOwnerArea(startArea);
		fixedWhiteDoorC->SetPlacement({ 14.0f, 12.827f, 0.0f });
		fixedWhiteDoorC->SetInitialColor(GameColor::White);
		fixedWhiteDoorC->SetCanBeColored(false);
		fixedWhiteDoorC->SetFixedColor(true);
		fixedWhiteDoorC->SetCanBeActivated(true);
		AddObject(fixedWhiteDoorC);
		startArea->RegisterDoor(fixedWhiteDoorC);
		levelManager.RegisterDoor(fixedWhiteDoorC);

		auto goalDoor = std::make_shared<RainbowGoalDoor>();
		goalDoor->Init();
		goalDoor->SetDoorId("Level2_Rainbow_Goal");
		goalDoor->SetOwnerArea(startArea);
		goalDoor->SetPlacement({ 23.0f, 12.827f, 0.0f });
		goalDoor->SetInitialColor(GameColor::Rainbow);
		goalDoor->SetCanBeColored(false);
		goalDoor->SetFixedColor(true);
		goalDoor->SetCanBeActivated(true);
		AddObject(goalDoor);
		startArea->RegisterDoor(goalDoor);
		levelManager.SetGoalDoor(goalDoor);
	}
	else if (isLevel3)
	{
		// Level 3: white -> red route through variable Door A,
		// then black Door B reaches the Z-offset rainbow-ball room.
		auto whiteBall = std::make_shared<ColorBall>();
		whiteBall->Init();
		whiteBall->SetBallColor(GameColor::White);
		whiteBall->SetOwnerArea(startArea);
		whiteBall->SetPos({ -44.0f, 1.633f, 0.0f });
		whiteBall->SaveInitialState();
		AddObject(whiteBall);
		startArea->RegisterBall(whiteBall);
		levelManager.RegisterBall(whiteBall);

		auto doorA = std::make_shared<PortalDoor>();
		doorA->Init();
		doorA->SetDoorId("Level3_Door_A_Variable");
		doorA->SetOwnerArea(startArea);
		doorA->SetPlacement({ -38.0f, 2.0f, 0.0f });
		doorA->SetInitialColor(GameColor::None);
		doorA->SetCanBeColored(true);
		doorA->SetFixedColor(false);
		doorA->SetCanBeActivated(true);
		AddObject(doorA);
		startArea->RegisterDoor(doorA);
		levelManager.RegisterDoor(doorA);

		auto fixedWhiteDoorC = std::make_shared<PortalDoor>();
		fixedWhiteDoorC->SetModelAssetPath("Asset/Data/door2/door2.gltf");
		fixedWhiteDoorC->Init();
		fixedWhiteDoorC->SetDoorId("Level3_Door_C_White");
		fixedWhiteDoorC->SetOwnerArea(startArea);
		fixedWhiteDoorC->SetPlacement({ 5.0f, 2.0f, 0.0f });
		fixedWhiteDoorC->SetInitialColor(GameColor::White);
		fixedWhiteDoorC->SetCanBeColored(false);
		fixedWhiteDoorC->SetFixedColor(true);
		fixedWhiteDoorC->SetCanBeActivated(true);
		AddObject(fixedWhiteDoorC);
		startArea->RegisterDoor(fixedWhiteDoorC);
		levelManager.RegisterDoor(fixedWhiteDoorC);

		auto redBall = std::make_shared<ColorBall>();
		redBall->Init();
		redBall->SetBallColor(GameColor::Red);
		redBall->SetOwnerArea(startArea);
		redBall->SetPos({ 9.0f, 1.633f, 0.0f });
		redBall->SaveInitialState();
		AddObject(redBall);
		startArea->RegisterBall(redBall);
		levelManager.RegisterBall(redBall);

		auto fixedRedDoorD = std::make_shared<PortalDoor>();
		fixedRedDoorD->SetModelAssetPath("Asset/Data/door2/door2.gltf");
		fixedRedDoorD->Init();
		fixedRedDoorD->SetDoorId("Level3_Door_D_Red");
		fixedRedDoorD->SetOwnerArea(startArea);
		fixedRedDoorD->SetPlacement({ 12.5f, 12.827f, 0.0f });
		fixedRedDoorD->SetInitialColor(GameColor::Red);
		fixedRedDoorD->SetCanBeColored(false);
		fixedRedDoorD->SetFixedColor(true);
		fixedRedDoorD->SetCanBeActivated(true);
		AddObject(fixedRedDoorD);
		startArea->RegisterDoor(fixedRedDoorD);
		levelManager.RegisterDoor(fixedRedDoorD);

		auto doorB = std::make_shared<PortalDoor>();
		doorB->Init();
		doorB->SetDoorId("Level3_Door_B_Variable");
		doorB->SetOwnerArea(startArea);
		doorB->SetPlacement({ 20.0f, 12.827f, 0.0f });
		doorB->SetInitialColor(GameColor::None);
		doorB->SetCanBeColored(true);
		doorB->SetFixedColor(false);
		doorB->SetCanBeActivated(true);
		AddObject(doorB);
		startArea->RegisterDoor(doorB);
		levelManager.RegisterDoor(doorB);

		auto blackBall = std::make_shared<ColorBall>();
		blackBall->Init();
		blackBall->SetBallColor(GameColor::Black);
		blackBall->SetOwnerArea(startArea);
		blackBall->SetPos({ 23.5f, 12.46f, 0.0f });
		blackBall->SaveInitialState();
		AddObject(blackBall);
		startArea->RegisterBall(blackBall);
		levelManager.RegisterBall(blackBall);

		auto fixedBlackDoorE = std::make_shared<PortalDoor>();
		fixedBlackDoorE->SetModelAssetPath("Asset/Data/door2/door2.gltf");
		fixedBlackDoorE->Init();
		fixedBlackDoorE->SetDoorId("Level3_Door_E_Black");
		fixedBlackDoorE->SetOwnerArea(rainbowRoomArea);
		fixedBlackDoorE->SetPlacement({ -45.0f, 2.0f, 20.0f });
		fixedBlackDoorE->SetInitialColor(GameColor::Black);
		fixedBlackDoorE->SetCanBeColored(false);
		fixedBlackDoorE->SetFixedColor(true);
		fixedBlackDoorE->SetCanBeActivated(true);
		AddObject(fixedBlackDoorE);
		rainbowRoomArea->RegisterDoor(fixedBlackDoorE);
		levelManager.RegisterDoor(fixedBlackDoorE);

		auto rainbowBall = std::make_shared<ColorBall>();
		rainbowBall->Init();
		rainbowBall->SetBallColor(GameColor::Rainbow);
		rainbowBall->SetOwnerArea(rainbowRoomArea);
		rainbowBall->SetPos({ -49.0f, 1.633f, 20.0f });
		rainbowBall->SaveInitialState();
		AddObject(rainbowBall);
		rainbowRoomArea->RegisterBall(rainbowBall);
		levelManager.RegisterBall(rainbowBall);

		auto bridgePlatform1 = std::make_shared<StagePlatform>(
			Math::Vector3(28.0f, 12.85f, 0.0f),
			Math::Vector3(3.0f, 0.25f, 2.0f));
		bridgePlatform1->Init();
		AddObject(bridgePlatform1);

		auto bridgePlatform2 = std::make_shared<StagePlatform>(
			Math::Vector3(31.5f, 14.0f, 0.0f),
			Math::Vector3(3.0f, 0.25f, 2.0f));
		bridgePlatform2->Init();
		AddObject(bridgePlatform2);

		auto goalDoor = std::make_shared<RainbowGoalDoor>();
		goalDoor->Init();
		goalDoor->SetDoorId("Level3_Door_G_RainbowGoal");
		goalDoor->SetOwnerArea(startArea);
		goalDoor->SetPlacement({ 35.0f, 16.188f, 0.0f });
		goalDoor->SetInitialColor(GameColor::None);
		goalDoor->SetCanBeColored(false);
		goalDoor->SetFixedColor(true);
		goalDoor->SetCanBeActivated(true);
		AddObject(goalDoor);
		startArea->RegisterDoor(goalDoor);
		levelManager.SetGoalDoor(goalDoor);
	}
	else if (isLevel4)
	{
		// Level 4 uses the four separated structures already modeled in map3.
		// All Areas share the XY plane; Area ownership controls portal routing.
		const auto createBall =
			[&](GameColor color, const std::shared_ptr<MapArea>& area,
				const Math::Vector3& position)
			{
				auto ball = std::make_shared<ColorBall>();
				ball->Init();
				ball->SetBallColor(color);
				ball->SetOwnerArea(area);
				ball->SetPos(position);
				ball->SaveInitialState();
				AddObject(ball);
				area->RegisterBall(ball);
				levelManager.RegisterBall(ball);
				return ball;
			};

		const auto createPortal =
			[&](std::string_view id, const std::shared_ptr<MapArea>& area,
				const Math::Vector3& position, GameColor initialColor,
				bool fixedColor)
			{
				auto door = std::make_shared<PortalDoor>();
				if (fixedColor)
				{
					door->SetModelAssetPath("Asset/Data/door2/door2.gltf");
				}
				door->Init();
				door->SetDoorId(id);
				door->SetOwnerArea(area);
				door->SetPlacement(position);
				door->SetInitialColor(initialColor);
				door->SetCanBeColored(!fixedColor);
				door->SetFixedColor(fixedColor);
				door->SetCanBeActivated(true);
				AddObject(door);
				area->RegisterDoor(door);
				levelManager.RegisterDoor(door);
				return door;
			};

		// Area 1: main Hub.
		// White ball sits in the recessed low floor between the left platform
		// and the raised center block.
		createBall(GameColor::White, startArea, { 2.8f, 1.633f, 0.0f });
		auto doorA = createPortal(
			"Level4_Door_A_Variable", startArea,
			{ -9.5f, 3.979f, 0.0f }, GameColor::None, false);
		doorA->SetBlockedBallColor(GameColor::Black);

		// Permanent green shortcut between the lower floor at Door A's old
		// position and the floating platform directly above it.
		auto lowerGreenDoor = createPortal(
			"Level4_Door_F_Green_Lower", startArea,
			{ 10.5f, 2.0f, 0.0f }, GameColor::Green, true);
		auto upperGreenDoor = createPortal(
			"Level4_Door_H_Green_Upper", startArea,
			{ 10.5f, 7.871f, 0.0f }, GameColor::Green, true);
		lowerGreenDoor->SetExitDistance(1.6f);
		upperGreenDoor->SetExitDistance(1.6f);
		lowerGreenDoor->SetTeleportCooldownDuration(0.8f);
		upperGreenDoor->SetTeleportCooldownDuration(0.8f);
		createBall(
			GameColor::Black, startArea,
			{ 15.5f, 2.530f, 0.0f });

		// Area 2: the hollow white room in the upper-left structure.
		createPortal(
			"Level4_Door_C_White", level4WhiteArea,
			{ -2.9f, 10.339f, 0.0f }, GameColor::White, true);
		createBall(
			GameColor::Red, level4WhiteArea,
			{ -6.8f, 9.972f, 0.0f });

		// Area 3: upper-middle L-shaped red room and its floating platform.
		auto fixedRedDoorD = createPortal(
			"Level4_Door_D_Red", level4RedArea,
			{ 7.8f, 13.765f, 0.0f }, GameColor::Red, true);
		fixedRedDoorD->SetAllowedEntrySide(1.0f);
		fixedRedDoorD->SetForcedExitSide(1.0f);
		auto doorB = createPortal(
			"Level4_Door_B_Variable", level4RedArea,
			{ 12.0f, 13.765f, 0.0f }, GameColor::None, false);
		doorB->SetSolidForPlayer(true);
		createBall(
			GameColor::Black, level4RedArea,
			{ 18.0f, 13.398f, 0.0f });
		auto lockedShortcutDoorLevel4 = createPortal(
			"Level4_Door_LockedShortcut", level4RedArea,
			{ 20.2f, 13.765f, 0.0f }, GameColor::None, true);
		lockedShortcutDoorLevel4->SetCanBeActivated(false);
		lockedShortcutDoorLevel4->SetSolidForPlayer(true);
		lockedShortcutDoorLevel4->SetLockedVisual(true);
		lockedShortcutDoorLevel4->SetPortalSurfaceVisualColor(
			{ 0.08f, 0.08f, 0.10f, 1.0f });

		// Area 4: right-side stepped black/rainbow-ball room.
		auto fixedBlackDoorE = createPortal(
			"Level4_Door_E_Black", level4BlackArea,
			{ 23.5f, 9.294f, 0.0f }, GameColor::Black, true);
		fixedBlackDoorE->SetAllowedEntrySide(1.0f);
		fixedBlackDoorE->SetForcedExitSide(1.0f);
		createBall(
			GameColor::Rainbow, level4BlackArea,
			{ 29.0f, 8.927f, 0.0f });

		auto goalDoor = std::make_shared<RainbowGoalDoor>();
		goalDoor->Init();
		goalDoor->SetDoorId("Level4_Door_G_RainbowGoal");
		goalDoor->SetOwnerArea(startArea);
		goalDoor->SetPlacement({ 34.0f, 2.635f, 0.0f });
		goalDoor->SetInitialColor(GameColor::None);
		goalDoor->SetCanBeColored(false);
		goalDoor->SetFixedColor(true);
		goalDoor->SetCanBeActivated(true);
		AddObject(goalDoor);
		startArea->RegisterDoor(goalDoor);
		levelManager.RegisterDoor(goalDoor);
		levelManager.SetGoalDoor(goalDoor);
	}
	else
	{
		// Level 5: Alpha uses map3 at Z=0. Beta uses a normal, unmirrored
		// map2 at Z=20 so its rendering and collision transforms stay stable.
		const auto createBall =
			[&](GameColor color, const std::shared_ptr<MapArea>& area,
				const Math::Vector3& position)
			{
				auto ball = std::make_shared<ColorBall>();
				ball->Init();
				ball->SetBallColor(color);
				ball->SetOwnerArea(area);
				ball->SetPos(position);
				ball->SaveInitialState();
				AddObject(ball);
				area->RegisterBall(ball);
				levelManager.RegisterBall(ball);
				return ball;
			};

		const auto createPortal =
			[&](std::string_view id, const std::shared_ptr<MapArea>& area,
				const Math::Vector3& position, GameColor initialColor,
				bool fixedColor)
			{
				auto door = std::make_shared<PortalDoor>();
				if (fixedColor)
				{
					door->SetModelAssetPath("Asset/Data/door2/door2.gltf");
				}
				door->Init();
				door->SetDoorId(id);
				door->SetOwnerArea(area);
				door->SetPlacement(position);
				door->SetInitialColor(initialColor);
				door->SetCanBeColored(!fixedColor);
				door->SetFixedColor(fixedColor);
				door->SetCanBeActivated(true);
				AddObject(door);
				area->RegisterDoor(door);
				levelManager.RegisterDoor(door);
				return door;
			};

		// Alpha: white -> red -> blue progression through Hub door A.
		createBall(GameColor::White, startArea, { -9.5f, 3.612f, 0.0f });
		auto startBallStorageDevice = std::make_shared<BallStorageDevice>();
		startBallStorageDevice->Init();
		startBallStorageDevice->SetOwnerArea(startArea);
		startBallStorageDevice->SetPlacement({ -7.2f, 3.78f, 0.0f });
		AddObject(startBallStorageDevice);

		auto level5DoorA = createPortal(
			"Level5_Door_A_Hub", startArea,
			{ 2.8f, 2.0f, 0.0f }, GameColor::None, false);
		level5DoorA->SetBlockedBallColor(GameColor::Yellow);
		level5DoorA->AddBlockedBallColor(GameColor::Black);
		level5DoorA->AddBlockedBallColor(GameColor::Rainbow);

		auto fixedPurpleDoorLower = createPortal(
			"Level5_Door_Purple_Lower", startArea,
			{ 10.5f, 2.0f, 0.0f }, GameColor::Purple, true);
		fixedPurpleDoorLower->SetAllowedEntrySide(-1.0f);
		fixedPurpleDoorLower->SetForcedExitSide(-1.0f);

		auto fixedPurpleDoorUpper = createPortal(
			"Level5_Door_Purple_Upper", startArea,
			{ 10.5f, 7.871f, 0.0f }, GameColor::Purple, true);
		fixedPurpleDoorUpper->SetAllowedEntrySide(-1.0f);
		fixedPurpleDoorUpper->SetForcedExitSide(-1.0f);
		createBall(
			GameColor::Yellow, startArea,
			{ 7.4f, 7.504f, 0.0f });
		createBall(
			GameColor::Black, startArea,
			{ 5.7f, 7.504f, 0.0f });

		auto fixedWhiteDoorW = createPortal(
			"Level5_Door_W_White", startArea,
			{ -2.9f, 10.339f, 0.0f }, GameColor::White, true);
		fixedWhiteDoorW->SetAllowedEntrySide(-1.0f);
		fixedWhiteDoorW->SetForcedExitSide(-1.0f);
		createBall(GameColor::Red, startArea, { -6.8f, 9.972f, 0.0f });
		createBall(GameColor::Rainbow, startArea, { -5.2f, 9.972f, 0.0f });

		createPortal(
			"Level5_Door_R_Red", startArea,
			{ 12.0f, 13.765f, 0.0f }, GameColor::Red, true);
		createBall(GameColor::Blue, startArea, { 18.0f, 13.398f, 0.0f });
		auto lockedShortcutDoorLevel5 = createPortal(
			"Level5_Door_LockedShortcut", startArea,
			{ 20.2f, 13.765f, 0.0f }, GameColor::None, true);
		lockedShortcutDoorLevel5->SetCanBeActivated(false);
		lockedShortcutDoorLevel5->SetSolidForPlayer(true);
		lockedShortcutDoorLevel5->SetLockedVisual(true);
		lockedShortcutDoorLevel5->SetPortalSurfaceVisualColor(
			{ 0.08f, 0.08f, 0.10f, 1.0f });

		// Alpha black return point and final rainbow-ball route.
		createPortal(
			"Level5_Door_K_Black", startArea,
			{ 23.5f, 9.294f, 0.0f }, GameColor::Black, true);
		auto goalDoor = std::make_shared<RainbowGoalDoor>();
		goalDoor->Init();
		goalDoor->SetDoorId("Level5_Door_G_RainbowGoal");
		goalDoor->SetOwnerArea(startArea);
		goalDoor->SetPlacement({ 34.0f, 2.635f, 0.0f });
		goalDoor->SetInitialColor(GameColor::None);
		goalDoor->SetCanBeColored(false);
		goalDoor->SetFixedColor(true);
		goalDoor->SetCanBeActivated(true);
		AddObject(goalDoor);
		startArea->RegisterDoor(goalDoor);
		levelManager.SetGoalDoor(goalDoor);

		// Beta / map2 route:
		// A(blue) -> U, use the yellow ball on lower B, teleport to upper Y,
		// then use the black ball on upper C to return through Alpha K.
		auto fixedBlueDoorU = createPortal(
			"Level5_Door_U_Blue", level5BetaArea,
			{ -45.0f, 2.0f, 20.0f }, GameColor::Blue, true);
		fixedBlueDoorU->SetAllowedEntrySide(1.0f);
		fixedBlueDoorU->SetForcedExitSide(1.0f);

		auto ballStorageDevice = std::make_shared<BallStorageDevice>();
		ballStorageDevice->Init();
		ballStorageDevice->SetOwnerArea(level5BetaArea);
		ballStorageDevice->SetPlacement({ -40.5f, 1.8f, 20.0f });
		AddObject(ballStorageDevice);

		auto betaDoorB = createPortal(
			"Level5_Door_B_YellowVariable", level5BetaArea,
			{ -36.0f, 2.0f, 20.0f }, GameColor::None, false);
		betaDoorB->SetAllowedEntrySide(-1.0f);
		betaDoorB->SetBlockedBallColor(GameColor::Black);
		betaDoorB->AddBlockedBallColor(GameColor::Rainbow);

		auto fixedYellowDoorY = createPortal(
			"Level5_Door_Y_Yellow", level5BetaArea,
			{ 14.0f, 12.827f, 20.0f }, GameColor::Yellow, true);
		fixedYellowDoorY->SetForcedExitSide(1.0f);

		auto betaDoorC = createPortal(
			"Level5_Door_C_BlackVariable", level5BetaArea,
			{ 23.0f, 12.827f, 20.0f }, GameColor::None, false);
		betaDoorC->SetAllowedEntrySide(-1.0f);
		betaDoorC->SetBlockedBallColor(GameColor::Rainbow);
	}

	m_camera = std::make_unique<KdCamera>();
	m_camera->SetProjectionMatrix(55.0f, 500.0f, 0.1f);

	// Keep XY as the gameplay plane while adding a mild perspective angle.
	// The camera-to-target distance is 20 game units.
	m_cameraTarget = player->GetPos() + Math::Vector3(0.0f, 3.5f, 0.0f);
	Math::Vector3 cameraOffset(-8.1f, 3.8f, -28.6f);
	cameraOffset.Normalize();
	cameraOffset *= 20.0f;
	const Math::Vector3 cameraPos = m_cameraTarget + cameraOffset;
	Math::Matrix view;
	DirectX::XMStoreFloat4x4(
		&view,
		DirectX::XMMatrixLookAtLH(cameraPos, m_cameraTarget, Math::Vector3::Up));
	m_camera->SetCameraMatrix(view.Invert());
	m_camera->SetFocus(30.0f, 15.0f, 15.0f);

	auto& ambient = KdShaderManager::Instance().WorkAmbientController();
	ambient.SetAmbientLight({ 0.45f, 0.45f, 0.45f, 1.0f });
	ambient.SetDirLight({ 0.4f, -1.0f, 0.3f }, { 1.0f, 1.0f, 1.0f });
	ambient.SetDirLightShadowArea({ 140.0f, 140.0f }, 100.0f);
}

void GameScene::UpdateFollowCamera()
{
	auto player = m_player.lock();
	if (!player || !m_camera) { return; }

	const float deltaSeconds = Application::Instance().GetDeltaSeconds();
	const float desiredBackgroundOffset = std::clamp(
		-(player->GetPos().x - m_backgroundOriginPlayerX) * 0.45f,
		-35.0f,
		35.0f);
	const float backgroundBlend = 1.0f - std::exp(-3.0f * deltaSeconds);
	m_backgroundOffsetX +=
		(desiredBackgroundOffset - m_backgroundOffsetX) * backgroundBlend;

	Math::Vector3 desiredMouseOffset = Math::Vector3::Zero;

	POINT mousePosition;
	RECT clientRect;
	const HWND windowHandle = Application::Instance().GetWindowHandle();
	if (windowHandle &&
		GetCursorPos(&mousePosition) &&
		ScreenToClient(windowHandle, &mousePosition) &&
		GetClientRect(windowHandle, &clientRect))
	{
		const float clientWidth =
			static_cast<float>(std::max(1L, clientRect.right - clientRect.left));
		const float clientHeight =
			static_cast<float>(std::max(1L, clientRect.bottom - clientRect.top));
		const float normalizedX = std::clamp(
			mousePosition.x / clientWidth * 2.0f - 1.0f, -1.0f, 1.0f);
		const float normalizedY = std::clamp(
			1.0f - mousePosition.y / clientHeight * 2.0f, -1.0f, 1.0f);

		desiredMouseOffset = {
			normalizedX * 1.2f,
			normalizedY * 0.7f,
			0.0f
		};
	}

	const float mouseFollowRate = 5.0f;
	const float mouseBlend = 1.0f - std::exp(-mouseFollowRate * deltaSeconds);
	m_mouseCameraOffset = Math::Vector3::Lerp(
		m_mouseCameraOffset, desiredMouseOffset, mouseBlend);

	const Math::Vector3 desiredTarget =
		player->GetPos() +
		Math::Vector3(0.0f, 3.5f, 0.0f) +
		m_mouseCameraOffset;

	// Exponential smoothing avoids camera jitter while remaining frame-rate independent.
	const float followRate = 8.0f;
	const float blend = 1.0f - std::exp(-followRate * deltaSeconds);
	m_cameraTarget = Math::Vector3::Lerp(m_cameraTarget, desiredTarget, blend);

	Math::Vector3 cameraOffset(-8.1f, 3.8f, -28.6f);
	cameraOffset.Normalize();
	cameraOffset *= 20.0f;
	const Math::Vector3 cameraPos = m_cameraTarget + cameraOffset;

	Math::Matrix view;
	DirectX::XMStoreFloat4x4(
		&view,
		DirectX::XMMatrixLookAtLH(cameraPos, m_cameraTarget, Math::Vector3::Up));
	m_camera->SetCameraMatrix(view.Invert());
}
