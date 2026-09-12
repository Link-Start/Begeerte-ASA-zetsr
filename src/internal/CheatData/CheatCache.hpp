#pragma once
#include "../../external/CppSDK/SDK.hpp"
#include "../Util/Util.h"

/*
2026/8/23

uh，使用核显开发外挂简直是灾难，你永远无法得知你的用户是否会遇到性能损失。
上一次休息是什么时候呢，似乎已经是很久之前的事情了。

*/

namespace CheatCache {
	namespace PostRender {
		// 在运行 Update() 之外的线程使用这些缓存会发生检查后使用
		inline SDK::AShooterPlayerController* LocalSPC = nullptr;
		inline SDK::AShooterCharacter* LocalSC = nullptr;
		inline SDK::AShooterPlayerState* LocalSPS = nullptr;

		inline SDK::UWorld* World = nullptr;
		inline SDK::UEngine* Engine = nullptr;
		inline SDK::AShooterGameState* SGS = nullptr;

		__forceinline void Update() {
			Engine = nullptr;
			World = nullptr;
			SGS = nullptr;
			LocalSPC = nullptr;
			LocalSC = nullptr;
			LocalSPS = nullptr;

			SDK::UEngine* cEngine = SDK::UEngine::GetEngine();
			SDK::UWorld* cWorld = SDK::UWorld::GetWorld();
			SDK::APlayerController* cLocalPC = g_Util::GetLocalPC();

			if (cEngine) {
				Engine = cEngine;
			}

			if (cWorld) {
				World = cWorld;
			}

			if (cWorld && cWorld->GameState && cWorld->GameState->IsA(SDK::AShooterGameState::StaticClass())) {
				SGS = static_cast<SDK::AShooterGameState*>(cWorld->GameState);
			}

			if (cLocalPC && cLocalPC->IsA(SDK::AShooterPlayerController::StaticClass())) {
				LocalSPC = static_cast<SDK::AShooterPlayerController*>(cLocalPC);
			}

			if (cLocalPC && cLocalPC->Character && cLocalPC->Character->IsA(SDK::AShooterCharacter::StaticClass())) {
				LocalSC = static_cast<SDK::AShooterCharacter*>(cLocalPC->Character);
			}

			if (cLocalPC && cLocalPC->PlayerState && cLocalPC->PlayerState->IsA(SDK::AShooterPlayerState::StaticClass())) {
				LocalSPS = static_cast<SDK::AShooterPlayerState*>(cLocalPC->PlayerState);
			}
		}
	}

	namespace Tick {
		// 在运行 Update() 之外的线程使用这些缓存会发生检查后使用
		inline SDK::AShooterPlayerController* LocalSPC = nullptr;
		inline SDK::AShooterCharacter* LocalSC = nullptr;
		inline SDK::AShooterPlayerState* LocalSPS = nullptr;

		inline SDK::UWorld* World = nullptr;
		inline SDK::UEngine* Engine = nullptr;
		inline SDK::AShooterGameState* SGS = nullptr;

		__forceinline void Update() {
			Engine = nullptr;
			World = nullptr;
			SGS = nullptr;
			LocalSPC = nullptr;
			LocalSC = nullptr;
			LocalSPS = nullptr;

			SDK::UEngine* cEngine = SDK::UEngine::GetEngine();
			SDK::UWorld* cWorld = SDK::UWorld::GetWorld();
			SDK::APlayerController* cLocalPC = g_Util::GetLocalPC();

			if (cEngine) {
				Engine = cEngine;
			}

			if (cWorld) {
				World = cWorld;
			}

			if (cWorld && cWorld->GameState && cWorld->GameState->IsA(SDK::AShooterGameState::StaticClass())) {
				SGS = static_cast<SDK::AShooterGameState*>(cWorld->GameState);
			}

			if (cLocalPC && cLocalPC->IsA(SDK::AShooterPlayerController::StaticClass())) {
				LocalSPC = static_cast<SDK::AShooterPlayerController*>(cLocalPC);
			}

			if (cLocalPC && cLocalPC->Character && cLocalPC->Character->IsA(SDK::AShooterCharacter::StaticClass())) {
				LocalSC = static_cast<SDK::AShooterCharacter*>(cLocalPC->Character);
			}

			if (cLocalPC && cLocalPC->PlayerState && cLocalPC->PlayerState->IsA(SDK::AShooterPlayerState::StaticClass())) {
				LocalSPS = static_cast<SDK::AShooterPlayerState*>(cLocalPC->PlayerState);
			}
		}
	}

	namespace PhysicsRotation {
		// 在运行 Update() 之外的线程使用这些缓存会发生检查后使用
		inline SDK::AShooterPlayerController* LocalSPC = nullptr;
		inline SDK::AShooterCharacter* LocalSC = nullptr;
		inline SDK::AShooterPlayerState* LocalSPS = nullptr;

		inline SDK::UWorld* World = nullptr;
		inline SDK::UEngine* Engine = nullptr;
		inline SDK::AShooterGameState* SGS = nullptr;

		__forceinline void Update() {
			Engine = nullptr;
			World = nullptr;
			SGS = nullptr;
			LocalSPC = nullptr;
			LocalSC = nullptr;
			LocalSPS = nullptr;

			SDK::UEngine* cEngine = SDK::UEngine::GetEngine();
			SDK::UWorld* cWorld = SDK::UWorld::GetWorld();
			SDK::APlayerController* cLocalPC = g_Util::GetLocalPC();

			if (cEngine) {
				Engine = cEngine;
			}

			if (cWorld) {
				World = cWorld;
			}

			if (cWorld && cWorld->GameState && cWorld->GameState->IsA(SDK::AShooterGameState::StaticClass())) {
				SGS = static_cast<SDK::AShooterGameState*>(cWorld->GameState);
			}

			if (cLocalPC && cLocalPC->IsA(SDK::AShooterPlayerController::StaticClass())) {
				LocalSPC = static_cast<SDK::AShooterPlayerController*>(cLocalPC);
			}

			if (cLocalPC && cLocalPC->Character && cLocalPC->Character->IsA(SDK::AShooterCharacter::StaticClass())) {
				LocalSC = static_cast<SDK::AShooterCharacter*>(cLocalPC->Character);
			}

			if (cLocalPC && cLocalPC->PlayerState && cLocalPC->PlayerState->IsA(SDK::AShooterPlayerState::StaticClass())) {
				LocalSPS = static_cast<SDK::AShooterPlayerState*>(cLocalPC->PlayerState);
			}
		}
	}
}