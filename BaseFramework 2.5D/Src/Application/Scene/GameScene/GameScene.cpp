#include "GameScene.h"
#include"../SceneManager.h"

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

		void DrawUnLit() override
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
	auto map = std::make_shared<Map1>();
	map->Init();
	AddObject(map);

	m_camera = std::make_unique<KdCamera>();
	m_camera->SetProjectionMatrix(60.0f, 500.0f, 0.1f);

	// Face the XY plane directly. Z controls foreground/background depth.
	// The model bounds are approximately X:-1..90, Y:-1..24, Z:-1..1.
	const Math::Vector3 cameraPos(45.0f, 11.5f, -90.0f);
	const Math::Vector3 targetPos(45.0f, 11.5f, 0.0f);
	Math::Matrix view;
	DirectX::XMStoreFloat4x4(
		&view,
		DirectX::XMMatrixLookAtLH(cameraPos, targetPos, Math::Vector3::Up));
	m_camera->SetCameraMatrix(view.Invert());

	auto& ambient = KdShaderManager::Instance().WorkAmbientController();
	ambient.SetAmbientLight({ 0.65f, 0.65f, 0.65f, 1.0f });
	ambient.SetDirLight({ 0.4f, -1.0f, 0.3f }, { 1.0f, 1.0f, 1.0f });
	ambient.SetDirLightShadowArea({ 140.0f, 140.0f }, 100.0f);
}
