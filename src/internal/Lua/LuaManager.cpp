#include "LuaManager.h"
#include "../../external/SDK/SDK_Headers.hpp"
#include "../ESP/ESP.h"
#include "../Util/infoCPU.h"
#include "../Util/infoGPU.h"
#include "../Util/Util.h"
#include "../Log/LogManager.h"
#include <algorithm>
#include <iostream>

std::string LuaManager::HttpRequest(const std::string& url) {
    std::string response;
    HINTERNET hInternet = InternetOpenA("Mozilla/5.0 (Windows NT 10.0; Win64; x64)", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID;

        HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, flags, 0);
        if (hConnect) {
            char buffer[4096];
            DWORD bytesRead;
            while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
                response.append(buffer, bytesRead);
            }
            InternetCloseHandle(hConnect);
        }
        else {
            // 调试用：如果这里返回 0，说明 URL 打开失败
            // printf("Error: %lu\n", GetLastError()); 
        }
        InternetCloseHandle(hInternet);
    }
    return response;
}

void LuaManager::FetchWorkshopScripts() {
    std::thread([this]() {
        // --- 修复点 1: 进入无限循环 ---
        while (true) {
            // 请求 GitHub API 获取文件列表 JSON
            std::string json = HttpRequest("https://api.github.com/repos/zetsr/Begeerte-ASA/contents/work_shop");

            // --- 修复点 2: 如果 API 请求失败，等待并重试 ---
            if (json.empty() || json.length() < 10) {
                // 等待 1 秒后重试，避免请求过快
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            std::vector<LuaScript> workshopList;
            size_t entryStart = 0;
            size_t entryEnd = 0;

            // 解析 JSON 逻辑保持不变...
            while ((entryStart = json.find("{", entryStart)) != std::string::npos) {
                entryEnd = json.find("}", entryStart);
                if (entryEnd == std::string::npos) break;

                std::string entryBlock = json.substr(entryStart, entryEnd - entryStart);
                std::string nameKey = "\"name\":\"";
                size_t nPos = entryBlock.find(nameKey);

                if (nPos != std::string::npos) {
                    nPos += nameKey.length();
                    size_t nEnd = entryBlock.find("\"", nPos);
                    std::string fileName = entryBlock.substr(nPos, nEnd - nPos);

                    if (fileName.find(".lua") != std::string::npos) {
                        std::string urlKey = "\"download_url\":\"";
                        size_t dPos = entryBlock.find(urlKey);

                        if (dPos != std::string::npos) {
                            dPos += urlKey.length();
                            size_t dEnd = entryBlock.find("\"", dPos);
                            std::string downloadUrl = entryBlock.substr(dPos, dEnd - dPos);

                            // 执行内存下载
                            std::string content = HttpRequest(downloadUrl);

                            // --- 修复点 3: 只要有一个文件下载失败，就视为本次整体失败，重新循环 ---
                            if (content.empty()) {
                                workshopList.clear();
                                break;
                            }

                            LuaScript script;
                            script.name = "work_shop/" + fileName;
                            script.isWorkshop = true;
                            script.scriptContent = content;
                            script.isLoaded = false;
                            script.hasError = false;
                            workshopList.push_back(std::move(script));
                        }
                    }
                }
                entryStart = entryEnd + 1;
            }

            // --- 修复点 4: 只有当成功获取到脚本且列表不为空时，才更新并退出线程 ---
            if (!workshopList.empty()) {
                std::lock_guard<std::mutex> lock(m_luaMutex);

                m_scripts.erase(
                    std::remove_if(m_scripts.begin(), m_scripts.end(), [](const LuaScript& s) {
                        return s.isWorkshop;
                        }),
                    m_scripts.end()
                            );

                m_scripts.insert(m_scripts.begin(), workshopList.begin(), workshopList.end());

                // 成功下载并更新后，退出线程
                break;
            }

            // 如果走到这里说明解析出的列表是空的，或者部分下载失败了
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }).detach(); // 使用 detach 确保不阻塞主线程
}

void LuaManager::Initialize(const std::string& scriptDir) {
    if (scriptDir.empty()) return;

    try {
        fs::path p = fs::absolute(scriptDir);
        m_scriptDir = p.string();
        if (!fs::exists(m_scriptDir)) fs::create_directories(m_scriptDir);
    }
    catch (const fs::filesystem_error& e) {
        return;
    }

    InitVM();
    RefreshFileList();
}

void LuaManager::BindClient() {
    if (!m_lua) return;

    sol::table client = m_lua->create_named_table("Client");

    client.set_function("AddLog", [](float r, float g, float b, float a, const std::string& text) {
        g_LogManager::AddLog(r, g, b, a, text);
        });
}

void LuaManager::BindImGui() {
    if (!m_lua) return;

    sol::table imgui = m_lua->create_named_table("ImGui");
    auto dl = []() { return ImGui::GetBackgroundDrawList(); };

    imgui.set_function("Color", [](int r, int g, int b, int a) { return IM_COL32(r, g, b, a); });

    imgui.set_function("GetDeltaTime", []() {
        return ImGui::GetIO().DeltaTime;
        });

    imgui.set_function("GetFPS", []() {
        return ImGui::GetIO().Framerate;
        });

    imgui.set_function("IsMouseDown", [](int button) {
        return ImGui::IsMouseDown(button);
        });

    imgui.set_function("IsKeyDown", [](int key) {
        return ImGui::IsKeyDown((ImGuiKey)key);
        });

    imgui.set_function("GetScreenSize", [](sol::this_state s) {
        sol::state_view lua(s);
        ImVec2 size = ImGui::GetIO().DisplaySize;
        sol::table res = lua.create_table();
        res["x"] = size.x;
        res["y"] = size.y;
        return res;
        });

    imgui.set_function("GetMousePos", [](sol::this_state s) {
        sol::state_view lua(s);
        ImVec2 pos = ImGui::GetIO().MousePos;
        sol::table res = lua.create_table();
        res["x"] = pos.x;
        res["y"] = pos.y;
        return res;
        });

    imgui.set_function("CalcTextSize", [](sol::this_state s, const char* text) {
        sol::state_view lua(s);
        ImVec2 size = ImGui::CalcTextSize(text);
        sol::table res = lua.create_table();
        res["x"] = size.x;
        res["y"] = size.y;
        return res;
        });

    imgui.set_function("AddRect", [dl](float x1, float y1, float x2, float y2, ImU32 col, float rounding, float thickness) {
        dl()->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), col, rounding, 0, thickness);
        });

    imgui.set_function("AddRectFilled", [dl](float x1, float y1, float x2, float y2, ImU32 col, float rounding) {
        dl()->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col, rounding);
        });

    imgui.set_function("AddRectFilledMultiColor", [dl](float x1, float y1, float x2, float y2, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left) {
        dl()->AddRectFilledMultiColor(ImVec2(x1, y1), ImVec2(x2, y2), col_upr_left, col_upr_right, col_bot_right, col_bot_left);
        });

    imgui.set_function("AddQuad", [dl](float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, ImU32 col, float thickness) {
        dl()->AddQuad(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), ImVec2(x4, y4), col, thickness);
        });

    imgui.set_function("AddQuadFilled", [dl](float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, ImU32 col) {
        dl()->AddQuadFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), ImVec2(x4, y4), col);
        });

    imgui.set_function("AddTriangle", [dl](float x1, float y1, float x2, float y2, float x3, float y3, ImU32 col, float thickness) {
        dl()->AddTriangle(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), col, thickness);
        });

    imgui.set_function("AddTriangleFilled", [dl](float x1, float y1, float x2, float y2, float x3, float y3, ImU32 col) {
        dl()->AddTriangleFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), col);
        });

    imgui.set_function("AddCircle", [dl](float x, float y, float radius, ImU32 col, int segments, float thickness) {
        dl()->AddCircle(ImVec2(x, y), radius, col, segments, thickness);
        });

    imgui.set_function("AddCircleFilled", [dl](float x, float y, float radius, ImU32 col, int segments) {
        dl()->AddCircleFilled(ImVec2(x, y), radius, col, segments);
        });

    imgui.set_function("AddNgon", [dl](float x, float y, float radius, ImU32 col, int segments, float thickness) {
        dl()->AddNgon(ImVec2(x, y), radius, col, segments, thickness);
        });

    imgui.set_function("AddNgonFilled", [dl](float x, float y, float radius, ImU32 col, int segments) {
        dl()->AddNgonFilled(ImVec2(x, y), radius, col, segments);
        });

    imgui.set_function("AddEllipse", [dl](float x, float y, float radius_x, float radius_y, ImU32 col, float rot, int segments, float thickness) {
        dl()->AddEllipse(ImVec2(x, y), ImVec2(radius_x, radius_y), col, rot, segments, thickness);
        });

    imgui.set_function("AddEllipseFilled", [dl](float x, float y, float radius_x, float radius_y, ImU32 col, float rot, int segments) {
        dl()->AddEllipseFilled(ImVec2(x, y), ImVec2(radius_x, radius_y), col, rot, segments);
        });

    imgui.set_function("AddBezierQuadratic", [dl](float x1, float y1, float x2, float y2, float x3, float y3, ImU32 col, float thickness, int segments) {
        dl()->AddBezierQuadratic(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), col, thickness, segments);
        });

    imgui.set_function("AddBezierCubic", [dl](float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, ImU32 col, float thickness, int segments) {
        dl()->AddBezierCubic(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), ImVec2(x4, y4), col, thickness, segments);
        });

    imgui.set_function("AddLine", [dl](float x1, float y1, float x2, float y2, ImU32 col, float thickness) {
        dl()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), col, thickness);
        });

    imgui.set_function("AddText", [dl](float x, float y, ImU32 col, const char* text) {
        dl()->AddText(ImVec2(x, y), col, text);
        });
}

