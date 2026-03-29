#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <memory>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"

#define SOL_ALL_SAFETIES_ON 1
#include "../../external/sol2/sol.hpp"

namespace fs = std::filesystem;

struct LuaScript {
    std::string name;
    fs::path path;
    bool isLoaded = false;
    bool hasError = false;
    bool isWorkshop = false;
    bool isLibrary = false;

    std::string scriptContent;
    std::string lastError;

    sol::environment env;
};

class LuaManager {
public:
    static LuaManager& Get() {
        static LuaManager* instance = nullptr;
        if (!instance) {
            instance = new LuaManager();
        }
        return *instance;
    }

    void SetEnabled(bool enabled);
    bool IsEnabled() const { return m_enabled; }

    void Initialize(const std::string& scriptDir);
    void Shutdown();
    void ReloadAll();
    void RefreshFileList();
    std::vector<LuaScript>& GetScripts() { return m_scripts; }
    bool SetScriptState(int index, bool load);

    lua_State* GetState() { return m_lua ? m_lua->lua_state() : nullptr; }
    const std::string& GetScriptDir() const { return m_scriptDir; }
    
    void Lua_OnShutDown(LuaScript& script);
    void Lua_OnConsoleMessage(std::string Message);
    void Lua_OnDisconnect(std::string ServerIP, int32_t ServerPort);
    void Lua_OnWorldTick();
    void Lua_OnPaintMenu(float MenuAlpha);
    void Lua_OnPaint();
    void Lua_OnMenuOpen();

    void Update();
    void ActualReloadAll();

    void FetchWorkshopScripts();

private:
    static bool CheckNoLuaFlag() {
        int argc;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!argv) return false;

        bool found = false;
        for (int i = 0; i < argc; i++) {
            // _wcsicmp 返回 0 表示字符串相等（忽略大小写）
            if (_wcsicmp(argv[i], L"-nolua") == 0 || _wcsicmp(argv[i], L"nolua") == 0) {
                found = true;
                break;
            }
        }
        LocalFree(argv);
        return found; // 找到返回 true
    }

    bool m_enabled = !CheckNoLuaFlag();
    bool m_needsReload = false;
    std::mutex m_luaMutex;
    std::atomic<bool> m_pendingReset{ false };

    LuaManager() = default;
    ~LuaManager() { Shutdown(); }

    std::unique_ptr<sol::state> m_lua;
    std::string m_scriptDir;
    std::vector<LuaScript> m_scripts;

    void InitVM();
    void Bind_G();
    void BindClient();
    void BindImGui();
    void BindSDK();
    void BindSystem();
    bool ExecuteScript(LuaScript& script);

    std::string HttpRequest(const std::string& url);
};