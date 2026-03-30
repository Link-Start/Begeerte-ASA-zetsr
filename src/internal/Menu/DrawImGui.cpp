#define NOMINMAX  
#if defined(__cpp_char8_t)
#define U8(str) reinterpret_cast<const char*>(u8##str)
#else
#define U8(str) u8##str
#endif

#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../../external/SDK/SDK_Headers.hpp"
#include "../ESP/ESP.h"
#include "../Config/Configs.h"
#include "../ESP/DrawESP.h"
#include "ConfigImGui.h"
#include "DrawImGui.h"

// #include "Aimbot.h"
#include "Visuals_Menu.h"
#include "EntityList_Menu.h"
#include "StructureList_Menu.h"
#include "ItemList_Menu.h"
#include "Misc_Menu.h"
#include "Configs_Menu.h"
#include "Lua_Menu.h"

#include "../Log/LogManager.h"
#include "../Config/ConfigManager.h"
#include "../Lua/LuaManager.h"
#include "../Util/Util.h"

#include <cstdio>
#include <string>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <map>

namespace g_DrawImGui {
	float g_MenuAlpha = 0.0f;

	void MyImGuiDraw(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		if (!style_initialized) { SetupCustomImGuiStyle(); style_initialized = true; }
		const float FADE_SPEED = 5.0f;
		static bool g_PrevMenuState = false;
		bool isNowOpen = g_MDX12::g_MenuState::g_isOpen;

		if (isNowOpen && !g_PrevMenuState) {
			LuaManager::Get().RefreshFileList();
			ConfigManager::Get().RefreshFileList();
			LuaManager::Get().Lua_OnMenuOpen();
		}
		g_PrevMenuState = isNowOpen;

		float deltaTime = ImGui::GetIO().DeltaTime;

		if (g_MDX12::g_MenuState::g_isOpen) {
			g_MenuAlpha += deltaTime * FADE_SPEED;
			if (g_MenuAlpha > 1.0f) g_MenuAlpha = 1.0f;
		}
		else {
			g_MenuAlpha -= deltaTime * FADE_SPEED;
			if (g_MenuAlpha < 0.0f) g_MenuAlpha = 0.0f;
		}

		if (g_MenuAlpha > 0.001f) {
			LuaManager::Get().Lua_OnPaintMenu(g_MenuAlpha);

			ImGui::GetStyle().WindowMinSize = ImVec2(920.0f, 720.0f);
			ImGui::SetNextWindowSize(ImVec2(920, 720), ImGuiCond_FirstUseEver);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_MenuAlpha);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(22.0f, 22.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 18.0f);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(7.0f * g_Util::inv255, 8.0f * g_Util::inv255, 10.0f * g_Util::inv255, 0.97f));
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(110.0f * g_Util::inv255, 231.0f * g_Util::inv255, 183.0f * g_Util::inv255, 0.08f));

			ImGuiWindowFlags wFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
			ImGui::GetIO().ConfigWindowsResizeFromEdges = false;

			if (ImGui::Begin("Begeerte", nullptr, wFlags)) {

				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				ImVec2 title_pos = ImGui::GetWindowPos();
				float title_width = ImGui::GetWindowWidth();
				float time = ImGui::GetTime();

				draw_list->AddRectFilledMultiColor(
					ImVec2(title_pos.x, title_pos.y),
					ImVec2(title_pos.x + title_width, title_pos.y + 2.0f),
					IM_COL32(79, 214, 166, 0),
					IM_COL32(110, 231, 183, 180),
					IM_COL32(110, 231, 183, 180),
					IM_COL32(79, 214, 166, 0)
				);

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(22.0f, 11.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(1.0f, 1.0f, 1.0f, 0.03f));
				ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.43f, 0.91f, 0.72f, 0.1f));
				ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.43f, 0.91f, 0.72f, 0.18f));
				ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImVec4(1.0f, 1.0f, 1.0f, 0.03f));
				ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImVec4(1.0f, 1.0f, 1.0f, 0.05f));

				if (ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_None)) {

                    // Aimbot_Menu();
					Visuals_Menu();
					EntityList_Menu();
					StructureList_Menu();
					ItemList_Menu();
					Misc_Menu();
					Configs_Menu();
					Lua_Menu();

					ImGui::EndTabBar();

				}

				ImGui::PopStyleColor(5);
				ImGui::PopStyleVar(2);
			}

			ImGui::End();
			ImGui::PopStyleColor(2);
			ImGui::PopStyleVar(4);
		}

		// g_Aimbot::Tick();
		g_DrawESP::DrawESP();
		g_LogManager::Run();
		LuaManager::Get().Lua_OnPaint();
	}
}