void LuaManager::BindSDK() {
    if (!m_lua) return;

    // --- 1. 基础结构 ---
    m_lua->new_usertype<SDK::FVector>("FVector",
        sol::constructors<SDK::FVector(), SDK::FVector(float, float, float)>(),
        "X", &SDK::FVector::X, "Y", &SDK::FVector::Y, "Z", &SDK::FVector::Z
        );

    // --- 注册 UNetConnection 类 ---
    m_lua->new_usertype<SDK::UNetConnection>("UNetConnection",
        // 绑定我们封装的成员函数
        "GetFirstIP", &SDK::UNetConnection::GetFirstIP,
        "GetPort", &SDK::UNetConnection::GetPort
        );

    // --- 注册 UNetDriver 类 ---
    m_lua->new_usertype<SDK::UNetDriver>("UNetDriver",
        // 绑定 ServerConnection 指针
        "ServerConnection", &SDK::UNetDriver::ServerConnection
        );

    sol::table sdk = m_lua->create_named_table("SDK");
    sdk.set_function("GetNetDriver", []() -> sol::optional<SDK::UNetDriver*> {
        SDK::UWorld* W = SDK::UWorld::GetWorld();
        if (W && W->NetDriver) {
            return W->NetDriver;
        }
        return sol::nullopt;
        });

    sdk.set_function("GetLocalPC", []() {
        SDK::APlayerController* pc = g_Util::GetLocalPC();
        return (pc && pc->Pawn) ? (uintptr_t)pc : 0;
        });

    sdk.set_function("GetActors", [this](sol::this_state s) {
        sol::state_view lua(s);
        sol::table res = lua.create_table();
        SDK::UWorld* W = SDK::UWorld::GetWorld();

        if (W && W->PersistentLevel) {
            int idx = 1;
            SDK::TArray<SDK::AActor*>& actors = W->PersistentLevel->Actors;
            for (int i = 0; i < actors.Num(); i++) {
                SDK::AActor* a = actors[i];
                if (a) {
                    res[idx++] = (uintptr_t)a;
                }
            }
        }
        return res;
        });

    sdk.set_function("GetCharacterClass", []() { return (uintptr_t)SDK::APrimalCharacter::StaticClass(); });
    sdk.set_function("GetDinoClass", []() { return (uintptr_t)SDK::APrimalDinoCharacter::StaticClass(); });
    sdk.set_function("GetDroppedItemClass", []() { return (uintptr_t)SDK::ADroppedItem::StaticClass(); });
    sdk.set_function("GetContainerClass", []() { return (uintptr_t)SDK::APrimalStructureItemContainer::StaticClass(); });
    sdk.set_function("GetTurretClass", []() { return (uintptr_t)SDK::APrimalStructureTurret::StaticClass(); });

    sol::table actor_api = m_lua->create_named_table("Actor");
    actor_api.set_function("IsA", [](uintptr_t a, uintptr_t cls) { return (a && cls) ? ((SDK::AActor*)a)->IsA((SDK::UClass*)cls) : false; });
    actor_api.set_function("GetLocation", [](uintptr_t a) -> sol::optional<SDK::FVector> { return (a) ? sol::make_optional(((SDK::AActor*)a)->K2_GetActorLocation()) : sol::nullopt; });
    actor_api.set_function("GetDistance", [](uintptr_t a, uintptr_t b) { return (a && b) ? ((SDK::AActor*)a)->GetDistanceTo((SDK::AActor*)b) : 0.0f; });
    actor_api.set_function("IsHidden", [](uintptr_t a) { return a ? (bool)((SDK::AActor*)a)->bHidden : true; });
    actor_api.set_function("GetClassName", [](uintptr_t a) -> std::string {
        SDK::AActor* act = (SDK::AActor*)a;
        return (act && act->Class) ? act->Class->GetName() : "";
        });

    sol::table char_api = m_lua->create_named_table("Character");
    char_api.set_function("GetInfo", [](uintptr_t a) {
        SDK::APrimalCharacter* c = (SDK::APrimalCharacter*)a;
        if (!c) return std::make_tuple(0.0f, 0.0f, false, std::string("Unknown"));

        std::string name = c->GetDescriptiveName().ToString();

        if (c->PlayerState) name = c->PlayerState->GetPlayerName().ToString();

        return std::make_tuple(c->GetHealth(), c->GetMaxHealth(), (bool)c->IsDead(), name);
        });

    char_api.set_function("GetExactPing", [](uintptr_t a) {
        float ping = 0.0f;

        SDK::APrimalCharacter* c = (SDK::APrimalCharacter*)a;
        if (!c) return ping;
        if (c->PlayerState) ping = c->PlayerState->GetExactPing();

        return ping;
        });

    char_api.set_function("GetRelation", [](uintptr_t target, uintptr_t local) -> int {
        if (!target || !local) return 0;
        return (int)g_ESP::GetRelation((SDK::APrimalCharacter*)target, (SDK::APrimalCharacter*)local);
        });

    sol::table item_api = m_lua->create_named_table("Item");
    item_api.set_function("GetDroppedInfo", [](uintptr_t a) {
        SDK::ADroppedItem* dropped = (SDK::ADroppedItem*)a;
        if (!dropped || !dropped->MyItem) return std::make_tuple(false, std::string(""), 0, 0.0f, false, std::string(""));

        SDK::UPrimalItem* it = dropped->MyItem;
        std::string name = it->DescriptiveNameBase.ToString();
        if (it->CustomItemName.IsValid() && !it->CustomItemName.ToString().empty()) name = it->CustomItemName.ToString();

        std::string className = it->Class ? it->Class->GetName() : "";
        return std::make_tuple(true, name, it->ItemQuantity, it->ItemRating, (bool)it->bIsBlueprint, className);
        });

    sol::table cont_api = m_lua->create_named_table("Container");
    cont_api.set_function("GetInfo", [](uintptr_t a) {
        SDK::APrimalStructureItemContainer* c = (SDK::APrimalStructureItemContainer*)a;
        std::string name = c ? c->GetDescriptiveName().ToString() : "";
        if (name.empty() || name == "None") name = "Supply Crate";
        return name;
        });

    sol::table pc_api = m_lua->create_named_table("PC");
    pc_api.set_function("GetPawn", [](uintptr_t pc) -> uintptr_t { return pc ? (uintptr_t)((SDK::APlayerController*)pc)->Pawn : 0; });
    pc_api.set_function("ProjectToScreen", [](uintptr_t pc, SDK::FVector worldLoc) -> std::tuple<bool, float, float> {
        SDK::FVector2D screenPos;
        bool ok = pc ? ((SDK::APlayerController*)pc)->ProjectWorldLocationToScreen(worldLoc, &screenPos, false) : false;
        return std::make_tuple(ok, (float)screenPos.X, (float)screenPos.Y);
        });
}

