#pragma once
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../../external/SDK/SDK_Headers.hpp"
#include <cmath>
#include <algorithm>
#include <string>

namespace g_Util {
    static const float inv255 = 1.0f / 255.0f; 

    inline std::string ToLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return std::tolower(c);
            });
        return s;
    }

    inline ImU32 GetU32Color(float color[4]) {
        return ImGui::ColorConvertFloat4ToU32(*(ImVec4*)color);
    }

    inline ImU32 ToImColor(float r, float g, float b, float a) {
        return ImGui::ColorConvertFloat4ToU32(ImVec4(r * inv255, g * inv255, b * inv255, a * inv255));
    }

    inline ImU32 GetHealthColor(float healthPercent) {
        healthPercent = fmaxf(0.0f, fminf(healthPercent, 1.0f));

        float r = fminf(1.0f, 2.0f - 2.0f * healthPercent);
        float g = fminf(1.0f, 2.0f * healthPercent);

        return ToImColor(r * 255.0f, g * 255.0f, 0.0f, 255.0f);
    }

    inline bool IsEntityMatch(std::string displayName, std::string filter) {
        if (filter.empty()) return true;

        std::string nameLower = g_Util::ToLower(displayName);
        std::string filterLower = g_Util::ToLower(filter);

        size_t lastPos = 0;
        for (size_t j = 0; j < filterLower.length(); ) {
            unsigned char c = static_cast<unsigned char>(filterLower[j]);
            int charLen = 1;
            if (c >= 0xf0) charLen = 4;
            else if (c >= 0xe0) charLen = 3;
            else if (c >= 0xc0) charLen = 2;

            std::string sub = filterLower.substr(j, charLen);
            size_t foundPos = nameLower.find(sub, lastPos);

            if (foundPos == std::string::npos) return false;

            lastPos = foundPos + charLen;
            j += charLen;
        }
        return true;
    }

    inline bool IsStructureMatch(const std::string& structureName, const std::string& filter) {
        if (filter.empty()) return true;

        std::string lowerName = structureName;
        std::string lowerFilter = filter;

        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);

        return lowerName.find(lowerFilter) != std::string::npos;
    }

    inline SDK::APlayerController* GetLocalPC() {
        SDK::UWorld* World = SDK::UWorld::GetWorld();

        if (!World) {
            return nullptr;
        }

        if (!World->OwningGameInstance) {
            return nullptr;
        }

        if (World->OwningGameInstance->LocalPlayers.Num() == 0) {
            return nullptr;
        }

        auto LocalPlayer = World->OwningGameInstance->LocalPlayers[0];
        if (!LocalPlayer) {
            return nullptr;
        }

        auto PC = LocalPlayer->PlayerController;
        if (!PC) {
            return nullptr;
        }

        return PC;
    }

    inline float GetAngleDistance(SDK::FVector CamLoc, SDK::FVector TargetLoc, SDK::FRotator CamRot) {
        SDK::FVector Diff = { TargetLoc.X - CamLoc.X, TargetLoc.Y - CamLoc.Y, TargetLoc.Z - CamLoc.Z };
        SDK::FVector DirToTarget = SDK::UKismetMathLibrary::Normal(Diff, 0.0001f);
        SDK::FVector CamForward = SDK::UKismetMathLibrary::GetForwardVector(CamRot);
        float Dot = SDK::UKismetMathLibrary::Dot_VectorVector(DirToTarget, CamForward);
        Dot = SDK::UKismetMathLibrary::FClamp(Dot, -1.0f, 1.0f);
        return SDK::UKismetMathLibrary::DegAcos(Dot);
    }

    inline void MimicMouseClick(bool bPress) {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = bPress ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}