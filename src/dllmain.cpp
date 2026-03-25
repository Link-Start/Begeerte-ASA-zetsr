#include "external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "external/SDK/SDK_Headers.hpp"
#include "internal/ESP/ESP.h"
#include "internal/Config/Configs.h"
#include "internal/ESP/DrawESP.h"
#include "internal/Menu/DrawImGui.h"
#include "internal/Lua/LuaManager.h"
#include "internal/Config/ConfigManager.h"
#include "internal/UWorld/Tick.h"

extern "C" {
#include "external/Minimal-D3D12-Hook-ImGui/MinHook/src/buffer.c"
#include "external/Minimal-D3D12-Hook-ImGui/MinHook/src/hook.c"
#include "external/Minimal-D3D12-Hook-ImGui/MinHook/src/trampoline.c"
#include "external/Minimal-D3D12-Hook-ImGui/MinHook/src/hde/hde64.c"
}

void init(LPVOID lpParam) {
    g_MDX12::Initialize(lpParam);
    g_MDX12::SetSetupImGuiCallback(g_DrawImGui::MyImGuiDraw);
    g_MDX12::SetSetupUWorldTickCallback(g_UWorld::Tick);
    // g_Hook::initWorldTick();

    ConfigManager::Get().Initialize("cfg");
    LuaManager::Get().Initialize("lua");
    LuaManager::Get().FetchWorkshopScripts();
}

void MainThread(LPVOID lpParam) {
    init(lpParam);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        if (HANDLE h = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr)) CloseHandle(h);
        break;

    case DLL_PROCESS_DETACH:
        g_MDX12::FinalCleanupAll();
        break;
    }
    return TRUE;
}