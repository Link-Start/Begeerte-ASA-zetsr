#pragma once
#define NOMINMAX  
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../Config/Configs.h"
#include "ConfigImGui.h"
#include "Aimbot_Menu.h"
#include "../Util/Util.h"
#include "../../external/SDK/SDK_Headers.hpp"

namespace g_DrawImGui {
	void Aimbot_Menu() {
		if (ImGui::BeginTabItem(U8("自瞄"))) {
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 14.0f));
			BeginTabRegion("AimBotRegion");

			ImGui::TextColored(ThemeColors::ACCENT, U8("队友设置"));
			DrawAnimatedSeparator();
			DrawCustomCheckbox(U8("自动瞄准"), &g_Config::bAimbotEnabled);
			DrawCustomSliderFloat(U8("瞄准范围"), &g_Config::AimbotFOV, 0.1f, 180.0f, "%.1f", 0.1f, U8("°"));
			DrawCustomSliderFloat(U8("瞄准速度"), &g_Config::AimbotSmooth, 0.1f, 100.0f, "%.1f", 0.1f, "%");
			DrawCustomCheckbox(U8("自动射击"), &g_Config::bTriggerbotEnabled);
			DrawAnimatedSeparator();

			EndTabRegion();
			ImGui::PopStyleVar();
			ImGui::EndTabItem();
		}
	}
}