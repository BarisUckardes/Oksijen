#include "SpotLight.h"

namespace Oksijen
{
	SpotLight::SpotLight(GraphicsDevice* pDevice, GraphicsMemory* pMemory, const unsigned int width, const unsigned int height, const VkFormat format) :
		mDevice(pDevice), mShadowTexture(nullptr), mShadowTextureView(nullptr),mPosition(glm::vec3(0.0f)), mDirection({1.0f,-1.0f,1.0f}), mColor({1.0f,1.0f,1.0f}),mFieldOfView(60.0f), mFormat(format), mWidth(width), mHeight(height), mPower(1.0f)
	{
		Resize(pMemory, width, height);
	}
	SpotLight::~SpotLight()
	{
		ClearResources();
	}
	void SpotLight::SetPosition(const glm::vec3& pos)
	{
		mPosition = pos;
	}
	void SpotLight::SetDirection(const glm::vec3& direction)
	{
		mDirection = direction;
	}
	void SpotLight::SetColor(const glm::vec3& color)
	{
		mColor = color;
	}
	void SpotLight::SetFieldOfView(const float fov)
	{
		mFieldOfView = fov;
	}
	void SpotLight::SetPower(const float power)
	{
		mPower = power;
	}
	void SpotLight::Resize(GraphicsMemory* pMemory, const unsigned int width, const unsigned int height)
	{
		//Clear former resources
		ClearResources();

		//Create texture
		mShadowTexture = mDevice->CreateTexture(pMemory, VK_IMAGE_TYPE_2D, mFormat, width, height, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_IMAGE_LAYOUT_UNDEFINED);

		//Create texture view
		constexpr VkComponentMapping mapping =
		{
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		mShadowTextureView = mDevice->CreateTextureView(mShadowTexture, 0, 0, VK_IMAGE_VIEW_TYPE_2D, mFormat, mapping, VK_IMAGE_ASPECT_DEPTH_BIT);
	}
	void SpotLight::ClearResources()
	{
		if (mShadowTexture != nullptr)
		{
			delete mShadowTexture;
			mShadowTexture = nullptr;
		}
		if (mShadowTextureView != nullptr)
		{
			delete mShadowTextureView;
			mShadowTextureView = nullptr;
		}
	}
}