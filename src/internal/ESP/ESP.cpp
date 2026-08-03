// ESP.cpp
#include "../../external/SDK/SDK_Headers.hpp"
#include "../../external/Shadow-Gui/include/Shadow.h"
#include "ESP.h"
#include "../Config/Configs.h"
#include "../Util/Util.h"
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>

/*
namespace {
    // 辅助绘制实心矩形的方法，利用Canvas的默认贴图
    void DrawFilledRect(SDK::UCanvas* Canvas, const SDK::FVector2D& Pos, const SDK::FVector2D& Size, const SDK::FLinearColor& Color) {
        if (!Canvas || !Canvas->DefaultTexture) return;
        Canvas->K2_DrawTexture(Canvas->DefaultTexture, Pos, Size, SDK::FVector2D{ 0, 0 }, SDK::FVector2D{ 1, 1 }, Color, SDK::EBlendMode::BLEND_Translucent, 0.0f, SDK::FVector2D{ 0, 0 });
    }
}
*/

namespace g_ESP {
    RelationType GetRelation(SDK::APrimalCharacter* TargetChar, SDK::APrimalCharacter* LocalChar) {
        if (!TargetChar || !LocalChar) return RelationType::Enemy;

        int targetTeam = TargetChar->TargetingTeam;
        int localTeam = LocalChar->TargetingTeam;

        if (targetTeam != 0 && targetTeam == localTeam) {
            return RelationType::Team;
        }

        return RelationType::Enemy;
    }

