#pragma once
#define NOMINMAX  
#include "../../../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../../../Config/Configs.h"
#include "../../../Util/Util.h"
#include "../../../Config/ConfigManager.h"
#include "../../../Lua/LuaManager.h"
#include "../../../Hack/Hack.h"
#include "HandleDisconnect.h"

namespace g_UWorld {
	namespace UNetDriver {
		namespace UNetConnection {
			void HandleDisconnect(SDK::UNetConnection* UNetConnection) {
				LuaManager::Get().Lua_OnDisconnect(UNetConnection ? UNetConnection->GetFirstIP() : "?", UNetConnection ? UNetConnection->GetPort() : 0);
			}
		}
	}
}