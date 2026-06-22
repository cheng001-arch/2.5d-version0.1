#include "PortalVisualController.h"

void PortalVisualController::UpdateDoorColorVisual(GameColor color, bool isActivated)
{
	auto model = m_model.lock();
	if (!model) { return; }

	Math::Vector4 visualColor = { 0.18f, 0.22f, 0.28f, 1.0f };
	if (isActivated)
	{
		switch (color)
		{
		case GameColor::Black:
			// Keep a small amount of light so the black surface remains readable.
			visualColor = { 0.025f, 0.025f, 0.035f, 1.0f };
			break;
		case GameColor::White:
			visualColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			break;
		case GameColor::Red:
			visualColor = { 1.0f, 0.035f, 0.025f, 1.0f };
			break;
		case GameColor::Rainbow:
			visualColor = { 1.0f, 0.35f, 0.8f, 1.0f };
			break;
		default:
			break;
		}
	}

	SetSurfaceColor(visualColor);
}

void PortalVisualController::SetSurfaceColor(const Math::Vector4& color)
{
	auto model = m_model.lock();
	if (!model) { return; }

	for (auto& material : model->WorkMaterials())
	{
		if (material.m_name == "M_PortalTintable")
		{
			material.m_baseColorRate = color;
			break;
		}
	}
}