void LuaManager::BindSystem() {
    if (!m_lua) return;
    sol::table system_api = m_lua->create_named_table("System");

    system_api.set_function("GetCPUStats", []() -> std::tuple<double, double> {
        g_infoCPU::CPUStats stats;
        if (g_infoCPU::GetCPUInformation(stats)) {
            // 返回元组：第一个是使用率，第二个是频率
            return std::make_tuple(stats.usagePercentage, stats.frequencyMHz);
        }
        return std::make_tuple(0.0, 0.0);
        });

    system_api.set_function("GetGPUStats", []() -> std::tuple<double, double> {
        g_infoGPU::GPUStats stats;
        if (g_infoGPU::GetGPUInformation(stats)) {
            // 返回元组：第一个是使用率
            return std::make_tuple(stats.usagePercentage, 0.0);
        }
        return std::make_tuple(0.0, 0.0);
        });
}

void LuaManager::InitVM() {
    for (auto& script : m_scripts) {
        script.env = sol::nil;
    }

    m_lua.reset(new sol::state());
    m_lua->open_libraries();

    BindClient();
    BindImGui();
    BindSDK();
    BindSystem();
}

void LuaManager::Shutdown() {
    std::lock_guard<std::mutex> lock(m_luaMutex);
    for (auto& s : m_scripts) s.env = sol::nil;
    m_scripts.clear();
    m_lua.reset();
}

