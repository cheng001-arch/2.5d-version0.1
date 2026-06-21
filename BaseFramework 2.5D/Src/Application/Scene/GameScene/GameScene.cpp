#include "GameScene.h"
#include"../SceneManager.h"
#include "../../GameSystem/GameManager.h"
#include "../../GameSystem/LevelManager.h"
#include "../../GameSystem/MapArea.h"
#include "../../GameObject/PlayerController2_5D.h"
#include "../../GameObject/ColorBall.h"
#include "../../GameObject/PortalDoor.h"
#include "../../GameObject/PlayerVoidRespawnController.h"
#include "../../main.h"

namespace
{
	class Map1 : public KdGameObject
	{
	public:
		void Init() override
		{
			m_model = std::make_shared<KdModelData>();
			if (!m_model->Load("Asset/Data/map1/map_floor.gltf"))
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
				"Map1Ground", m_model, KdCollider::TypeGround | KdCollider::TypeBump);
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
		std::shared_ptr<KdModelData> m_model;
	};
}

void GameScene::Event()
{
	UpdateFollowCamera();

	if (GetAsyncKeyState('P') & 0x0001)
	{
		auto& gameManager = GameManager::Instance();
		gameManager.IsPaused() ? gameManager.ResumeGame() : gameManager.PauseGame();
	}

	if (GetAsyncKeyState('R') & 0x0001)
	{
		GameManager::Instance().RestartLevel();
	}

	if (GetAsyncKeyState('T') & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}
}

void GameScene::Init()
{
	auto& levelManager = LevelManager::Instance();
	levelManager.ClearLevel();
	levelManager.SetPlayerSpawnPoint({ 2.0f, 1.0f, 0.0f });
	levelManager.SetVoidBaseY(0.0f);
	levelManager.SetVoidFallDistance(8.0f);

	auto map = std::make_shared<Map1>();
	map->Init();
	AddObject(map);

	auto startArea = std::make_shared<MapArea>();
	startArea->SetAreaId(0);
	startArea->SetAreaName("Area_Start");
	startArea->SetGameplayZ(0.0f);
	startArea->SetAreaRoot(map);
	AddObject(startArea);
	levelManager.RegisterArea(startArea);
	levelManager.InitializeLevel();

	auto player = std::make_shared<PlayerController2_5D>();
	player->Init();
	player->SetCurrentArea(startArea);
	player->SetPos(levelManager.GetPlayerSpawnPoint());
	AddObject(player);
	levelManager.SetPlayer(player);
	m_player = player;

	auto voidRespawnController = std::make_shared<PlayerVoidRespawnController>();
	voidRespawnController->SetPlayer(player);
	AddObject(voidRespawnController);

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

	// Import test: two independent door instances with independently tinted
	// Portal_Surface materials.
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

	m_camera = std::make_unique<KdCamera>();
	m_camera->SetProjectionMatrix(55.0f, 500.0f, 0.1f);

	// Keep XY as the gameplay plane while adding a mild perspective angle.
	// The camera-to-target distance is exactly 30 game units.
	m_cameraTarget = player->GetPos() + Math::Vector3(0.0f, 3.5f, 0.0f);
	Math::Vector3 cameraOffset(-8.1f, 3.8f, -28.6f);
	cameraOffset.Normalize();
	cameraOffset *= 30.0f;
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
	const Math::Vector3 desiredTarget =
		player->GetPos() + Math::Vector3(0.0f, 3.5f, 0.0f);

	// Exponential smoothing avoids camera jitter while remaining frame-rate independent.
	const float followRate = 8.0f;
	const float blend = 1.0f - std::exp(-followRate * deltaSeconds);
	m_cameraTarget = Math::Vector3::Lerp(m_cameraTarget, desiredTarget, blend);

	Math::Vector3 cameraOffset(-8.1f, 3.8f, -28.6f);
	cameraOffset.Normalize();
	cameraOffset *= 30.0f;
	const Math::Vector3 cameraPos = m_cameraTarget + cameraOffset;

	Math::Matrix view;
	DirectX::XMStoreFloat4x4(
		&view,
		DirectX::XMMatrixLookAtLH(cameraPos, m_cameraTarget, Math::Vector3::Up));
	m_camera->SetCameraMatrix(view.Invert());
}
