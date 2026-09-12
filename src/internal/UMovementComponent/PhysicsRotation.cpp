#pragma once
#include "../../external/CppSDK/SDK.hpp"
#include "../CheatData/CheatCache.hpp"
#include "../Config/Configs.h"
#include "../Hack/Hack.h"
#include "PhysicsRotation.h"

namespace g_UMovementComponent {
	void PhysicsRotation(SDK::UMovementComponent* rcx, float DeltaTime) {
		CheatCache::PhysicsRotation::Update();
		g_Hack::ForceTurn(rcx, DeltaTime);
	}
}