#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Oksijen
{
	Camera::Camera() : mFieldOfView(60.0f),mPosition(glm::vec3(0,0,0)),mRotation(glm::vec3(0,0,0)),mNearClip(0.1f),mFarClip(100.0f),mAspectRatio(1.0f)
	{

	}

	void Camera::Move(const glm::vec3& amount)
	{
		mPosition += amount;
	}

	void Camera::Rotate(const glm::vec3& amount)
	{
		mRotation += amount;
	}

	void Camera::SetPosition(const glm::vec3& pos)
	{
		mPosition = pos;
	}

	void Camera::SetRotation(const glm::vec3& rot)
	{
		mRotation = rot;
	}

	void Camera::SetFieldOfView(const float fov)
	{
		mFieldOfView = glm::clamp(fov,60.0f,179.0f);
	}

	void Camera::SetAspectRatio(const float aspect)
	{
		mAspectRatio = aspect;
	}

	void Camera::SetNearClip(const float clip)
	{
		mNearClip = clip;
	}

	void Camera::SetFarClip(const float clip)
	{
		mFarClip = clip;
	}

	glm::vec3 Camera::GetForward() const noexcept
	{
		const float yaw = mRotation.y;
		const float pitch = mRotation.x;
		glm::vec3 radial;

		radial.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		radial.y = sin(glm::radians(pitch));  
		radial.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		return radial;
	}

	glm::mat4 Camera::GetViewMatrix() const noexcept
	{
		const glm::vec3 direction = GetForward();
		const glm::vec3 right = glm::cross(direction,glm::vec3(0,1,0));
		const glm::vec3 up = glm::cross(direction, right);

		return glm::lookAtRH(mPosition,mPosition+direction,up);
	}

	glm::mat4 Camera::GetProjectionMatrix() const noexcept
	{
		return glm::perspectiveRH_NO(glm::radians(mFieldOfView), mAspectRatio, mNearClip, mFarClip);
	}

}