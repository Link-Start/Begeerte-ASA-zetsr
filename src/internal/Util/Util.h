#pragma once
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../../external/SDK/SDK_Headers.hpp"
#include <cmath>
#include <algorithm>
#include <string>
#include <sstream>

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

    inline bool IsCookedMeat(SDK::UPrimalItem* Item) {
        if (!Item || !Item->Class) return false;
        std::string name = Item->Class->GetName();
        return name.find("PrimalItemConsumable_CookedMeat") != std::string::npos;
    }

    inline bool IsRawMeat(SDK::UPrimalItem* Item) {
        if (!Item || !Item->Class) return false;
        std::string name = Item->Class->GetName();
        return name.find("PrimalItemConsumable_RawMeat") != std::string::npos;
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

    inline static std::vector<std::string> SplitFilterTokens(const std::string& filter) {
        std::vector<std::string> tokens;
        std::stringstream ss(filter);
        std::string token;
        while (std::getline(ss, token, ',')) {
            size_t start = token.find_first_not_of(" \t");
            size_t end = token.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                tokens.push_back(token.substr(start, end - start + 1));
            }
            else if (start != std::string::npos) {
                tokens.push_back(token.substr(start));
            }
        }
        return tokens;
    }

    inline static bool IsEntityMatchMulti(const std::string& displayName, const char* filterBuf) {
        std::string filter = filterBuf;
        if (filter.empty()) return true;

        std::vector<std::string> tokens = SplitFilterTokens(filter);
        if (tokens.empty()) return true;

        for (const std::string& token : tokens) {
            if (token.empty()) continue;
            if (g_Util::IsEntityMatch(displayName, token.c_str())) return true;
        }
        return false;
    }

    inline bool IsStructureMatch(const std::string& structureName, const std::string& filter) {
        if (filter.empty()) return true;

        std::string lowerName = structureName;
        std::string lowerFilter = filter;

        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);

        return lowerName.find(lowerFilter) != std::string::npos;
    }

    static bool IsStructureMatchMulti(const std::string& structureName, const char* filterBuf) {
        std::string filter = filterBuf;
        if (filter.empty()) return true;

        std::vector<std::string> tokens = SplitFilterTokens(filter);
        if (tokens.empty()) return true;

        for (const std::string& token : tokens) {
            if (token.empty()) continue;
            if (g_Util::IsStructureMatch(structureName, token.c_str())) return true;
        }
        return false;
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

    inline void ProcessDinoFeed(SDK::AShooterPlayerController* PC, SDK::APrimalDinoCharacter* Dino) {
        if (!Dino || Dino->IsDead()) return;

        float curHP = Dino->GetHealth();
        float maxHP = Dino->GetMaxHealth();
        if (maxHP <= 0.0f || curHP >= (maxHP - 1.0f)) return;

        SDK::UPrimalInventoryComponent* Inv = Dino->MyInventoryComponent;
        if (!Inv) return;

        SDK::TArray<SDK::UPrimalItem*>& Items = Inv->InventoryItems;
        SDK::UPrimalItem* BestMeat = nullptr;

        // 寻找食物：优先熟肉，其次生肉
        for (int i = 0; i < Items.Num(); i++) {
            SDK::UPrimalItem* Item = Items[i];
            if (!Item || !Item->Class) continue;

            if (IsCookedMeat(Item)) {
                BestMeat = Item;
                break; // 熟肉回血多，找到直接跳出
            }
            if (IsRawMeat(Item)) {
                BestMeat = Item; // 先记下生肉，继续看有没有熟肉
            }
        }

        if (BestMeat) {
            // 使用针对龙的 RPC
            PC->ServerRequestInventoryUseItem(Inv, BestMeat->ItemID);
        }
    }
}