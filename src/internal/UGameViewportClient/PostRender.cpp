#pragma once
#include "../../external/CppSDK/SDK.hpp"
#include "../../external/Shadow-Gui/include/Shadow.h"
#include "../ESP/DrawESP.h"
#include "../Lua/LuaManager.h"
#include "../Util/Util.h"
#include "PostRender.h"
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

namespace g_UGameViewportClient {
    // 辅助打印：使用纯英文打印，彻底规避 VS 输出窗口的 UTF-8/GBK 乱码问题
    void LogMessageEng(const char* format, ...)
    {
        char buffer[512];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        OutputDebugStringA(buffer);
    }

    void ApplyCrossVersionViewportScan(SDK::UGameViewportClient* rcx)
    {
        if (!rcx) return;

        uintptr_t class_base = (uintptr_t)rcx;

        // 1. 获取 FViewport 物理指针 [1]
        uintptr_t* pViewportPtr = (uintptr_t*)(class_base + 0x098);
        void* pRealViewport = pViewportPtr ? (void*)*pViewportPtr : nullptr;

        // 指向当前的 EngineShowFlags [1]
        unsigned char* pEngineShowFlags = (unsigned char*)(class_base + 0x0B8);

        if (!pRealViewport || !pEngineShowFlags)
        {
            return;
        }

        // 2. 静态测试变量（Bit-level 诊断）
        static int g_TestBitIndex = 0;           // 当前测试的位索引 (0 ~ 63，对应前 8 字节的 64 个位)
        static bool g_BitDisabled[64] = { false }; // 记录每个 bit 是否被我们强制关闭
        static unsigned char g_OriginalBytes[8] = { 0 }; // 备份前 8 字节原始状态
        static bool bBackupDone = false;

        if (!bBackupDone)
        {
            memcpy(g_OriginalBytes, pEngineShowFlags, 8);
            bBackupDone = true;
            LogMessageEng("[*] ShowFlags front 8 bytes backed up successfully.\n");
        }


        // 27 删掉反射
            // 37 删掉除了世界网格之外的所有东西
        // 38 减少渲染距离
            // 39 删掉除了世界网格之外的所有东西
        // 41 删掉阴影
            // 42 关掉光柱，不知道干啥的
            // 43 删掉角色模型
            // 44 关掉光柱，不知道干啥的
            // 47 删掉光源
            // 48 会删掉水源、方舟、石头的一部分贴图
            // 49 删掉地面贴图
            // 50 禁用一部分后处理？不确定
            // 54 删掉天空，应该把体积云雾都一起删了，帧数大幅提升
            // 60 会让后处理变得很奇怪
        // r.SkylightIntensityMultiplier 5




        // -----------------------------------------------------------------
        // 3. 键盘实时交互诊断机制（诊断期间免去反复编译修改的痛苦）：
        //    -> 按 [F5] 键：递增测试的 bit 索引 (0 -> 1 -> ... -> 63)
        //    -> 按 [F6] 键：递减测试的 bit 索引
        //    -> 按 [F7] 键：强行将该单独的 bit 改写为 0 (关闭该 bit 开关)
        //    -> 按 [F8] 键：重置恢复原始画质
        // -----------------------------------------------------------------
        bool bStateChanged = false;

        if (GetAsyncKeyState(VK_F5) & 0x0001)
        {
            g_TestBitIndex = (g_TestBitIndex + 1) % 64;
            int byte_pos = g_TestBitIndex / 8;
            int bit_pos = g_TestBitIndex % 8;
            LogMessageEng("[*] Bit Diagnostic: Active Index [%d] -> (Byte: %d, Bit: %d)\n",
                g_TestBitIndex, byte_pos, bit_pos);
            bStateChanged = true;
        }

        if (GetAsyncKeyState(VK_F6) & 0x0001)
        {
            g_TestBitIndex = (g_TestBitIndex - 1 + 64) % 64;
            int byte_pos = g_TestBitIndex / 8;
            int bit_pos = g_TestBitIndex % 8;
            LogMessageEng("[*] Bit Diagnostic: Active Index [%d] -> (Byte: %d, Bit: %d)\n",
                g_TestBitIndex, byte_pos, bit_pos);
            bStateChanged = true;
        }

        if (GetAsyncKeyState(VK_F7) & 0x0001)
        {
            g_BitDisabled[g_TestBitIndex] = !g_BitDisabled[g_TestBitIndex];
            LogMessageEng("[!] Bit Diagnostic: Toggled Bit [%d] state to: %s\n",
                g_TestBitIndex, g_BitDisabled[g_TestBitIndex] ? "DISABLED (0)" : "ENABLED (1)");
            bStateChanged = true;
        }

        if (GetAsyncKeyState(VK_F8) & 0x0001)
        {
            // 恢复前 8 字节为原始状态
            memcpy(pEngineShowFlags, g_OriginalBytes, 8);
            memset(g_BitDisabled, 0, sizeof(g_BitDisabled));
            LogMessageEng("[!] Bit Diagnostic: One-click restore executed. All bits reverted to original values.\n");
            bStateChanged = true;
        }

        // 4. 实时计算并应用当前的 bit 级修改
        if (bStateChanged)
        {
            // 恢复前 8 字节为最原始状态，防止脏数据干扰
            memcpy(pEngineShowFlags, g_OriginalBytes, 8);

            // 针对被我们标记为 DISABLED 的 bit，执行精确的按位清除
            for (int i = 0; i < 64; i++)
            {
                if (g_BitDisabled[i])
                {
                    int byte_offset = i / 8;
                    int bit_position = i % 8;

                    // 按位清除运算 (例如将第 2 位设为 0: pEngineShowFlags[byte] &= ~(1 << 2))
                    pEngineShowFlags[byte_offset] &= ~(1 << bit_position);
                }
            }
        }
    }

    void PostRender(SDK::UGameViewportClient* rcx, SDK::UCanvas* canvas) {
        ApplyCrossVersionViewportScan(rcx);

        SDK::UWorld* World = SDK::UWorld::GetWorld();

        Shadow::NewFrame(canvas);
        g_Util::Welcome(World, canvas);
        g_DrawESP::DrawESP(canvas);
        LuaManager::Get().Lua_OnPostRender();
        Shadow::Render();
    }
}