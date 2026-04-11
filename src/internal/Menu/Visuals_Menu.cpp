#pragma once
#define NOMINMAX
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../Config/Configs.h"
#include "ConfigImGui.h"
#include "Visuals_Menu.h"
#include "../Util/Util.h"
#include "../../external/SDK/SDK_Headers.hpp"

namespace g_DrawImGui {
    void Visuals_Menu() {
        if (ImGui::BeginTabItem(U8("视觉"))) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 14.0f));
            BeginTabRegion("VisualsRegion");

            ImGui::TextColored(ThemeColors::GetAccent(), U8("全局设置"));
            DrawAnimatedSeparator();
            DrawColorPickerRow(U8("方框"), &g_Config::bDrawBox, "BoxCol1", g_Config::BoxColor);
            DrawColorPickerRow(U8("名称"), &g_Config::bDrawName, "NameCol1", g_Config::NameColor);
            DrawCustomCheckbox(U8("血量"), &g_Config::bDrawHealthBar);
            DrawColorPickerRow(U8("眩晕"), &g_Config::bDrawTorpor, "TorporCol", g_Config::TorporColor);
            DrawColorPickerRow(U8("尸体"), &g_Config::bDrawRagdoll, "RagdollCol", g_Config::RagdollColor);
            DrawAnimatedSeparator();

            ImGui::TextColored(ThemeColors::GetAccent(), U8("额外信息"));
            DrawAnimatedSeparator();
            DrawColorPickerRow(U8("距离"), &g_Config::bDrawDistance, "DistCol1", g_Config::DistanceColor);
            DrawColorPickerRow(U8("显示瞄准点"), &g_Config::bDrawAimPoints, "AimPointsCol1", g_Config::AimPointsColor);

            // 2026/4/11 @zetsr
            // 应该是之前移除aimbot的时候被注释掉了，我们可能并不需要分别做骨骼和瞄准点的cb，使用复选框或者单选框应该更好，以后可以加个glow类型的，不过骨骼确实很好看
            // DrawColorPickerRow(U8("显示瞄准骨骼"), &g_Config::bDrawAimSkeleton, "AimSkeletonCol1", g_Config::AimSkeletonColor);
            DrawAnimatedSeparator();

            ImGui::TextColored(ThemeColors::GetAccent(), U8("世界信息"));
            DrawAnimatedSeparator();

            DrawColorPickerRow(U8("掉落的物品"), &g_Config::bDrawDroppedItems, "DroppedItemNameCol", g_Config::DroppedItemNameColor);
            if (g_Config::bDrawDroppedItems) {
                // DrawCustomColorPicker("DroppedItemDistanceCol", g_Config::DroppedItemDistanceColor, U8("物品距离"));
                DrawCustomColorPicker("DroppedItemPiledCol", g_Config::DroppedItemPiledColor, U8("堆叠颜色"));
                DrawCustomColorPicker("DroppedItemCryopodCol", g_Config::DroppedItemCryopodColor, U8("低温仓颜色"));
                DrawCustomColorPicker("DroppedItemEggCol", g_Config::DroppedItemEggColor, U8("蛋颜色"));
                DrawCustomColorPicker("DroppedItemMeatCol", g_Config::DroppedItemMeatColor, U8("肉类颜色"));
                DrawCustomColorPicker("DroppedItemSpoiledMeatCol", g_Config::DroppedItemSpoiledMeatColor, U8("腐肉颜色"));
                DrawCustomColorPicker("DroppedItemWoodCol", g_Config::DroppedItemWoodColor, U8("木头颜色"));
                DrawCustomColorPicker("DroppedItemThatchCol", g_Config::DroppedItemThatchColor, U8("茅草颜色"));
                DrawCustomColorPicker("DroppedItemMetalCol", g_Config::DroppedItemMetalColor, U8("金属颜色"));
                DrawCustomColorPicker("DroppedItemStoneCol", g_Config::DroppedItemStoneColor, U8("石头颜色"));
                DrawCustomColorPicker("DroppedItemCrystalCol", g_Config::DroppedItemCrystalColor, U8("水晶颜色"));
                DrawCustomColorPicker("DroppedItemGemCol", g_Config::DroppedItemGemColor, U8("宝石颜色"));
                DrawCustomColorPicker("DroppedItemPearlCol", g_Config::DroppedItemPearlColor, U8("珍珠颜色"));
                DrawCustomColorPicker("DroppedItemHideCol", g_Config::DroppedItemHideColor, U8("兽皮颜色"));
                DrawCustomColorPicker("DroppedItemPeltCol", g_Config::DroppedItemPeltColor, U8("毛皮颜色"));
                DrawCustomColorPicker("DroppedItemKeratinCol", g_Config::DroppedItemKeratinColor, U8("角质颜色"));
                DrawCustomColorPicker("DroppedItemChitinCol", g_Config::DroppedItemChitinColor, U8("甲壳素颜色"));
                DrawCustomColorPicker("DroppedItemCorruptedPolymerCol", g_Config::DroppedItemCorruptedPolymerColor, U8("腐化瘤颜色"));
                DrawCustomColorPicker("DroppedItemPolymer_OrganicCol", g_Config::DroppedItemPolymer_OrganicColor, U8("有机聚合物颜色"));
                DrawCustomColorPicker("DroppedItemPolymerCol", g_Config::DroppedItemPolymerColor, U8("聚合物颜色"));
                DrawCustomSliderFloat(U8("物品显示距离"), &g_Config::DroppedItemMaxDistance, 1.0f, 500.0f, "%.0f", 1.0f, "m");
            }
            DrawAnimatedSeparator();

            // DrawColorPickerRow(U8("显示建筑"), &g_Config::bDrawStructures, "StructureNameCol", g_Config::StructureNameColor);
            DrawCustomCheckbox(U8("显示建筑"), &g_Config::bDrawStructures);
            if (g_Config::bDrawStructures) {
                // DrawCustomColorPicker("StructureOwnerCol", g_Config::StructureOwnerColor, U8("建筑所有者"));
                // DrawCustomColorPicker("StructureDistanceCol", g_Config::StructureDistanceColor, U8("建筑距离"));
                DrawCustomCheckbox(U8("仅显示敌人建筑"), &g_Config::bOnlyDrawStructuresEnemy);
                DrawCustomSliderFloat(U8("建筑显示距离"), &g_Config::StructureMaxDistance, 1.0f, 10000.0f, "%.0f", 1.0f, "m");
            }
            DrawAnimatedSeparator();

            DrawColorPickerRow(U8("显示水源"), &g_Config::bDrawWater, "WaterNameCol", g_Config::WaterNameColor);
            if (g_Config::bDrawWater) {
                // DrawCustomColorPicker("WaterDistanceCol", g_Config::WaterDistanceColor, U8("水源距离"));
                DrawCustomSliderFloat(U8("显示最近水源数量"), &g_Config::WaterMaxCount, 1.0f, 10.0f, "%.0f", 1.0f, U8("个"));
            }
            DrawAnimatedSeparator();

            /*
            DrawColorPickerRow(U8("显示视野外的威胁"), &g_Config::bEnableOOF, "OOFCol1", g_Config::OOFColor);
            if (g_Config::bEnableOOF) {
                float avail = ImGui::GetContentRegionAvail().x;
                float frame_h = ImGui::GetFrameHeight();
                float pad_y = ImGui::GetStyle().FramePadding.y;
                float btn_sz = frame_h - pad_y * 2.0f;
                if (btn_sz <= 0.0f) btn_sz = frame_h * 0.8f;
                float target = ImGui::GetCursorPosX() + avail - btn_sz;
                ImGui::SetCursorPosX(target);
                ImGui::Dummy(ImVec2(0, 6.0f));
                DrawCustomSliderFloat(U8("箭头尺寸"), &g_Config::OOFSize, 5.0f, 30.0f, "%.1f", 1.0f);
                DrawCustomSliderFloat(U8("屏幕半径"), &g_Config::OOFRadius, 0.5f, 1.00f, "%.2f", 0.01f);
                DrawCustomSliderFloat(U8("呼吸速度"), &g_Config::OOFBreathSpeed, 0.1f, 5.0f, "%.1f", 0.1f);
                DrawCustomSliderFloat(U8("最小透明度"), &g_Config::OOFMinAlpha, 0.1f, 0.9f, "%.2f", 0.01f);
                DrawCustomSliderFloat(U8("最大透明度"), &g_Config::OOFMaxAlpha, 0.2f, 1.0f, "%.2f", 0.01f);
            }
            */

            EndTabRegion();
            ImGui::PopStyleVar();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(U8("队友"))) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 14.0f));
            BeginTabRegion("TeamRegion");

            ImGui::TextColored(ThemeColors::GetAccent(), U8("队友设置"));
            DrawAnimatedSeparator();
            DrawColorPickerRow(U8("方框##Team"), &g_Config::bDrawBoxTeam, "BoxColTeam", g_Config::BoxColorTeam);
            DrawColorPickerRow(U8("名称##Team"), &g_Config::bDrawNameTeam, "NameColTeam", g_Config::NameColorTeam);
            DrawCustomCheckbox(U8("血量##Team"), &g_Config::bDrawHealthBarTeam);
            DrawColorPickerRow(U8("眩晕##Team"), &g_Config::bDrawTorporTeam, "TorporColTeam", g_Config::TorporColorTeam);
            DrawColorPickerRow(U8("尸体##Team"), &g_Config::bDrawRagdollTeam, "RagdollColTeam", g_Config::RagdollColorTeam);
            DrawAnimatedSeparator();

            ImGui::TextColored(ThemeColors::GetAccent(), U8("额外信息"));
            DrawAnimatedSeparator();
            DrawColorPickerRow(U8("距离##Team"), &g_Config::bDrawDistanceTeam, "DistColTeam", g_Config::DistanceColorTeam);
            DrawAnimatedSeparator();

            EndTabRegion();
            ImGui::PopStyleVar();
            ImGui::EndTabItem();
        }
    }
}