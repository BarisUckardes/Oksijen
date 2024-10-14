#pragma once
#include <glm/glm.hpp>

namespace Oksijen
{
	class RUNTIME_API Camera final
	{
	public:
		Camera();
		~Camera() = default;

		FORCEINLINE glm::vec3 GetPosition() const noexcept { return mPosition; }
		FORCEINLINE glm::vec3 GetRotation() const noexcept { return mRotation; }
		FORCEINLINE float GetFieldOfView() const noexcept { return mFieldOfView; }
		FORCEINLINE float GetAspectRatio() const noexcept { return mAspectRatio; }
		FORCEINLINE float GetNearClip() const noexcept { return mNearClip;}
		FORCEINLINE float GetFarClip() const noexcept { return mFarClip; }

		void Move(const glm::vec3& amount);
		void Rotate(const glm::vec3& amount);
		void SetPosition(const glm::vec3& pos);
		void SetRotation(const glm::vec3& rot);
		void SetFieldOfView(const float fov);
		void SetAspectRatio(const float aspect);
		void SetNearClip(const float clip);
		void SetFarClip(const float clip);

		glm::vec3 GetForward() const noexcept;
		glm::mat4 GetViewMatrix() const noexcept;
		glm::mat4 GetProjectionMatrix() const noexcept;
	private:
		glm::vec3 mPosition;
		glm::vec3 mRotation;
		float mFieldOfView;
		float mAspectRatio;
		float mNearClip;
		float mFarClip;
	};
}