    void BarManager::AddBar(SDK::UCanvas* Canvas, BoxRect rect, float currentValue, float maxValue, Shadow::Color color, BarPos pos, BarOrientation orientation, float a) {
        if (!rect.valid || maxValue <= 0 || !Canvas) return;

        Shadow::ShadowDrawList* dl = Shadow::GetBackgroundDrawList();
        if (!dl) return;

        float percentage = std::clamp(currentValue / maxValue, 0.0f, 1.0f);

        Shadow::Color bgColor = { 0.0f, 0.0f, 0.0f, a * 0.7f };
        color.a = a;

        if (pos == BarPos::Left && orientation == BarOrientation::Vertical) {
            float boxHeight = rect.bottomRight.y - rect.topLeft.y;
            float barWidth = 2.5f;
            float barMargin = 4.0f + leftOffset;

            Shadow::Vec2 barBgTop = { rect.topLeft.x - barMargin - barWidth, rect.topLeft.y };
            Shadow::Vec2 barBgBottom = { rect.topLeft.x - barMargin, rect.bottomRight.y };

            Shadow::Vec2 bgPos = { barBgTop.x - 1.0f, barBgTop.y - 1.0f };
            Shadow::Vec2 bgSize = { (barBgBottom.x + 1.0f) - bgPos.x, (barBgBottom.y + 1.0f) - bgPos.y };
            dl->AddRectFilled(bgPos, bgSize, bgColor);

            if (currentValue > 0) {
                float dynamicHeight = boxHeight * percentage;
                Shadow::Vec2 fgPos = { barBgTop.x, barBgBottom.y - dynamicHeight };
                Shadow::Vec2 fgSize = { barWidth, dynamicHeight };
                dl->AddRectFilled(fgPos, fgSize, color);
            }

            leftOffset += barWidth + 2.0f;
        }
        else if (pos == BarPos::Right && orientation == BarOrientation::Vertical) {
            float boxHeight = rect.bottomRight.y - rect.topLeft.y;
            float barWidth = 2.5f;
            float barMargin = 4.0f + rightOffset;

            Shadow::Vec2 barBgTop = { rect.bottomRight.x + barMargin, rect.topLeft.y };
            Shadow::Vec2 barBgBottom = { rect.bottomRight.x + barMargin + barWidth, rect.bottomRight.y };

            Shadow::Vec2 bgPos = { barBgTop.x - 1.0f, barBgTop.y - 1.0f };
            Shadow::Vec2 bgSize = { (barBgBottom.x + 1.0f) - bgPos.x, (barBgBottom.y + 1.0f) - bgPos.y };
            dl->AddRectFilled(bgPos, bgSize, bgColor);

            if (currentValue > 0) {
                float dynamicHeight = boxHeight * percentage;
                Shadow::Vec2 fgPos = { barBgTop.x, barBgBottom.y - dynamicHeight };
                Shadow::Vec2 fgSize = { barWidth, dynamicHeight };
                dl->AddRectFilled(fgPos, fgSize, color);
            }

            rightOffset += barWidth + 2.0f;
        }
        else if (pos == BarPos::Top && orientation == BarOrientation::Horizontal) {
            float boxWidth = rect.bottomRight.x - rect.topLeft.x;
            float barHeight = 2.5f;
            float barMargin = 4.0f + topOffset;

            Shadow::Vec2 barBgLeft = { rect.topLeft.x, rect.topLeft.y - barMargin - barHeight };
            Shadow::Vec2 barBgRight = { rect.bottomRight.x, rect.topLeft.y - barMargin };

            Shadow::Vec2 bgPos = { barBgLeft.x - 1.0f, barBgLeft.y - 1.0f };
            Shadow::Vec2 bgSize = { (barBgRight.x + 1.0f) - bgPos.x, (barBgRight.y + 1.0f) - bgPos.y };
            dl->AddRectFilled(bgPos, bgSize, bgColor);

            if (currentValue > 0) {
                float dynamicWidth = boxWidth * percentage;
                Shadow::Vec2 fgPos = { barBgLeft.x, barBgLeft.y };
                Shadow::Vec2 fgSize = { dynamicWidth, barHeight };
                dl->AddRectFilled(fgPos, fgSize, color);
            }

            topOffset += barHeight + 2.0f;
        }
        else if (pos == BarPos::Bottom && orientation == BarOrientation::Horizontal) {
            float boxWidth = rect.bottomRight.x - rect.topLeft.x;
            float barHeight = 2.5f;
            float barMargin = 4.0f + bottomOffset;

            Shadow::Vec2 barBgLeft = { rect.topLeft.x, rect.bottomRight.y + barMargin };
            Shadow::Vec2 barBgRight = { rect.bottomRight.x, rect.bottomRight.y + barMargin + barHeight };

            Shadow::Vec2 bgPos = { barBgLeft.x - 1.0f, barBgLeft.y - 1.0f };
            Shadow::Vec2 bgSize = { (barBgRight.x + 1.0f) - bgPos.x, (barBgRight.y + 1.0f) - bgPos.y };
            dl->AddRectFilled(bgPos, bgSize, bgColor);

            if (currentValue > 0) {
                float dynamicWidth = boxWidth * percentage;
                Shadow::Vec2 fgPos = { barBgLeft.x, barBgLeft.y };
                Shadow::Vec2 fgSize = { dynamicWidth, barHeight };
                dl->AddRectFilled(fgPos, fgSize, color);
            }

            bottomOffset += barHeight + 2.0f;
        }
    }

