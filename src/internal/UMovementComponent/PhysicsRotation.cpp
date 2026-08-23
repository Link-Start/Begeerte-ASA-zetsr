#pragma once
#include "../../external/CppSDK/SDK.hpp"
#include "../CheatData/DynamicData.hpp"
#include "../Config/Configs.h"
#include "../Hack/Hack.h"
#include "PhysicsRotation.h"

namespace g_UMovementComponent {
	void PhysicsRotation(SDK::UMovementComponent* rcx, float DeltaTime) {
		_PR::Update();
		g_Hack::ForceTurn(rcx, DeltaTime);
	}
}