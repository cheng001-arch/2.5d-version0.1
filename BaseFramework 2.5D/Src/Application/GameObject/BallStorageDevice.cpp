#include "BallStorageDevice.h"
#include "ColorBall.h"
#include "PlayerController2_5D.h"
#include "PlayerCarrySlot.h"

void BallStorageDevice::Init()
{
	m_model = std::make_shared<KdModelData>();
	if (!m_model->Load("Asset/Textures/tai/tai.gltf"))
	{
		m_model.reset();
		m_isExpired = true;
	}
}

void BallStorageDevice::Update()
{
	auto ball = m_storedBall.lock();
	if (ball && !ball->IsStored())
	{
		m_storedBall.reset();
	}
}

void BallStorageDevice::GenerateDepthMapFromLight()
{
	if (!m_model) { return; }
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void BallStorageDevice::DrawLit()
{
	if (!m_model) { return; }
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void BallStorageDevice::SetPlacement(const Math::Vector3& position)
{
	m_centerPosition = position;
	m_ballRestPosition = position + Math::Vector3(0.0f, 1.45f, 0.0f);
	m_mWorld =
		Math::Matrix::CreateScale(m_modelScale) *
		Math::Matrix::CreateTranslation(position);
}

void BallStorageDevice::Interact(PlayerController2_5D& player)
{
	auto& carrySlot = player.WorkCarrySlot();
	auto storedBall = m_storedBall.lock();

	if (storedBall)
	{
		if (carrySlot.HasBall()) { return; }

		storedBall->ReleaseFromStorage();
		carrySlot.PickUpBall(storedBall);
		m_storedBall.reset();
		return;
	}

	auto carriedBall = carrySlot.ReleaseCurrentBall();
	if (!carriedBall) { return; }

	carriedBall->SetOwnerArea(player.GetCurrentArea());
	carriedBall->StoreAt(m_ballRestPosition);
	m_storedBall = carriedBall;
}
