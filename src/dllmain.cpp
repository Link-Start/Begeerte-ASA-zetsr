#include "external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "internal/Menu/DrawImGui.h"
#include "internal/AActor/Tick.h"
#include "internal/UNetConnection/HandleDisconnect.h"
#include "internal/UConsole/OutputTextLine.h"
#include "internal/UGameViewportClient/PostRender.h"
#include "internal/UMovementComponent/PhysicsRotation.h"
#include "internal/AActor/TakeDamage.h"
#include "internal/Config/ConfigManager.h"
#include "internal/Language/LanguageManager.h"
#include "internal/Lua/LuaManager.h"

void init(LPVOID lpParam) {
    g_MDX12::Initialize(lpParam);
    g_MDX12::SetSetupImGuiCallback(g_DrawImGui::MyImGuiDraw);
    g_MDX12::SetSetupActorTickCallback(g_AActor::Tick);
    g_MDX12::SetSetupHandleDisconnectCallback(g_UNetConnection::HandleDisconnect);
    g_MDX12::SetSetupOutputTextLineCallback(g_UConsole::OutputTextLine);
    g_MDX12::SetSetupPostRenderCallback(g_UGameViewportClient::PostRender);
    g_MDX12::SetSetupPhysicsRotationCallback(g_UMovementComponent::PhysicsRotation);
    g_MDX12::SetSetupTakeDamageCallback(g_AActor::TakeDamage);

    ConfigManager::Get().Initialize("cfg");
    // LanguageManager::Get().Initialize("language");
    // LanguageManager::Get().FetchWorkshopScripts();
    // LuaManager::Get().Initialize("lua");
    // LuaManager::Get().FetchWorkshopScripts();
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