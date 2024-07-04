#pragma once
#include <glm/vec3.hpp>
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Texture/Texture.h>
#include <Runtime/Graphics/Texture/TextureView.h>

namespace Oksijen
{
	class RUNTIME_API DirectionalLight final
	{
	public:
		DirectionalLight(GraphicsDevice* pDevice,GraphicsMemory* pMemory,const unsigned int width,const unsigned int height);
		~DirectionalLight();

		FORCEINLINE glm::vec3 GetDirection() const noexcept { return mDirection; }
		FORCEINLINE float GetPower() const noexcept { return mPower; }
		FORCEINLINE glm::vec3 GetColor() const noexcept { return mColor; }

		FORCEINLINE unsigned int GetWidth() const noexcept { return mWidth; }
		FORCEINLINE unsigned int GetHeight() const noexcept { return mHeight; }

		FORCEINLINE Texture* GetShadowTexture() const noexcept { return mShadowTexture; }
		FORCEINLINE TextureView* GetShaderTextureView() const noexcept { return mShadowTextureView; }

		void SetDirection(const glm::vec3& direction);
		void SetPower(const float power);
		void SetColor(const glm::vec3& color);
		void Resize(GraphicsMemory* pMemory,const unsigned int width, const unsigned int height);
	private:
		void ClearResources();
	private:
		GraphicsDevice* mDevice;

		glm::vec3 mDirection;
		float mPower;
		glm::vec3 mColor;

		unsigned int mWidth;
		unsigned int mHeight;

		Texture* mShadowTexture;
		TextureView* mShadowTextureView;
	};
}