#pragma once
#define NOMINMAX
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../Config/Configs.h"
#include "../Util/Util.h"
#include "../Config/ConfigManager.h"
#include "../Lua/LuaManager.h"
#include "../Log/LogManager.h"
#include "Hack.h"

namespace g_Hack {
    static int32_t g_CurrentNoteIndex = 0;
    static const int32_t g_MaxNoteIndex = 200;

    void DumpServerInfo() {
        SDK::UWorld* World = SDK::UWorld::GetWorld();
        if (!World || !World->NetDriver || !World->NetDriver->ServerConnection) {
            g_LogManager::AddLog(255, 50, 55, 255, "当前不在服务器内");
            return;
        }

        SDK::UNetConnection* ServerConnection = World->NetDriver->ServerConnection;

        g_LogManager::AddLog(255, 200, 255, 255, std::format("[{}] [{}]", ServerConnection->GetFirstIP(), ServerConnection->GetPort()));
    }

    void Suicide(SDK::UWorld* World) {
        if (!World || !World->OwningGameInstance || World->OwningGameInstance->LocalPlayers.Num() == 0) return;

        SDK::ULocalPlayer* LP = World->OwningGameInstance->LocalPlayers[0];
        if (!LP || !LP->PlayerController) return;

        if (!LP->PlayerController->IsA(SDK::AShooterPlayerController::StaticClass())) return;
        SDK::AShooterPlayerController* PC = static_cast<SDK::AShooterPlayerController*>(LP->PlayerController);

        if (!PC) return;

        if (PC->Character && PC->Character->IsA(SDK::AShooterCharacter::StaticClass())) {
            SDK::AShooterCharacter* MyHuman = static_cast<SDK::AShooterCharacter*>(PC->Character);

            if (MyHuman) {
                MyHuman->BPSuicide();
            }
        }
    }

    void UnlockExplorerNotes(SDK::UWorld* World) {
        if (!World || !World->OwningGameInstance || World->OwningGameInstance->LocalPlayers.Num() == 0) return;

        SDK::ULocalPlayer* LP = World->OwningGameInstance->LocalPlayers[0];
        if (!LP || !LP->PlayerController || !LP->PlayerController->Pawn) return;

        if (!LP->PlayerController->IsA(SDK::AShooterPlayerController::StaticClass())) return;
        SDK::AShooterPlayerController* PC = static_cast<SDK::AShooterPlayerController*>(LP->PlayerController);

        if (!PC)  return;

        if (g_CurrentNoteIndex <= g_MaxNoteIndex) {
            PC->UnlockExplorerNote(g_CurrentNoteIndex, true, true);
            g_CurrentNoteIndex++;
        }
        else {
            g_CurrentNoteIndex = 0;
        }
    }

    void AutoFeed(SDK::UWorld* World) {
        if (!World || !World->OwningGameInstance || World->OwningGameInstance->LocalPlayers.Num() == 0) return;

        SDK::ULocalPlayer* LP = World->OwningGameInstance->LocalPlayers[0];
        if (!LP || !LP->PlayerController) return;

        SDK::AShooterPlayerController* PC = static_cast<SDK::AShooterPlayerController*>(LP->PlayerController);
        SDK::APrimalDinoCharacter* TargetDino = nullptr;

        if (PC->Pawn && PC->Pawn->IsA(SDK::APrimalDinoCharacter::StaticClass())) {
            TargetDino = static_cast<SDK::APrimalDinoCharacter*>(PC->Pawn);
        }

        else if (PC->Character && PC->Character->IsA(SDK::AShooterCharacter::StaticClass())) {
            SDK::AShooterCharacter* MyHuman = static_cast<SDK::AShooterCharacter*>(PC->Character);

            if (MyHuman) {
                TargetDino = MyHuman->GetRidingDino();
            }
        }

        // 只有找到恐龙时才执行喂食逻辑
        if (TargetDino) {
            g_Util::ProcessDinoFeed(PC, TargetDino);
        }
    }

    void SuperFlyer(SDK::UWorld* World) {
        if (!World || !World->OwningGameInstance || World->OwningGameInstance->LocalPlayers.Num() == 0) return;

        SDK::ULocalPlayer* LP = World->OwningGameInstance->LocalPlayers[0];
        if (!LP || !LP->PlayerController) return;

        SDK::AShooterPlayerController* PC = static_cast<SDK::AShooterPlayerController*>(LP->PlayerController);
        SDK::APrimalDinoCharacter* TargetDino = nullptr;

        if (PC->Pawn && PC->Pawn->IsA(SDK::APrimalDinoCharacter::StaticClass())) {
            TargetDino = static_cast<SDK::APrimalDinoCharacter*>(PC->Pawn);

            // 允许恐龙左右飞，倒飞
            TargetDino->bFlyerDinoAllowBackwardsFlight = true;
            TargetDino->bFlyerDinoAllowStrafing= true;
        }
    }

    void ServerRequestInventoryUseItem(SDK::AShooterPlayerController* PC, SDK::UPrimalInventoryComponent* InventoryComp, SDK::FItemNetID& ItemID) {
        PC->ServerRequestInventoryUseItem(InventoryComp, ItemID);
    }

    void ServerRequestRemoteDropAllItems(SDK::AShooterPlayerController* PC, SDK::UPrimalInventoryComponent* InventoryComp, SDK::FString& CurrentCustomFolderFilter, SDK::FString& CurrentNameFilter) {
        PC->ServerRequestRemoteDropAllItems(InventoryComp, CurrentCustomFolderFilter, CurrentNameFilter);
    }
}