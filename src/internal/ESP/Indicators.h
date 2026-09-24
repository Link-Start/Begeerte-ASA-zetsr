// Indicators.h
#pragma once

#include <string>
#include "../../external/CppSDK/SDK.hpp"
#include "../../external/Shadow-Gui/src/Shadow.h"
#include "../Util/Util.h"
#include "../Config/Configs.h"
#include "../../XorStr.h"

namespace g_Indicators {
    // 起始点
    constexpr float kStartX = 5.0f;
    constexpr float kStartY = 5.0f;

    // 行间距
    constexpr float kPaddingY = 0.0f;

    // 当前绘制的 Y 坐标偏移量
    inline float g_CurrentY = kStartY;

    // 在每帧绘制开始前调用，重置绘制位置到起始点 (5.0f, 5.0f)
    __forceinline void Reset() {
        g_CurrentY = kStartY;
    }

    __forceinline void AddTop(Shadow::Color color, const std::string& text) {
        if (text.empty()) return;
        static SDK::UFont* OpenSansRegular12 = g_Util::GetOpenSansRegular12();
        if (!OpenSansRegular12) return;

        Shadow::PushFont(OpenSansRegular12);
        Shadow::PushTextOutline();
        Shadow::GetBackgroundDrawList()->AddText({ kStartX, g_CurrentY }, color, text);
        float textHeight = Shadow::MeasureTextSize(text).y;
        Shadow::PopTextOutline();
        Shadow::PopFont();

        g_CurrentY += textHeight + kPaddingY;
    }

