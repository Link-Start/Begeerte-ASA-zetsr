#pragma once
#include "../../external/CppSDK/SDK.hpp"
#include "../Util/Util.h"

/*
2026/8/23

uh，使用核显开发外挂简直是灾难，你永远无法得知你的用户是否会遇到性能损失。
上一次休息是什么时候呢，似乎已经是很久之前的事情了。

*/

namespace _DD {
	// 在运行 Update() 之外的线程使用这些缓存会发生检查后使用
	inline SDK::AShooterPlayerController* LocalSPC = nullptr;
	inline SDK::AShooterCharacter* LocalSC = nullptr;
	inline SDK::AShooterPlayerState* LocalSPS = nullptr;

	inline SDK::UWorld* World = nullptr;
	inline SDK::UEngine* Engine = nullptr;
	inline SDK::AShooterGameState* SGS = nullptr;

	__forceinline void Update() {
		_DD::Engine = nullptr;
		_DD::World = nullptr;
		_DD::SGS = nullptr;
		_DD::LocalSPC = nullptr;
		_DD::LocalSC = nullptr; 
		_DD::LocalSPS = nullptr;

		SDK::UEngine* Engine = SDK::UEngine::GetEngine();
		SDK::UWorld* World = SDK::UWorld::GetWorld();
		SDK::APlayerController* LocalPC = g_Util::GetLocalPC();

		if (Engine) {
			_DD::Engine = Engine;
		}

		if (World) {
		    _DD::World = World;
		}

		if (World && World->GameState && World->GameState->IsA(SDK::AShooterGameState::StaticClass())) {
			_DD::SGS = static_cast<SDK::AShooterGameState*>(World->GameState);
		}

		if (LocalPC && LocalPC->IsA(SDK::AShooterPlayerController::StaticClass())) {
			_DD::LocalSPC = static_cast<SDK::AShooterPlayerController*>(LocalPC);
		}

		if (LocalPC && LocalPC->Character && LocalPC->Character->IsA(SDK::AShooterCharacter::StaticClass())) {
			_DD::LocalSC = static_cast<SDK::AShooterCharacter*>(LocalPC->Character);
		}

		if (LocalPC && LocalPC->PlayerState && LocalPC->PlayerState->IsA(SDK::AShooterPlayerState::StaticClass())) {
			_DD::LocalSPS = static_cast<SDK::AShooterPlayerState*>(LocalPC->PlayerState);
		}
	}
}

namespace _TICK {
	// 在运行 Update() 之外的线程使用这些缓存会发生检查后使用
	inline SDK::AShooterPlayerController* LocalSPC = nullptr;
	inline SDK::AShooterCharacter* LocalSC = nullptr;
	inline SDK::AShooterPlayerState* LocalSPS = nullptr;

	inline SDK::UWorld* World = nullptr;
	inline SDK::UEngine* Engine = nullptr;
	inline SDK::AShooterGameState* SGS = nullptr;

	__forceinline void Update() {
		_TICK::Engine = nullptr;
		_TICK::World = nullptr;
		_TICK::SGS = nullptr;
		_TICK::LocalSPC = nullptr;
		_TICK::LocalSC = nullptr;
		_TICK::LocalSPS = nullptr;

		SDK::UEngine* Engine = SDK::UEngine::GetEngine();
		SDK::UWorld* World = SDK::UWorld::GetWorld();
		SDK::APlayerController* LocalPC = g_Util::GetLocalPC();

		if (Engine) {
			_TICK::Engine = Engine;
		}

		if (World) {
			_TICK::World = World;
		}

		if (World && World->GameState && World->GameState->IsA(SDK::AShooterGameState::StaticClass())) {
			_TICK::SGS = static_cast<SDK::AShooterGameState*>(World->GameState);
		}

		if (LocalPC && LocalPC->IsA(SDK::AShooterPlayerController::StaticClass())) {
			_TICK::LocalSPC = static_cast<SDK::AShooterPlayerController*>(LocalPC);
		}

		if (LocalPC && LocalPC->Character && LocalPC->Character->IsA(SDK::AShooterCharacter::StaticClass())) {
			_TICK::LocalSC = static_cast<SDK::AShooterCharacter*>(LocalPC->Character);
		}

		if (LocalPC && LocalPC->PlayerState && LocalPC->PlayerState->IsA(SDK::AShooterPlayerState::StaticClass())) {
			_TICK::LocalSPS = static_cast<SDK::AShooterPlayerState*>(LocalPC->PlayerState);
		}
	}
}

namespace _PR {
	// 在运行 Update() 之外的线程使用这些缓存会发生检查后使用
	inline SDK::AShooterPlayerController* LocalSPC = nullptr;
	inline SDK::AShooterCharacter* LocalSC = nullptr;
	inline SDK::AShooterPlayerState* LocalSPS = nullptr;

	inline SDK::UWorld* World = nullptr;
	inline SDK::UEngine* Engine = nullptr;
	inline SDK::AShooterGameState* SGS = nullptr;

	__forceinline void Update() {
		_PR::Engine = nullptr;
		_PR::World = nullptr;
		_PR::SGS = nullptr;
		_PR::LocalSPC = nullptr;
		_PR::LocalSC = nullptr;
		_PR::LocalSPS = nullptr;

		SDK::UEngine* Engine = SDK::UEngine::GetEngine();
		SDK::UWorld* World = SDK::UWorld::GetWorld();
		SDK::APlayerController* LocalPC = g_Util::GetLocalPC();

		if (Engine) {
			_PR::Engine = Engine;
		}

		if (World) {
			_PR::World = World;
		}

		if (World && World->GameState && World->GameState->IsA(SDK::AShooterGameState::StaticClass())) {
			_PR::SGS = static_cast<SDK::AShooterGameState*>(World->GameState);
		}

		if (LocalPC && LocalPC->IsA(SDK::AShooterPlayerController::StaticClass())) {
			_PR::LocalSPC = static_cast<SDK::AShooterPlayerController*>(LocalPC);
		}

		if (LocalPC && LocalPC->Character && LocalPC->Character->IsA(SDK::AShooterCharacter::StaticClass())) {
			_PR::LocalSC = static_cast<SDK::AShooterCharacter*>(LocalPC->Character);
		}

		if (LocalPC && LocalPC->PlayerState && LocalPC->PlayerState->IsA(SDK::AShooterPlayerState::StaticClass())) {
			_PR::LocalSPS = static_cast<SDK::AShooterPlayerState*>(LocalPC->PlayerState);
		}
	}
}