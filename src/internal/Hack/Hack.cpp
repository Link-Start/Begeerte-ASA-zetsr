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

    void ForceTurn(SDK::UMovementComponent* rcx, float DeltaTime) {
        // 2026/4/3 @zetsr
        // 在 PhysicsRotation.cpp 检查
        // 此外 if (!XXX) return 在 XXX 初始化为 false 的情况下似乎会意外进入后续分支，必须用 if (!XXX){ return; } 等有时间的时候调查
        // 
        // if (!g_Config::bForceTurn) return;

        SDK::APlayerController* LocalPC = g_Util::GetLocalPC();

        if (!LocalPC || !LocalPC->Pawn) return;

        SDK::AShooterCharacter* character = (SDK::AShooterCharacter*)LocalPC->Character;

        // 由于此函数是共用的，所以必须过滤掉除player与riding外的movement。最好的方法是每帧检查并hook虚函数
        // 可以增加一个过滤，只为恐龙启用
        if (!character || (uintptr_t)character->CharacterMovement != (uintptr_t)rcx || !LocalPC->PlayerCameraManager) {
            return;
        }

        SDK::FRotator rot = LocalPC->PlayerCameraManager->GetCameraRotation();

        // 开了之后会一直往前走
        rcx->K2_MoveUpdatedComponent(SDK::FVector{ 0,0,0 }, rot, nullptr, false, false);
    }
}