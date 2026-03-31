#pragma once
#define NOMINMAX
#include "../../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../../../external/SDK/SDK_Headers.hpp"
#include "../../Config/Configs.h"
#include "../../Util/Util.h"
#include "../../Config/ConfigManager.h"
#include "../../Lua/LuaManager.h"
#include "../../Hack/Hack.h"
#include "PostRender.h"
#include "../../ESP/DrawESP.h"
#include "../../ESP/ESP.h"
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

namespace g_UEngine {
    namespace UGameViewportClient {
        void PostRender(SDK::UGameViewportClient* rcx, SDK::UCanvas* canvas){

        }
    }
}