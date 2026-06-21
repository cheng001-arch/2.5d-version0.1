#include "PlayerGroundChecker.h"
#include "../Scene/SceneManager.h"

bool PlayerGroundChecker::CheckGround()
{
	m_isGrounded = false;
	m_distanceToGround = FLT_MAX;
	if (!m_owner) { return false; }

	const Math::Vector3 rayStart = m_owner->GetPos() + m_groundCheckPoint;
	const KdCollider::RayInfo ray(
		KdCollider::TypeGround, rayStart, Math::Vector3::Down,
		m_groundMeasureRange);

	float nearestHitDistance = FLT_MAX;
	for (auto& object : SceneManager::Instance().GetObjList())
	{
		if (!object || object.get() == m_owner) { continue; }
		std::list<KdCollider::CollisionResult> results;
		if (object->Intersects(ray, &results))
		{
			for (const auto& result : results)
			{
				const float hitDistance = rayStart.y - result.m_hitPos.y;
				if (hitDistance >= 0.0f && hitDistance < nearestHitDistance)
				{
					nearestHitDistance = hitDistance;
					m_groundHeight = result.m_hitPos.y;
					m_distanceToGround = std::max(0.0f, m_owner->GetPos().y - m_groundHeight);
				}
			}
		}
	}

	m_isGrounded = m_distanceToGround <= m_groundCheckRadius + 0.1f;
	return m_isGrounded;
}
