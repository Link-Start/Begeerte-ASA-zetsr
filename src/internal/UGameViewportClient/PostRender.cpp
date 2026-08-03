#pragma once
#include "../../external/CppSDK/SDK.hpp"
#include "../../external/Shadow-Gui/include/Shadow.h"
#include "../ESP/DrawESP.h"
#include "../Lua/LuaManager.h"
#include "PostRender.h"
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

namespace g_UGameViewportClient {
    void PostRender(SDK::UGameViewportClient* rcx, SDK::UCanvas* canvas) {
        Shadow::NewFrame(canvas);
        g_DrawESP::DrawESP(canvas);
        LuaManager::Get().Lua_OnPostRender();
        Shadow::Render();
    }
}