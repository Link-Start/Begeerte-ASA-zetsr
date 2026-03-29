#pragma once
#define NOMINMAX  
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../Config/Configs.h"
#include "ConfigImGui.h"
#include "StructureList_Menu.h"
#include "../Util/Util.h"
#include "../../external/SDK/SDK_Headers.hpp"

namespace g_DrawImGui {
	void StructureList_Menu() {
		if (ImGui::BeginTabItem(U8("建筑列表"))) {
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 14.0f));
			BeginTabRegion("StructureListRegion");

			ImGui::TextColored(ThemeColors::ACCENT, U8("建筑列表"));
			DrawAnimatedSeparator();

			DrawCustomCheckbox(U8("应用筛选到全局视觉"), &g_Config::bEnableStructureFilter);
			ImGui::SameLine();

			ImGui::PushItemWidth(-1.0f);
			ImGui::InputTextWithHint("##StructureSearch", U8("输入建筑名称进行过滤，多个用逗号分隔 (如: 大门,墙,地板)..."), g_Config::structureSearchBuf, IM_ARRAYSIZE(g_Config::structureSearchBuf));
			ImGui::PopItemWidth();

			{
				std::string filter = g_Config::structureSearchBuf;
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
			if (ImGui::BeginChild("##StructureListChild", ImVec2(0, 0), true)) {

				SDK::UWorld* World = SDK::UWorld::GetWorld();
				SDK::APlayerController* LocalPC = g_Util::GetLocalPC();

				if (World && World->PersistentLevel && LocalPC && LocalPC->Pawn) {
					SDK::TArray<SDK::AActor*>& Actors = World->PersistentLevel->Actors;

					for (int i = 0; i < Actors.Num(); i++) {
						SDK::AActor* TargetActor = Actors[i];

						if (!TargetActor || TargetActor->bHidden) continue;
						if (!TargetActor->IsA(SDK::APrimalStructure::StaticClass())) continue;

						SDK::APrimalStructure* Structure = (SDK::APrimalStructure*)TargetActor;

						if (!Structure) continue;
						if (Structure->Health <= 0.0f) continue;

						std::string structureName = Structure->GetDescriptiveName().ToString();
						if (structureName.empty() || structureName == "None") {
							structureName = "Structure";
						}

						if (g_Util::IsStructureMatchMulti(structureName, g_Config::structureSearchBuf)) {
							float dist = (LocalPC && LocalPC->Pawn && TargetActor) ? LocalPC->Pawn->GetDistanceTo(TargetActor) * 0.01f : 0.0f;

							std::string ownerStr = Structure->OwnerName.ToString();
							if (ownerStr.empty() || ownerStr == "None") {
								ownerStr = "*";
							}

							float curHP = Structure->Health;
							float maxHP = Structure->MaxHealth;
							int hpPercent = (maxHP > 0) ? (int)((curHP / maxHP) * 100.0f) : 0;

							ImGui::Text("[%dm] %s - %s - %.0f/%.0f",
								(int)dist,
								structureName.c_str(),
								ownerStr.c_str(),
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