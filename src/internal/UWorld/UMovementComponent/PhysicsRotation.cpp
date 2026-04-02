#pragma once
#define NOMINMAX  
#include "../../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../../Config/Configs.h"
#include "../../Util/Util.h"
#include "../../Config/ConfigManager.h"
#include "../../Lua/LuaManager.h"
#include "../../Hack/Hack.h"
#include "PhysicsRotation.h"

namespace g_UWorld {
	namespace UMovementComponent {
		void PhysicsRotation(SDK::UMovementComponent* rcx, float DeltaTime) {
			if (g_Config::bForceTurn) {
				g_Hack::ForceTurn(rcx, DeltaTime);
			}

		}
	}
}