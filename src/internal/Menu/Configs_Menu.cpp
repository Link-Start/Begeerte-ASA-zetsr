#pragma once
#define NOMINMAX
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../Config/Configs.h"
#include "../Config/ConfigManager.h"
#include "ConfigImGui.h"
#include "Configs_Menu.h"
#include "../Util/Util.h"
#include "../../external/SDK/SDK_Headers.hpp"

namespace g_DrawImGui {
    void Configs_Menu() {
		if (ImGui::BeginTabItem(U8("配置"))) {
			auto& mgr = ConfigManager::Get();
			auto& configs = mgr.GetConfigs();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 14.0f));
			BeginTabRegion("ConfigManagerRegion");

			ImGui::TextColored(ThemeColors::GetAccent(), U8("配置管理系统"));
			DrawAnimatedSeparator();

			if (DrawCustomButton(U8("刷新列表"))) {
				mgr.RefreshFileList();
			}
			ImGui::SameLine();
			if (DrawCustomButton(U8("打开目录"))) {
				std::string path = mgr.GetConfigDir();
				ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
			}

			ImGui::SameLine(0.0f, 20.0f);

			static char configNameBuf[65] = { 0 };
			ImGui::SetNextItemWidth(150.0f);
			ImGui::InputTextWithHint("##ConfigName", U8("输入参数名称..."), configNameBuf, sizeof(configNameBuf));
			ImGui::SameLine();
			if (DrawCustomButton(U8("创建"))) {
				std::string configName(configNameBuf);
				if (configName.empty()) {

				}
				else if (!ConfigManager::IsValidConfigName(configName)) {

				}
				else {
					if (mgr.CreateConfig(configName)) {
						configNameBuf[0] = '\0';
					}
					else {

					}
				}
			}

			ImGui::Spacing();

			static int selectedConfigIdx = -1;
			if (ImGui::BeginChild("##ConfigListChild", ImVec2(0, 300), true)) {
				if (configs.empty()) {
					ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2 - 10);
					ImGui::TextDisabled(U8("暂无配置文件"));
				}
				else {
					for (int i = 0; i < (int)configs.size(); i++) {
						auto& config = configs[i];
						ImGui::PushID(i);

						bool isSelected = (selectedConfigIdx == i);
						if (CustomSelectable(config.name.c_str(), isSelected, 8.0f) /*ImGui::Selectable(config.name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)*/) {
							selectedConfigIdx = i;
						}

						ImGui::PopID();
					}
				}
				ImGui::EndChild();
			}

			ImGui::Spacing();

			ImGui::BeginDisabled(selectedConfigIdx < 0 || selectedConfigIdx >= (int)configs.size());

			if (DrawCustomButton(U8("加载配置"), ImVec2(120, 0))) {
				if (selectedConfigIdx >= 0 && selectedConfigIdx < (int)configs.size()) {
					auto& selectedConfig = configs[selectedConfigIdx];
					if (mgr.LoadConfig(selectedConfig.name)) {

					}
					else {

					}
				}
			}

			ImGui::SameLine();

			if (DrawCustomButton(U8("保存配置"), ImVec2(120, 0))) {
				if (selectedConfigIdx >= 0 && selectedConfigIdx < (int)configs.size()) {
					auto& selectedConfig = configs[selectedConfigIdx];
					if (mgr.SaveConfig(selectedConfig.name)) {

					}
					else {

					}
				}
			}

			ImGui::EndDisabled();

			DrawAnimatedSeparator();
			EndTabRegion();
			ImGui::PopStyleVar();
			ImGui::EndTabItem();
		}
    }
}