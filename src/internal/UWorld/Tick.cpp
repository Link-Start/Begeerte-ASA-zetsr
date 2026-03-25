#pragma once
#define NOMINMAX  
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../Config/Configs.h"
#include "../Util/Util.h"
#include "../Config/ConfigManager.h"
#include "../Lua/LuaManager.h"
#include "../Hack/Hack.h"
#include "../../external/SDK/SDK_Headers.hpp"

namespace g_UWorld {
	void Tick(SDK::UWorld* world) {
		if (g_Config::bUnlockExplorerNotes) {
			g_Hack::UnlockExplorerNotes(world);
		}

		if (g_Config::bAutoFeed) {
			g_Hack::AutoFeed(world);
		}

		LuaManager::Get().Lua_OnWorldTick();
	}
}