bool LuaManager::SetScriptState(int index, bool load) {
    if (index < 0 || index >= (int)m_scripts.size()) return false;
    auto& script = m_scripts[index];

    if (load) {
        if (ExecuteScript(script)) {
            script.isLoaded = true;
            return true;
        }
        else {
            script.isLoaded = false;
            return false;
        }
    }
    else {
        script.isLoaded = false;
        script.env = sol::nil;
        script.hasError = false;
        return true;
    }
}

void LuaManager::Update() {
    std::lock_guard<std::mutex> lock(m_luaMutex);

    if (m_pendingReset) {
        InitVM();
        for (auto& script : m_scripts) {
            if (script.isLoaded) ExecuteScript(script);
        }
        m_pendingReset = false;
        m_needsReload = false;
        return;
    }

    if (m_needsReload) {
        ActualReloadAll();
        m_needsReload = false;
    }
}

void LuaManager::ActualReloadAll() {
    if (!m_lua) return;

    m_lua->collect_garbage();

    for (auto& script : m_scripts) {
        if (script.isLoaded) {
            ExecuteScript(script);
        }
    }
}

void LuaManager::ReloadAll() {
    m_pendingReset = true;
}

bool LuaManager::ExecuteScript(LuaScript& script) {
    if (!m_lua) return false;

    script.hasError = false;
    script.lastError.clear();

    sol::table env_table = m_lua->create_table();
    sol::table env_mt = m_lua->create_table();
    env_mt["__index"] = m_lua->globals();
    env_table[sol::metatable_key] = env_mt;
    script.env = sol::environment(*m_lua, env_table);

    sol::load_result load_result;
    if (script.isWorkshop) {
        load_result = m_lua->load(script.scriptContent, script.name);
    }
    else {
        load_result = m_lua->load_file(script.path.string());
    }

    if (!load_result.valid()) {
        sol::error err = load_result;
        script.hasError = true;
        script.lastError = "Load Error: " + std::string(err.what());

        std::string errorMsg = "[!] " + script.name + ": " + std::string(err.what());
        g_LogManager::AddLog(255.f, 50.f, 50.f, 255.f, errorMsg);
        return false;
    }

    sol::protected_function script_func = load_result;
    script.env.set_on(script_func);

    auto exec_result = script_func();
    if (!exec_result.valid()) {
        sol::error err = exec_result;
        script.hasError = true;
        script.lastError = "Exec Error: " + std::string(err.what());

        std::string errorMsg = "[!] " + script.name + ": " + std::string(err.what());
        g_LogManager::AddLog(255.f, 50.f, 50.f, 255.f, errorMsg);
        return false;
    }

    return true;
}

