/***
* Bare Metal Engine
* Written by Tomasz Jonarski (RexDex)
* Basic middleware testing project
***/

#include "build.h"

#include <chrono>
#include <thread>
#include <imgui.h>

//--

class ImGuiTest : public testing::Test
{
	ImGuiContext* ctx = nullptr;

public:
	static void* ImGuiMemAllocFunc(size_t sz, void* user_data)
	{
		return malloc(sz);
	}

	static void ImGuiMemFreeFunc(void* ptr, void* user_data)
	{
		free(ptr);
	}

	virtual void SetUp() override
	{
		ImGui::SetAllocatorFunctions(&ImGuiMemAllocFunc, &ImGuiMemFreeFunc, nullptr);

		ctx = ImGui::CreateContext();
		ASSERT_TRUE(ctx);

		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = 10000;
		io.DisplaySize.y = 10000;

		// Build atlas
		unsigned char* tex_pixels = NULL;
		int tex_w, tex_h;
		io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);
	}

	virtual void TearDown() override
	{
		ASSERT_EQ(ctx, ImGui::GetCurrentContext());
		ImGui::SetCurrentContext(nullptr);

		if (ctx)
		{
			ImGui::DestroyContext(ctx);
			ctx = nullptr;
		}
	}
};

//--

TEST(ImGui, NoContextActiveByDefault)
{
	ImGuiContext* c = ImGui::GetCurrentContext();
	ASSERT_EQ(nullptr, c);
}

TEST(ImGui, CreateContext)
{
	ImGuiContext* c = ImGui::CreateContext();
	ASSERT_TRUE(c);

	ImGui::DestroyContext(c);
}

TEST(ImGui, CreateContextAndActiveIt)
{
	EXPECT_EQ(nullptr, ImGui::GetCurrentContext());

	ImGuiContext* c = ImGui::CreateContext();
	ASSERT_TRUE(c);

	EXPECT_EQ(c, ImGui::GetCurrentContext());

	ImGui::SetCurrentContext(c);
	EXPECT_EQ(c, ImGui::GetCurrentContext());

	ImGui::DestroyContext(c);

	EXPECT_EQ(nullptr, ImGui::GetCurrentContext());
}

//--

TEST_F(ImGuiTest, SetupTeardown)
{}

TEST_F(ImGuiTest, ButtonClick)
{
	bool buttonClicked = false;

	for (int i = 0; i < 20; ++i)
	{
		auto& io = ImGui::GetIO();
		io.MousePos.x = 150;
		io.MousePos.y = 140;

		if (i >= 8 && i <= 10)
		{
			io.MouseDown[0] = true;
		}
		else
		{
			io.MouseDown[0] = false;
		}
		

		ImGui::NewFrame();

		if (io.MouseClicked[0])
		{
			printf("%d: Mouse clicked at [%f,%f]: %u\n", i, io.MouseClickedPos[0].x, io.MouseClickedPos[0].y, io.MouseClickedCount[0]);
		}
		else if (io.MouseReleased[0])
		{ 
			printf("%d: Mouse released at [%f,%f]: %u\n", i, io.MouseClickedPos[0].x, io.MouseClickedPos[0].y, io.MouseClickedCount[0]);
		}

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(5000, 5000));
		//ImGui::SetNextWindowFocus();

		{
			bool opened = true;
			if (ImGui::Begin("TestWindow", &opened))
			{
				if (ImGui::Button("TestButton", ImVec2(1000, 1000)))
				{
					printf("Button clicked!\n");
					buttonClicked = true;
				}

				if (i == 0)
				{
					auto posMin = ImGui::GetItemRectMin();
					auto posMax = ImGui::GetItemRectMax();
					printf("Button: [%f,%f] - [%f,%f]\n", posMin.x, posMin.y, posMax.x, posMax.y);
				}
			}

			ImGui::End();
		}

		if (i < 11)
		{
	 		EXPECT_FALSE(buttonClicked) << i;
		}
		else
		{
			EXPECT_TRUE(buttonClicked) << i;
		}

		std::this_thread::sleep_for(std::chrono::duration<uint32_t, std::milli>(10));
		ImGui::Render();
	}
}