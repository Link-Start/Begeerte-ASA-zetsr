#pragma once
#define NOMINMAX  
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../Config/Configs.h"
#include "ConfigImGui.h"
#include "Misc_Menu.h"
#include "../Util/Util.h"
#include "../../external/SDK/SDK_Headers.hpp"

namespace g_DrawImGui {
	void Misc_Menu() {
		if (ImGui::BeginTabItem(U8("其他功能"))) {
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 14.0f));
			BeginTabRegion("MiscRegion");

			ImGui::TextColored(ThemeColors::ACCENT, U8("其他功能"));
			DrawAnimatedSeparator();

			DrawCustomCheckbox(U8("一键满级"), &g_Config::bUnlockExplorerNotes);
			DrawCustomCheckbox(U8("自动喂肉"), &g_Config::bAutoFeed);

			ImGui::SameLine();
			DrawAnimatedSeparator();

			EndTabRegion();
			ImGui::PopStyleVar();
			ImGui::EndTabItem();
		}
	}
}