void LuaManager::RefreshFileList() {
    std::lock_guard<std::mutex> lock(m_luaMutex);
    std::vector<LuaScript> nextList;
    for (auto& s : m_scripts) {
        if (s.isWorkshop) {
            nextList.push_back(std::move(s));
        }
    }

    if (!m_scriptDir.empty() && fs::exists(m_scriptDir)) {
        for (const auto& entry : fs::directory_iterator(m_scriptDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                fs::path fullPath = fs::absolute(entry.path());

                auto it = std::find_if(m_scripts.begin(), m_scripts.end(), [&](const LuaScript& old) {
                    return !old.isWorkshop && fs::absolute(old.path) == fullPath;
                    });

                if (it != m_scripts.end()) {
                    nextList.push_back(std::move(*it));
                }
                else {
                    LuaScript s;
                    s.name = entry.path().filename().string();
                    s.path = fullPath;
                    s.isWorkshop = false;
                    s.isLoaded = false;
                    s.hasError = false;
                    nextList.push_back(std::move(s));
                }
            }
        }
    }

    m_scripts = std::move(nextList);
}

void LuaManager::Lua_OnConsoleMessage(std::string Message) {
    std::lock_guard<std::mutex> lock(m_luaMutex);
    if (!m_lua || !m_lua->lua_state()) return;

    for (auto& script : m_scripts) {
        if (!script.isLoaded || script.hasError || !script.env.valid()) continue;

        sol::object ConsoleMessageObj = script.env["OnConsoleMessage"];
        if (ConsoleMessageObj.is<sol::protected_function>()) {
            sol::protected_function ConsoleMessageFunc = ConsoleMessageObj;
            auto result = ConsoleMessageFunc(Message);
            if (!result.valid()) {
                sol::error err = result;
                script.hasError = true;
                script.lastError = "Runtime Error: " + std::string(err.what());

                std::string errorMsg = "[!] " + script.name + ": " + std::string(err.what());
                g_LogManager::AddLog(255.f, 50.f, 50.f, 255.f, errorMsg);
            }
        }
    }
}

