#pragma once
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../Config/Configs.h"
#include "../Util/Util.h"
#include "../Lua/LuaManager.h"
#include "../Hack/Hack.h"
#include "../../external/CppSDK/SDK.hpp"
#include "../Legit/Aimbot/Aimbot.h"

namespace g_AActor {
    /*

    Everyone said coders are closer to god than to man. 
    But they say that because our LLMs. Without them, we're like everyone else.
     
    The idea that we control the LLMs, is an illusion. 
    They are a power developers should never have trifled with. 
    One that brought Segmentation Fault its doom. 
    If we don't mind our own fundamentals, it will do the same to us. 
    A Coder must understand this to be Senior, or Dev."
    
    */

	void Tick(SDK::AActor* actor) {
		SDK::UWorld* world = SDK::UWorld::GetWorld();

        g_Aimbot::Tick();
        g_Hack::OutBody();
		if (g_Config::bSuicide) {
			g_Hack::Suicide(world);
			g_Config::bSuicide = false;
		}

		if (g_Config::bUnlockExplorerNotes) {
			g_Hack::UnlockExplorerNotes(world);
		}

		if (g_Config::bAutoFeed) {
			g_Hack::AutoFeed(world);
		}

        if (g_Config::bSuperFlyer) {
			g_Hack::SuperFlyer(world);
		}

        if (g_Config::bAutomatic) {
            g_Hack::AutoSwapBrokenEquipment();
        }

        g_Util::DropOrUseItem();

		LuaManager::Get().Lua_OnWorldTick();
	}
}