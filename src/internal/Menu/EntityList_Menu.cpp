#pragma once
#define NOMINMAX  
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../Config/Configs.h"
#include "ConfigImGui.h"
#include "EntityList_Menu.h"
#include "../Util/Util.h"
#include "../../external/SDK/SDK_Headers.hpp"

namespace g_DrawImGui {

	void EntityList_Menu() {
		if (ImGui::BeginTabItem(U8("生物列表"))) {
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 14.0f));
			BeginTabRegion("EntityListRegion");

			ImGui::TextColored(ThemeColors::ACCENT, U8("生物列表"));
			DrawAnimatedSeparator();

			ImGui::Checkbox(U8("应用筛选到全局视觉"), &g_Config::bEnableFilter);
			ImGui::SameLine();

			ImGui::PushItemWidth(-1.0f);
			ImGui::InputTextWithHint("##EntitySearch", U8("输入生物名称进行过滤，多个用逗号分隔 (如: 南巨,霸王龙,迅猛龙)..."), g_Config::entitySearchBuf, IM_ARRAYSIZE(g_Config::entitySearchBuf));
			ImGui::PopItemWidth();

			// 显示当前激活的筛选词列表
			{
				std::string filter = g_Config::entitySearchBuf;
				if (!filter.empty()) {
					std::vector<std::string> tokens = g_Util::SplitFilterTokens(filter);
					std::vector<std::string> validTokens;
					for (const auto& t : tokens) {
						if (!t.empty()) validTokens.push_back(t);
					}
					if (!validTokens.empty()) {
						ImGui::Spacing();
						ImGui::TextDisabled(U8("当前筛选 (%zu 项):"), validTokens.size());
						ImGui::SameLine();
						for (size_t i = 0; i < validTokens.size(); i++) {
							ImGui::TextColored(ThemeColors::ACCENT, "%s", validTokens[i].c_str());
							if (i + 1 < validTokens.size()) ImGui::SameLine();
						}
					}
				}
			}

			ImGui::Spacing();
			if (ImGui::BeginChild("##EntityListChild", ImVec2(0, 0), true)) {

				SDK::UWorld* World = SDK::UWorld::GetWorld();
				SDK::APlayerController* LocalPC = g_Util::GetLocalPC();

				if (World && World->PersistentLevel && LocalPC && LocalPC->Pawn) {
					SDK::TArray<SDK::AActor*>& Actors = World->PersistentLevel->Actors;
					SDK::AActor* LocalPawn = LocalPC->Pawn;

					for (int i = 0; i < Actors.Num(); i++) {
						SDK::AActor* TargetActor = Actors[i];

						if (!TargetActor || TargetActor == LocalPawn || TargetActor->bHidden) continue;
						if (!TargetActor->IsA(SDK::APrimalCharacter::StaticClass())) continue;

						SDK::APrimalCharacter* TargetChar = (SDK::APrimalCharacter*)TargetActor;
						if (!TargetChar) continue;
						if (TargetChar->IsDead()) continue;

						std::string gender = "?";
						if (LocalPC->Pawn && TargetActor && TargetChar) {
							gender = TargetActor->IsFemale() ? "F" : "M";
						}

						std::string displayName;
						if (gender != "?") {
							displayName = TargetChar->PlayerState ? TargetChar->PlayerState->GetPlayerName().ToString() + "-" + gender : TargetChar->GetDescriptiveName().ToString() + "-" + gender;
						}
						else {
							displayName = TargetChar->PlayerState ? TargetChar->PlayerState->GetPlayerName().ToString() : TargetChar->GetDescriptiveName().ToString();
						}

						if (displayName.empty() || displayName == "None") continue;

						// 使用多关键词 OR 匹配替换原有的单关键词匹配
						if (g_Util::IsEntityMatchMulti(displayName, g_Config::entitySearchBuf)) {
							float dist = (LocalPC && LocalPC->Pawn && TargetActor) ? LocalPC->Pawn->GetDistanceTo(TargetActor) * 0.01f : 0.0f;
							float curHP = TargetChar->GetHealth();
							float maxHP = TargetChar->GetMaxHealth();

							ImGui::Text("[%dm] %s - %.0f/%.0f",
								(int)dist,
								displayName.c_str(),
								curHP,
								maxHP);
						}
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