void LuaManager::Lua_OnDisconnect(std::string ServerIP, int32_t ServerPort) {
    std::lock_guard<std::mutex> lock(m_luaMutex);
    if (!m_lua || !m_lua->lua_state()) return;

    for (auto& script : m_scripts) {
        if (!script.isLoaded || script.hasError || !script.env.valid()) continue;

        sol::object DisconnectObj = script.env["OnDisconnect"];
        if (DisconnectObj.is<sol::protected_function>()) {
            sol::protected_function DisconnectFunc = DisconnectObj;
            auto result = DisconnectFunc(ServerIP, ServerPort);
            if (!result.valid()) {
                sol::error err = result;
                script.hasError = true;
                script.lastError = "Runtime Error: " + std::string(err.what());

                std::string errorMsg = "[!] " + script.name + ": " + std::string(err.what());
                g_LogManager::AddLog(255.f, 50.f, 50.f, 255.f, errorMsg);
            }
        }
    }
}

void LuaManager::Lua_OnWorldTick() {
    std::lock_guard<std::mutex> lock(m_luaMutex);
    if (!m_lua || !m_lua->lua_state()) return;

    for (auto& script : m_scripts) {
        if (!script.isLoaded || script.hasError || !script.env.valid()) continue;

        sol::object WorldTickObj = script.env["OnWorldTick"];
        if (WorldTickObj.is<sol::protected_function>()) {
            sol::protected_function WorldTickFunc = WorldTickObj;
            auto result = WorldTickFunc();
            if (!result.valid()) {
                sol::error err = result;
                script.hasError = true;
                script.lastError = "Runtime Error: " + std::string(err.what());

                std::string errorMsg = "[!] " + script.name + ": " + std::string(err.what());
                g_LogManager::AddLog(255.f, 50.f, 50.f, 255.f, errorMsg);
            }
        }
    }
}

