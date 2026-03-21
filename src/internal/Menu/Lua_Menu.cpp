#pragma once
#define NOMINMAX
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../Config/Configs.h"
#include "../Lua/LuaManager.h"
#include "ConfigImGui.h"
#include "Lua_Menu.h"
#include "../Util/Util.h"
#include "../../external/SDK/SDK_Headers.hpp"

namespace g_DrawImGui {
    void Lua_Menu() {
		LuaManager::Get().Update();
		if (ImGui::BeginTabItem(U8("脚本"))) {
			auto& mgr = LuaManager::Get();
			auto& scripts = mgr.GetScripts();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 14.0f));
			BeginTabRegion("LuaManagerRegion");

			ImGui::TextColored(ThemeColors::ACCENT, U8("脚本管理系统"));
			DrawAnimatedSeparator();

			if (ImGui::Button(U8("刷新列表"))) {
				mgr.RefreshFileList();
			}
			ImGui::SameLine();
			if (ImGui::Button(U8("打开目录"))) {
				std::string path = mgr.GetScriptDir();
				ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
			}

			ImGui::Spacing();

			if (ImGui::BeginChild("##LuaScriptListChild", ImVec2(0, 300), true)) {
				if (scripts.empty()) {
					ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5 - 10);
					// ImGui::TextDisabled(U8("无可用脚本"));
				}
				else {
					for (int i = 0; i < (int)scripts.size(); i++) {
						auto& script = scripts[i];
						ImGui::PushID(i);

						bool isActive = script.isLoaded;
						if (isActive) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
						}
						else {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
						}

						std::string prefix = isActive ? U8("* ") : U8("");
						std::string displayName = prefix + script.name;

						if (CustomSelectable(displayName.c_str(), false, 8.0f) /*ImGui::Selectable(displayName.c_str(), false, ImGuiSelectableFlags_SpanAllColumns)*/) {
							bool targetState = !script.isLoaded;
							mgr.SetScriptState(i, targetState);
						}

						ImGui::PopStyleColor();

						if (script.hasError) {
							ImGui::SameLine(ImGui::GetWindowWidth() - 35);
							ImGui::TextDisabled("[!] ");

							if (ImGui::IsItemHovered()) {
								ImGui::BeginTooltip();
								ImGui::TextUnformatted(U8("右键点击图标以复制错误详情"));
								ImGui::Separator();
								ImGui::TextUnformatted(script.lastError.c_str());
								ImGui::EndTooltip();
							}

							std::string popupId = "ErrorPopup_" + std::to_string(i);
							if (ImGui::BeginPopupContextItem(popupId.c_str())) {
								if (CustomSelectable(U8("复制错误信息"), false, 8.0f) /*ImGui::Selectable(U8("复制错误信息"))*/) {
									ImGui::SetClipboardText(script.lastError.c_str());
								}
								ImGui::EndPopup();
							}
						}

						ImGui::PopID();
					}
				}
				ImGui::EndChild();
			}

			DrawAnimatedSeparator();
			EndTabRegion();
			ImGui::PopStyleVar();
			ImGui::EndTabItem();
		}
    }
}