    void FlagManager::AddFlag(SDK::UCanvas* Canvas, BoxRect rect, const std::string& text, Shadow::Color color, FlagPos pos, float alphaMult, const BarManager* barMgr) {
        if (!rect.valid || text.empty() || !Canvas) return;

        Shadow::ShadowDrawList* dl = Shadow::GetBackgroundDrawList();
        if (!dl) return;

        static SDK::UFont* SansationBold18 = nullptr;
        static SDK::UFont* OpenSansRegular12 = nullptr;

        if (!SansationBold18) {
            SDK::UObject* _Font = SDK::UObject::FindObject("Font SansationBold18.SansationBold18");
            if (_Font && _Font->IsA(SDK::UFont::StaticClass())) SansationBold18 = (SDK::UFont*)_Font;
        }
        if (!OpenSansRegular12) {
            SDK::UObject* _Font = SDK::UObject::FindObject("Font OpenSansRegular12.OpenSansRegular12");
            if (_Font && _Font->IsA(SDK::UFont::StaticClass())) OpenSansRegular12 = (SDK::UFont*)_Font;
        }

        if (!SansationBold18 || !OpenSansRegular12) return;
        SDK::UFont* currentFont = (pos == FlagPos::Top) ? SansationBold18 : OpenSansRegular12;

        // 缩放字体
        Shadow::Vec2 scaleSize = { 1.0f * g_Config::ESPScale, 1.0f * g_Config::ESPScale };

        // 获取精确宽高
        Shadow::PushFont(currentFont, g_Config::ESPScale);
        Shadow::Vec2 textSize = Shadow::MeasureTextSize(text);
        Shadow::PopFont();

        Shadow::Vec2 drawPos;

        // 【关键修复2】：补全全部四个方向的 barOffset（解决文字和血条重叠的问题）
        if (pos == FlagPos::Right) {
            float barOffset = barMgr ? barMgr->GetRightOffset() : 0.0f;
            drawPos = { (float)(rect.bottomRight.x + 5.0f + barOffset), (float)(rect.topLeft.y + rightY) };
            rightY += textSize.y + 1.0f;
        }
        else if (pos == FlagPos::Left) {
            float barOffset = barMgr ? barMgr->GetLeftOffset() : 0.0f;
            drawPos = { (float)(rect.topLeft.x - 6.0f - barOffset - textSize.x), (float)(rect.topLeft.y + leftY) };
            leftY += textSize.y + 1.0f;
        }
        else if (pos == FlagPos::Top) {
            float barOffset = barMgr ? barMgr->GetTopOffset() : 0.0f;
            float centerX = (rect.topLeft.x + rect.bottomRight.x) * 0.5f;
            drawPos = { (float)(centerX - textSize.x * 0.5f), (float)(rect.topLeft.y - barOffset - topY - textSize.y - 3.0f) };
            topY += textSize.y + 1.0f;
        }
        else if (pos == FlagPos::Bottom) {
            float barOffset = barMgr ? barMgr->GetBottomOffset() : 0.0f;
            float centerX = (rect.topLeft.x + rect.bottomRight.x) * 0.5f;
            drawPos = { (float)(centerX - textSize.x * 0.5f), (float)(rect.bottomRight.y + barOffset + bottomY + 3.0f) };
            bottomY += textSize.y + 1.0f;
        }

        color.a *= alphaMult;
        Shadow::Color shadowCol = { 0.0f, 0.0f, 0.0f, color.a };

        // 渲染文字
        dl->AddText(currentFont, g_Config::ESPScale, shadowCol, shadowCol, drawPos, color, text);
        // Canvas->K2_DrawText(currentFont, fText, drawPos, scaleSize, color, 0.0f, shadowCol, SDK::FVector2D{ 1.0f, 1.0f }, false, false, true, shadowCol);
    }

    BoxRect GetBox(SDK::UCanvas* Canvas, SDK::AActor* entity, float width_scale) {
        BoxRect rect;
        rect.valid = false;
        if (!entity || entity->bHidden) return rect;
        auto PC = g_Util::GetLocalPC();
        if (!PC) return rect;

        SDK::FVector origin, extent;
        entity->GetActorBounds(true, &origin, &extent, false);

        SDK::FVector worldTop = { origin.X, origin.Y, origin.Z + extent.Z };
        SDK::FVector worldBottom = { origin.X, origin.Y, origin.Z - extent.Z };
        SDK::FVector2D screenTop, screenBottom;

        if (PC->ProjectWorldLocationToScreen(worldTop, &screenTop, false) &&
            PC->ProjectWorldLocationToScreen(worldBottom, &screenBottom, false)) {

            float height = std::abs(screenBottom.Y - screenTop.Y);
            float width = height * width_scale;
            rect.topLeft = { (float)(screenTop.X - width * 0.5f), (float)screenTop.Y };
            rect.bottomRight = { (float)(screenTop.X + width * 0.5f), (float)screenBottom.Y };
            rect.valid = true;
        }

        return rect;
    }