void LuaManager::Lua_OnPaintMenu(float MenuAlpha) {
    std::lock_guard<std::mutex> lock(m_luaMutex);
    if (!m_lua || !m_lua->lua_state()) return;

    for (auto& script : m_scripts) {
        if (!script.isLoaded || script.hasError || !script.env.valid()) continue;

        sol::object drawMenuObj = script.env["OnPaintMenu"];
        if (drawMenuObj.is<sol::protected_function>()) {
            sol::protected_function drawMenuFunc = drawMenuObj;
            auto result = drawMenuFunc(MenuAlpha);
            if (!result.valid()) {
                sol::error err = result;
                script.hasError = true;
                script.lastError = "Runtime Error: " + std::string(err.what());

                std::string errorMsg = "[!] " + script.name + ": " + std::string(err.what());
                g_LogManager::AddLog(255.f, 50.f, 50.f, 255.f, errorMsg);
            }
        }
    }
}

void LuaManager::Lua_OnPaint() {
    std::lock_guard<std::mutex> lock(m_luaMutex);
    if (!m_lua || !m_lua->lua_state()) return;

    for (auto& script : m_scripts) {
        if (!script.isLoaded || script.hasError || !script.env.valid()) continue;

        sol::object drawObj = script.env["OnPaint"];
        if (drawObj.is<sol::protected_function>()) {
            sol::protected_function drawFunc = drawObj;
            auto result = drawFunc();
            if (!result.valid()) {
                sol::error err = result;
                script.hasError = true;
                script.lastError = "Runtime Error: " + std::string(err.what());

                std::string errorMsg = "[!] " + script.name + ": " + std::string(err.what());
                g_LogManager::AddLog(255.f, 50.f, 50.f, 255.f, errorMsg);
            }
        }
    }
}

void LuaManager::Lua_OnMenuOpen() {
    std::lock_guard<std::mutex> lock(m_luaMutex);
    if (!m_lua || !m_lua->lua_state()) return;

    for (auto& script : m_scripts) {
        if (!script.isLoaded || script.hasError || !script.env.valid()) continue;

        sol::object MenuOpenObj = script.env["OnMenuOpen"];
        if (MenuOpenObj.is<sol::protected_function>()) {
            sol::protected_function MenuOpenFunc = MenuOpenObj;
            auto result = MenuOpenFunc();
            if (!result.valid()) {
                sol::error err = result;
                script.hasError = true;
                script.lastError = "Runtime Error: " + std::string(err.what());

                std::string errorMsg = "[!] " + script.name + ": " + std::string(err.what());
                g_LogManager::AddLog(255.f, 50.f, 50.f, 255.f, errorMsg);
            }
        }
    }
}