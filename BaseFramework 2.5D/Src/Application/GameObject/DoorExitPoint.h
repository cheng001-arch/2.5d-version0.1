#pragma once

class DoorExitPoint
{
public:
	void SetPosition(const Math::Vector3& position) { m_position = position; }
	const Math::Vector3& GetPosition() const { return m_position; }

	void SetFacingDirectionAfterExit(float direction)
	{
		m_facingDirectionAfterExit = direction < 0.0f ? -1.0f : 1.0f;
	}
	float GetFacingDirectionAfterExit() const { return m_facingDirectionAfterExit; }

private:
	Math::Vector3 m_position = Math::Vector3::Zero;
	float m_facingDirectionAfterExit = 1.0f;
};
