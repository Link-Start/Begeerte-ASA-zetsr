#pragma once
#include "../../external/CppSDK/SDK.hpp"
#include "../Util/Util.h"

namespace _DD {
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