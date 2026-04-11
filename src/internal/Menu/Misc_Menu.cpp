#pragma once
#define NOMINMAX
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/hooks.h"
#include "../Config/Configs.h"
#include "ConfigImGui.h"
#include "Misc_Menu.h"
#include "../Util/Util.h"
#include "../Hack/Hack.h"
#include "../../external/SDK/SDK_Headers.hpp"

namespace g_DrawImGui {
	void Misc_Menu() {
		if (ImGui::BeginTabItem(U8("其他功能"))) {
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 14.0f));
			BeginTabRegion("MiscRegion");

			ImGui::TextColored(ThemeColors::GetAccent(), U8("菜单设置"));
			DrawAnimatedSeparator();

			DrawCustomColorPicker("MenuCol", g_Config::MenuColor, U8("菜单颜色"));

			DrawCustomCheckbox(U8("锁定布局"), &g_Config::bMenuLockResize);
			ImGui::SameLine();

			if (DrawCustomButton(U8("重置布局"))) {
				g_Config::bMenuNeedReset = true;
			}

			DrawAnimatedSeparator();

			ImGui::TextColored(ThemeColors::GetAccent(), U8("其他功能"));
			DrawAnimatedSeparator();

			if (DrawCustomButton(U8("自杀"))) {
				g_Config::bSuicide = true;
			}

			ImGui::SameLine();
			if (DrawCustomButton(U8("卸载"))) {
				g_Hook::StopAllHooks();
			}

			ImGui::SameLine();
			if (DrawCustomButton(U8("获取服务器信息"))) {
				g_Hack::DumpServerInfo();
			}

			DrawAnimatedSeparator();

			DrawCustomCheckbox(U8("一键满级"), &g_Config::bUnlockExplorerNotes);
			DrawCustomCheckbox(U8("自动喂肉"), &g_Config::bAutoFeed);
			DrawCustomCheckbox(U8("飞行龙加强"), &g_Config::bSuperFlyer);
            DrawCustomCheckbox(U8("秒转头"), &g_Config::bForceTurn);

			DrawAnimatedSeparator();

			EndTabRegion();
			ImGui::PopStyleVar();
			ImGui::EndTabItem();
		}
	}
}