    inline void DrawNetGraph()
    {
        static SDK::UFont* OpenSansRegular12 = g_Util::GetOpenSansRegular12();
        if (!OpenSansRegular12) return;

        Shadow::PushFont(OpenSansRegular12);
        Shadow::PushTextOutline();

        // -------------------------------------------------------------
        // 1. 本地状态管理（完全封装在内部，无全局变量）
        // -------------------------------------------------------------
        constexpr size_t CLIENT_SAMPLES = 120; // 客户端帧统计窗口 (约1~2秒)
        constexpr size_t SERVER_SAMPLES = 32;  // 服务端 Tick 统计窗口
        constexpr size_t PING_SAMPLES = 40;  // Ping 抖动统计窗口

        struct SlotLayout {
            bool initialized = false;
            float slot_fps_w = 0.0f;
            float slot_cvar_w = 0.0f;
            float slot_ping_w = 0.0f;
            float slot_sv_w = 0.0f;
            float slot_svar_w = 0.0f;
            float ms_w = 0.0f;
            float pm_w = 0.0f;
            float space_w = 0.0f;
            float col_spacing = 0.0f;
            float line_spacing = 0.0f;
        };

        struct NetGraphState {
            SlotLayout layout{};

            // 客户端 DeltaTime (秒)
            std::array<float, CLIENT_SAMPLES> client_dt_history{};
            size_t client_idx = 0;
            size_t client_count = 0;

            // 服务端 DeltaTime (毫秒)
            std::array<float, SERVER_SAMPLES> server_dt_ms_history{};
            size_t server_idx = 0;
            size_t server_count = 0;

            // Ping 历史记录
            std::array<float, PING_SAMPLES> ping_history{};
            size_t ping_idx = 0;
            size_t ping_count = 0;

            float smooth_client_var = 0.0f;
            float prev_server_var = 0.0f;
            float smooth_ping_jitter = 0.0f;
            float peak_server_fps = 0.0f;
        };
        static NetGraphState state;

        // -------------------------------------------------------------
        // 2. 测量并锁定各列槽位及公共单位/符号尺寸
        // -------------------------------------------------------------
        if (!state.layout.initialized) {
            auto& L = state.layout;
            L.ms_w = Shadow::MeasureTextSize("ms").x;
            L.pm_w = Shadow::MeasureTextSize("+-").x;
            L.space_w = Shadow::MeasureTextSize(" ").x;

            L.slot_fps_w = Shadow::MeasureTextSize("fps: 999").x;
            L.slot_cvar_w = Shadow::MeasureTextSize("var: 99.9 ms").x;
            L.slot_ping_w = Shadow::MeasureTextSize("ping: 999 ms").x;
            L.slot_sv_w = Shadow::MeasureTextSize("sv: 99.9 +- 99.9 ms").x;
            L.slot_svar_w = Shadow::MeasureTextSize("var: 99.999 ms").x;

            const float line_h = Shadow::MeasureTextSize("A").y;
            L.col_spacing = Shadow::MeasureTextSize("      ").x; // 6个空格
            L.line_spacing = line_h * 1.18f;
            L.initialized = true;
        }

        const auto& L = state.layout;

        // -------------------------------------------------------------
        // 3. 客户端数据采集与计算 (FPS & Client Var)
        // -------------------------------------------------------------
        const float raw_client_dt = Shadow::GetIO().DeltaTime;
        const float client_dt = std::max(0.0001f, raw_client_dt);

        state.client_dt_history[state.client_idx] = client_dt;
        state.client_idx = (state.client_idx + 1) % CLIENT_SAMPLES;
        if (state.client_count < CLIENT_SAMPLES) ++state.client_count;

        float client_sum = 0.0f;
        for (size_t i = 0; i < state.client_count; ++i) {
            client_sum += state.client_dt_history[i];
        }
        const float client_avg_dt = client_sum / static_cast<float>(state.client_count);
        const float client_fps = (client_avg_dt > 0.0f) ? (1.0f / client_avg_dt) : 0.0f;

        float client_var_sq_sum = 0.0f;
        for (size_t i = 0; i < state.client_count; ++i) {
            const float diff = (state.client_dt_history[i] - client_avg_dt) * 1000.0f;
            client_var_sq_sum += diff * diff;
        }
        const float instant_client_var = std::sqrt(client_var_sq_sum / static_cast<float>(state.client_count));
        state.smooth_client_var = std::lerp(state.smooth_client_var, instant_client_var, 0.05f);

        // -------------------------------------------------------------
        // 4. 服务端状态与 Ping 获取（含离线条件判断）
        // -------------------------------------------------------------
        const float raw_server_fps = g_Util::GetServerFPS();
        const float current_ping = g_Util::GetClientPing();

        const bool is_connected_server = !(current_ping <= 0.0f && raw_server_fps <= 0.0f);

        float server_avg_sv_ms = 0.0f;
        float server_stddev_ms = 0.0f;
        float server_var_ms = 0.0f;
        float server_var_delta = 0.0f;
        float dynamic_nominal_dt_ms = 33.33f;
        float server_avg_dt_ms = 0.0f;

        if (is_connected_server) {
            const float valid_server_fps = std::max(0.1f, raw_server_fps);
            const float current_server_dt_ms = (1.0f / valid_server_fps) * 1000.0f;

            state.peak_server_fps = std::max(state.peak_server_fps, valid_server_fps);
            state.peak_server_fps = std::lerp(state.peak_server_fps, valid_server_fps, 0.001f);
            dynamic_nominal_dt_ms = (1.0f / std::max(1.0f, state.peak_server_fps)) * 1000.0f;

            state.server_dt_ms_history[state.server_idx] = current_server_dt_ms;
            state.server_idx = (state.server_idx + 1) % SERVER_SAMPLES;
            if (state.server_count < SERVER_SAMPLES) ++state.server_count;

            float server_dt_sum = 0.0f;
            for (size_t i = 0; i < state.server_count; ++i) {
                server_dt_sum += state.server_dt_ms_history[i];
            }
            server_avg_dt_ms = server_dt_sum / static_cast<float>(state.server_count);

            float server_dt_diff_sq_sum = 0.0f;
            for (size_t i = 0; i < state.server_count; ++i) {
                const float diff = state.server_dt_ms_history[i] - server_avg_dt_ms;
                server_dt_diff_sq_sum += diff * diff;
            }
            server_stddev_ms = (state.server_count > 1)
                ? std::sqrt(server_dt_diff_sq_sum / static_cast<float>(state.server_count - 1))
                : 0.0f;

            const float frame_overrun = std::max(0.0f, server_avg_dt_ms - dynamic_nominal_dt_ms);
            server_avg_sv_ms = std::max(0.1f, frame_overrun + (server_stddev_ms * 1.5f));

            float server_var_diff_sq_sum = 0.0f;
            for (size_t i = 0; i < state.server_count; ++i) {
                const float jitter_err = std::abs(state.server_dt_ms_history[i] - dynamic_nominal_dt_ms);
                server_var_diff_sq_sum += jitter_err * jitter_err;
            }
            server_var_ms = (state.server_count > 1)
                ? std::sqrt(server_var_diff_sq_sum / static_cast<float>(state.server_count - 1))
                : 0.0f;

            server_var_delta = std::abs(server_var_ms - state.prev_server_var);
            state.prev_server_var = server_var_ms;

            // Ping 抖动分析
            const float safe_ping = std::max(0.0f, current_ping);
            state.ping_history[state.ping_idx] = safe_ping;
            state.ping_idx = (state.ping_idx + 1) % PING_SAMPLES;
            if (state.ping_count < PING_SAMPLES) ++state.ping_count;

            float ping_sum = 0.0f;
            for (size_t i = 0; i < state.ping_count; ++i) {
                ping_sum += state.ping_history[i];
            }
            const float ping_avg = ping_sum / static_cast<float>(state.ping_count);

            float ping_sq_diff_sum = 0.0f;
            for (size_t i = 0; i < state.ping_count; ++i) {
                const float diff = state.ping_history[i] - ping_avg;
                ping_sq_diff_sum += diff * diff;
            }
            const float instant_ping_jitter = (state.ping_count > 1)
                ? std::sqrt(ping_sq_diff_sum / static_cast<float>(state.ping_count - 1))
                : 0.0f;

            state.smooth_ping_jitter = std::lerp(state.smooth_ping_jitter, instant_ping_jitter, 0.15f);
        }

        // -------------------------------------------------------------
        // 5. 格式化纯数值文本
        // -------------------------------------------------------------
        const std::string val_fps = std::format("{:.0f}", client_fps);
        const std::string val_cvar = std::format("{:.1f}", state.smooth_client_var);
        const std::string val_ping = is_connected_server ? std::format("{:.0f}", std::max(0.0f, current_ping)) : "";

        const std::string val_sv_main = is_connected_server ? std::format("{:.1f}", server_avg_sv_ms) : "";
        const std::string val_sv_dev = is_connected_server ? std::format("{:.1f}", server_stddev_ms) : "";
        const std::string val_svar = is_connected_server ? std::format("{:.3f}", server_var_ms) : "";

        // -------------------------------------------------------------
        // 6. 坐标基准与渲染绘制（顶部与右边缘保持相同的边距）
        // -------------------------------------------------------------
        const float screen_w = Shadow::GetIO().DisplaySize.x;
        constexpr float margin = 10.0f; // 顶部与右边缘统一边距

        const float right_edge = screen_w - margin;
        const float line1_y = margin;
        const float line2_y = margin + L.line_spacing;

        auto* draw_list = Shadow::GetBackgroundDrawList();

        struct Color4f { float r, g, b, a; };
        constexpr Color4f col_yellow{ 1.f, 1.f, 0.65f, 1.f }; // 常规黄
        constexpr Color4f col_red{ 1.f, 0.1f, 0.1f, 1.f };  // 警示红

        auto GetQuantizedNumWidth = [](const std::string& num_text) -> float {
            if (num_text.empty()) return 0.0f;
            std::string template_str = num_text;
            for (char& c : template_str) {
                if (c >= '0' && c <= '9') {
                    c = '9';
                }
            }
            return Shadow::MeasureTextSize(template_str).x;
            };

        auto DrawUnitSlot = [&](float slot_x, float slot_w, float y, const char* label, const std::string& val_text, Color4f color, bool has_ms = true) {
            if (val_text.empty()) return;

            draw_list->AddText({ slot_x, y }, { color.r, color.g, color.b, color.a }, label);

            if (has_ms) {
                const float ms_x = (slot_x + slot_w) - L.ms_w;
                draw_list->AddText({ ms_x, y }, { color.r, color.g, color.b, color.a }, "ms");

                const float q_val_w = GetQuantizedNumWidth(val_text);
                const float val_x = ms_x - L.space_w - q_val_w;
                draw_list->AddText({ val_x, y }, { color.r, color.g, color.b, color.a }, val_text);
            }
            else {
                const float q_val_w = GetQuantizedNumWidth(val_text);
                const float val_x = (slot_x + slot_w) - q_val_w;
                draw_list->AddText({ val_x, y }, { color.r, color.g, color.b, color.a }, val_text);
            }
            };

        auto DrawSvSlot = [&](float slot_x, float slot_w, float y, const std::string& v_main, const std::string& v_dev, Color4f color) {
            if (v_main.empty() || v_dev.empty()) return;

            draw_list->AddText({ slot_x, y }, { color.r, color.g, color.b, color.a }, "sv:");

            const float ms_x = (slot_x + slot_w) - L.ms_w;
            draw_list->AddText({ ms_x, y }, { color.r, color.g, color.b, color.a }, "ms");

            const float q_dev_w = GetQuantizedNumWidth(v_dev);
            const float dev_x = ms_x - L.space_w - q_dev_w;
            draw_list->AddText({ dev_x, y }, { color.r, color.g, color.b, color.a }, v_dev);

            const float pm_x = dev_x - L.space_w - L.pm_w;
            draw_list->AddText({ pm_x, y }, { color.r, color.g, color.b, color.a }, "+-");

            const float q_main_w = GetQuantizedNumWidth(v_main);
            const float main_x = pm_x - L.space_w - q_main_w;
            draw_list->AddText({ main_x, y }, { color.r, color.g, color.b, color.a }, v_main);
            };

        // -------------------------------------------------------------
        // 7. 第一行绘制（对齐到 right_edge）
        // -------------------------------------------------------------
        if (is_connected_server) {
            const float l1_col3_x = right_edge - L.slot_ping_w;
            const float l1_col2_x = l1_col3_x - L.col_spacing - L.slot_cvar_w;
            const float l1_col1_x = l1_col2_x - L.col_spacing - L.slot_fps_w;

            const bool is_ping_unstable = (state.smooth_ping_jitter > 12.0f);
            const Color4f ping_color = is_ping_unstable ? col_red : col_yellow;

            DrawUnitSlot(l1_col1_x, L.slot_fps_w, line1_y, "fps:", val_fps, col_yellow, false);
            DrawUnitSlot(l1_col2_x, L.slot_cvar_w, line1_y, "var:", val_cvar, col_yellow, true);
            DrawUnitSlot(l1_col3_x, L.slot_ping_w, line1_y, "ping:", val_ping, ping_color, true);
        }
        else {
            const float l1_col2_x = right_edge - L.slot_cvar_w;
            const float l1_col1_x = l1_col2_x - L.col_spacing - L.slot_fps_w;

            DrawUnitSlot(l1_col1_x, L.slot_fps_w, line1_y, "fps:", val_fps, col_yellow, false);
            DrawUnitSlot(l1_col2_x, L.slot_cvar_w, line1_y, "var:", val_cvar, col_yellow, true);
        }

        // -------------------------------------------------------------
        // 8. 第二行绘制（对齐到 right_edge）
        // -------------------------------------------------------------
        if (is_connected_server) {
            const float l2_col2_x = right_edge - L.slot_svar_w;
            const float l2_col1_x = l2_col2_x - L.col_spacing - L.slot_sv_w;

            const Color4f sv_color = (server_avg_dt_ms > dynamic_nominal_dt_ms * 1.05f) ? col_red : col_yellow;
            const bool is_svar_unstable = (server_var_ms > 1.2f) || (server_var_delta > 0.4f);
            const Color4f svar_color = is_svar_unstable ? col_red : col_yellow;

            DrawSvSlot(l2_col1_x, L.slot_sv_w, line2_y, val_sv_main, val_sv_dev, sv_color);
            DrawUnitSlot(l2_col2_x, L.slot_svar_w, line2_y, "var:", val_svar, svar_color, true);
        }

        Shadow::PopTextOutline();
        Shadow::PopFont();
    }

