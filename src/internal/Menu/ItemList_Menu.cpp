#pragma once
#define NOMINMAX  
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../Config/Configs.h"
#include "ConfigImGui.h"
#include "ItemList_Menu.h"
#include "../Util/Util.h"
#include "../../external/SDK/SDK_Headers.hpp"
#include <map>
#include <string>
#include <vector>
namespace g_DrawImGui {

    struct MergedItem {
        std::string DisplayName;
        SDK::FString OriginalFName; // 用于丢弃时的 Filter
        int TotalQuantity = 0;
        SDK::UPrimalItem* LastItemPtr = nullptr; // 用于获取描述和使用物品（取最后一组）
    };

    void ItemList_Menu() {
        if (ImGui::BeginTabItem(U8("背包管理"))) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 14.0f));
            BeginTabRegion("ItemListRegion");

            ImGui::InputTextWithHint("##ItemSearch", U8("输入物品名称搜索..."), g_Config::itemSearchBuf, IM_ARRAYSIZE(g_Config::itemSearchBuf));

            DrawAnimatedSeparator();

            if (ImGui::BeginChild("##MergedItemList", ImVec2(0, 0), true)) {
                SDK::AShooterPlayerController* PC = (SDK::AShooterPlayerController*)g_Util::GetLocalPC();
                SDK::APawn* LocalPawn = PC ? PC->Pawn : nullptr;
                SDK::APrimalCharacter* Character = (SDK::APrimalCharacter*)LocalPawn;
                SDK::UPrimalInventoryComponent* Inv = Character ? Character->MyInventoryComponent : nullptr;

                if (PC && LocalPawn && Character && Inv->InventoryItems.Num() > 0) {

                    // 1. 数据归并处理
                    std::map<std::string, MergedItem> mergedMap;
                    SDK::TArray<SDK::UPrimalItem*>& Items = Inv->InventoryItems;

                    for (int i = 0; i < Items.Num(); i++) {
                        SDK::UPrimalItem* Item = Items[i];
                        if (!Item || !Item->DescriptiveNameBase) continue;

                        std::string name = Item->DescriptiveNameBase.ToString();
                        if (name.empty() || name == "None") continue;

                        // 搜索过滤判断
                        if (strlen(g_Config::itemSearchBuf) > 0 && name.find(g_Config::itemSearchBuf) == std::string::npos)
                            continue;

                        auto& m = mergedMap[name];
                        m.DisplayName = name;
                        m.OriginalFName = Item->DescriptiveNameBase;
                        m.TotalQuantity += Item->ItemQuantity;
                        m.LastItemPtr = Item; // 存一个指针用于获取描述
                    }

                    // 2. 渲染归并后的列表
                    int idx = 0;
                    for (auto const& [name, data] : mergedMap) {
                        idx++;
                        std::string label = data.DisplayName + " x" + std::to_string(data.TotalQuantity);

                        if (ImGui::Selectable((label + "##" + std::to_string(idx)).c_str())) {
                            ImGui::OpenPopup(("ItemPopup" + std::to_string(idx)).c_str());
                        }

                        // 3. 悬浮窗显示详细信息
                        if (ImGui::IsItemHovered() && data.LastItemPtr && data.LastItemPtr->CustomItemDescription) {
                            ImGui::BeginTooltip();
                            SDK::UPrimalItem* p = data.LastItemPtr;

                            ImGui::TextColored(ThemeColors::GetAccent(), U8("物品: %s"), data.DisplayName.c_str());
                            ImGui::Text(U8("总数量: %d"), data.TotalQuantity);

                            // 描述信息
                            std::string desc = p->CustomItemDescription.ToString();
                            if (desc.empty() || desc == "None") desc = p->ItemDescription.ToString();
                            if (!desc.empty() && desc != "None") {
                                ImGui::Separator();
                                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 20.0f);
                                ImGui::TextDisabled(U8("说明: %s"), desc.c_str());
                                ImGui::PopTextWrapPos();
                            }

                            // 制造者信息
                            if (!p->CrafterCharacterName.ToString().empty() && p->CrafterCharacterName.ToString() != "None") {
                                ImGui::Separator();
                                ImGui::Text(U8("制造人: %s"), p->CrafterCharacterName.ToString().c_str());
                                ImGui::Text(U8("部落: %s"), p->CrafterTribeName.ToString().c_str());
                            }
                            ImGui::EndTooltip();
                        }

                        // 4. 交互菜单
                        if (ImGui::BeginPopup(("ItemPopup" + std::to_string(idx)).c_str())) {
                            if (ImGui::MenuItem(U8("使用一个 (从当前堆叠)"))) {
                                if (data.LastItemPtr) {
                                    // 不再直接调用 PC->ServerRequestInventoryUseItem
                                    g_Config::useItemID = data.LastItemPtr->ItemID;
                                    g_Config::bUseItem = true;
                                }
                            }

                            if (ImGui::MenuItem(U8("丢弃该物品"))) {
                                // 不再直接调用 PC->ServerRequestRemoteDropAllItems
                                g_Config::dropItemID = data.LastItemPtr->ItemID;
                                g_Config::bDropItem = true;
                            }
                            ImGui::EndPopup();
                        }
                    }
                }
                else {
                    ImGui::TextDisabled(U8("背包中没有匹配的物品"));
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