#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <gtest/gtest.h>

#include "CookieKat/Engine/Resources/Loaders/TextureLoader.h"
#include <CookieKat/Systems/Resources/ResourceSystem.h>
#include "CookieKat/Engine/Resources/Resources/RenderTextureResource.h"

using namespace CKE;

class ConversionLoading : public testing::Test
{
protected:
	void SetUp() override {
		m_TaskSystem.Initialize();
		m_AssetSystem.Initialize(&m_TaskSystem);
	}

	void TearDown() override {
		m_AssetSystem.Shutdown();
	}

	ResourceSystem m_AssetSystem{};
	TaskSystem     m_TaskSystem{};
};

TEST_F(ConversionLoading, SaveLoadTexture) {
	auto id = m_AssetSystem.LoadResource<RenderTextureResource>("Materials/Textures/Cerberus_N.tex");
	auto r = m_AssetSystem.GetResource<RenderTextureResource>(id);
	r->GetTexture();
}