    __forceinline void OnRender() {
        if (!g_Config::bShowInfo) return;

        DrawNetGraph();

        // 每帧开始前重置位置
        g_Indicators::Reset();

        static Shadow::Color col0 = { 1.f, 0.95f, 0.9f, 1.f };
        static Shadow::Color col1 = { 1.f, 1.f, 0.65f, 1.f };
        static Shadow::Color col2 = { 0.f, 1.f, 1.f, 1.f };
        static Shadow::Color col3 = { 0.f, 1.f, 0.f, 1.f };

        float fps = g_Util::GetFPS();
        float server_fps = 0.f;
        int NumPlayerConnected = 0;
        int NumTamedDinos = 0;
        int MaxTamedDinos = 0;

        SDK::AShooterGameState* GS = g_Util::GetAShooterGameState();
        if (GS) {
            server_fps = GS->ServerFramerate;
            NumPlayerConnected = GS->NumPlayerConnected;
            NumTamedDinos = GS->NumTamedDinos;
            MaxTamedDinos = GS->MaxTamedDinos;
        }

        // std::string fps_text = std::format("FPS: {:.0f}", fps);
        // std::string server_fps_text = std::format("SV: {:.1f}", server_fps);

        std::string player = _XOR_(u8"玩家").str();
        std::string tamed = _XOR_(u8"驯养").str();

        std::string NumPlayerConnected_test = std::format("{}: {}", player, NumPlayerConnected);
        std::string TamedDinos_test = std::format("{}: {}/{}", tamed, NumTamedDinos, MaxTamedDinos );

        // float ping = g_Util::GetClientPing();
        // std::string ping_text = std::format("Ping: {:.0f}", ping);

        // g_Indicators::AddTop(col0, fps_text);

        if (GS) {
            // g_Indicators::AddTop(col1, server_fps_text);
            g_Indicators::AddTop(col2, NumPlayerConnected_test);
            g_Indicators::AddTop(col3, TamedDinos_test);
        }

        // g_Indicators::AddTop(col1, ping_text); // 不好看
    }
}