    void DrawBox(SDK::UCanvas* Canvas, const BoxRect& rect, Shadow::Color color, float alpha) {
        if (!rect.valid || alpha < 0.001f || !Canvas) return;

        Shadow::ShadowDrawList* dl = Shadow::GetBackgroundDrawList();
        if (!dl) return;

        color.a *= alpha;

        float x = rect.topLeft.x;
        float y = rect.topLeft.y;
        float w = rect.bottomRight.x - rect.topLeft.x;
        float h = rect.bottomRight.y - rect.topLeft.y;
        float thickness = 1.0f; // 边框线条粗细

        // 1. 绘制黑色外边框阴影
        Shadow::Color shadowCol = { 0.0f, 0.0f, 0.0f, color.a };
        float s_t = thickness + 2.0f; // 阴影比主线稍微粗一点点
        dl->AddRectFilled( { x - 1.0f, y - 1.0f }, { w + 2.0f, s_t }, shadowCol); // 顶
        dl->AddRectFilled( { x - 1.0f, y + h - thickness - 1.0f }, { w + 2.0f, s_t }, shadowCol); // 底
        dl->AddRectFilled( { x - 1.0f, y - 1.0f }, { s_t, h + 2.0f }, shadowCol); // 左
        dl->AddRectFilled( { x + w - thickness - 1.0f, y - 1.0f }, { s_t, h + 2.0f }, shadowCol); // 右

        // 2. 绘制彩色主边框
        dl->AddRectFilled( { x, y }, { w, thickness }, color); // 顶
        dl->AddRectFilled( { x, y + h - thickness }, { w, thickness }, color); // 底
        dl->AddRectFilled( { x, y }, { thickness, h }, color); // 左
        dl->AddRectFilled( { x + w - thickness, y }, { thickness, h }, color); // 右
    }

    void DrawHealthBar(SDK::UCanvas* Canvas, BoxRect rect, float healthPercent, float maxHealth, float a) {
        if (!rect.valid || maxHealth <= 0 || !Canvas) return;

        Shadow::ShadowDrawList* dl = Shadow::GetBackgroundDrawList();
        if (!dl) return;

        float boxHeight = rect.bottomRight.y - rect.topLeft.y;
        float barWidth = 2.5f;
        float barMargin = 4.0f;
        Shadow::Vec2 barBgTop = { (float)(rect.topLeft.x - barMargin - barWidth), (float)rect.topLeft.y };
        Shadow::Vec2 barBgBottom = { (float)(rect.topLeft.x - barMargin), (float)rect.bottomRight.y };

        Shadow::Vec2 bgPos = { (float)(barBgTop.x - 1.0f), (float)(barBgTop.y - 1.0f) };
        Shadow::Vec2 bgSize = { (float)((barBgBottom.x + 1.0f) - bgPos.x), (float)((barBgBottom.y + 1.0f) - bgPos.y) };
        dl->AddRectFilled(bgPos, bgSize, { 0.0f, 0.0f, 0.0f, a * 0.7f });

        float percentage = std::clamp(healthPercent / maxHealth, 0.0f, 1.0f);

        float colr, colg, colb, cola;
        if (percentage > 0.5f)
            colr, colg, colb, cola = (1.0f - percentage) * 2.0f, 1.0f, 0.0f, a;
        else
            colr, colg, colb, cola = 1.0f, percentage * 2.0f, 0.0f, a;

        if (healthPercent > 0) {
            float dynamicHeight = boxHeight * percentage;
            Shadow::Vec2 fgPos = { (float)barBgTop.x, (float)(barBgBottom.y - dynamicHeight) };
            Shadow::Vec2 fgSize = { barWidth, dynamicHeight };
            dl->AddRectFilled(fgPos, fgSize, { colr, colg, colb, cola });
        }
    }
}