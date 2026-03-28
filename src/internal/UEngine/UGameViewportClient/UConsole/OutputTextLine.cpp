#pragma once
#define NOMINMAX  
#include "../../../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../../../Config/Configs.h"
#include "../../../Util/Util.h"
#include "../../../Config/ConfigManager.h"
#include "../../../Lua/LuaManager.h"
#include "../../../Hack/Hack.h"
#include "OutputTextLine.h"

namespace g_UEngine {
	namespace UGameViewportClient {
		namespace UConsole {
			void OutputTextLine(SDK::UConsole* Console, SDK::FString* Message) {
				LuaManager::Get().Lua_OnConsoleMessage(Message->ToString());
			}
		}
	}
}