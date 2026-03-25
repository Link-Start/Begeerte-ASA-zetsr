#include "mdx12_api.h"

namespace g_MDX12 {
    void SetSetupUWorldTickCallback(SetupUWorldTickCallback callback) {
        g_Callbacks::g_setupUWorldTickCallback = callback;
    }

    void SetupUWorldTick(SDK::UWorld* rcx) {
        if (g_Callbacks::g_setupUWorldTickCallback) {
            g_Callbacks::g_setupUWorldTickCallback(rcx);
        }
    }

    void SetSetupImGuiCallback(SetupImGuiCallback callback) {
        g_Callbacks::g_setupImGuiCallback = callback;
    }

    void SetupImGui(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
        // Call user-defined callback if set
        if (g_Callbacks::g_setupImGuiCallback) {
            g_Callbacks::g_setupImGuiCallback(pSwapChain, SyncInterval, Flags);
        }
    }
}