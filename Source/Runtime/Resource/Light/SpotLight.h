#pragma once
#include <glm/vec3.hpp>
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Texture/Texture.h>
#include <Runtime/Graphics/Texture/TextureView.h>

namespace Oksijen
{
	class RUNTIME_API SpotLight final
	{
	public:
		SpotLight(GraphicsDevice* pDevice, GraphicsMemory* pMemory, const unsigned int width, const unsigned int height, const VkFormat format);
		~SpotLight();

		FORCEINLINE glm::vec3 GetPosition() const noexcept { return mPosition; }
		FORCEINLINE glm::vec3 GetDirection() const noexcept { return mDirection; }
		FORCEINLINE float GetPower() const noexcept { return mPower; }
		FORCEINLINE glm::vec3 GetColor() const noexcept { return mColor; }
		FORCEINLINE float GetFieldOfView() const noexcept { return mFieldOfView; }

		FORCEINLINE VkFormat GetFormat() const noexcept { return mFormat; }
		FORCEINLINE unsigned int GetWidth() const noexcept { return mWidth; }
		FORCEINLINE unsigned int GetHeight() const noexcept { return mHeight; }

		FORCEINLINE Texture* GetShadowTexture() const noexcept { return mShadowTexture; }
		FORCEINLINE TextureView* GetShadowTextureView() const noexcept { return mShadowTextureView; }

		void SetPosition(const glm::vec3& pos);
		void SetDirection(const glm::vec3& direction);
		void SetColor(const glm::vec3& color);
		void SetFieldOfView(const float fov);
		void SetPower(const float power);
		void Resize(GraphicsMemory* pMemory, const unsigned int width, const unsigned int height);
	private:
		void ClearResources();
	private:
		GraphicsDevice* mDevice;

		glm::vec3 mPosition;
		glm::vec3 mDirection;
		float mPower;
		glm::vec3 mColor;
		float mFieldOfView;

		const VkFormat mFormat;
		unsigned int mWidth;
		unsigned int mHeight;

		Texture* mShadowTexture;
		TextureView* mShadowTextureView;
	};
}