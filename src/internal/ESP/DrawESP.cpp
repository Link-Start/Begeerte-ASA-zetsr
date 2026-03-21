// DrawESP.cpp
#include "../../external/Minimal-D3D12-Hook-ImGui/Main/mdx12_api.h"
#include "../../external/SDK/SDK_Headers.hpp"
#include "ESP.h"
#include "../Config/Configs.h"
#include "DrawESP.h"
#include "../Util/Util.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <memory>
#include <cstdio>   // snprintf

namespace g_DrawESP {
    static constexpr float FADE_IN_TIME = 0.10f;
    static constexpr float FADE_OUT_TIME = 0.20f;

    // -----------------------------------------------------------------------
    // 小工具：用 snprintf 替代 std::to_string 避免每帧堆分配
    // -----------------------------------------------------------------------
    static inline std::string IntToStr(int v)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", v);
        return buf; // NRVO / SSO，比 std::to_string 开销低
    }

    struct CachedFlag {
        std::string text;
        ImU32       color;
        g_ESP::FlagPos pos;
    };

    struct CachedBar {
        float               currentValue;
        float               maxValue;
        ImU32               color;
        g_ESP::BarPos       pos;
        g_ESP::BarOrientation orientation;
    };

    // -----------------------------------------------------------------------
    // 实体类型枚举，缓存 IsA 结果，避免每帧重复调用虚函数链
    // -----------------------------------------------------------------------
    enum class ActorType : uint8_t {
        Unknown = 0,
        PrimalCharacter,
        PhysicsVolume,
        DroppedItem,
        PrimalStructure,
    };

    struct ESPEntry {
        uintptr_t      actorKey = 0;
        ActorType      actorType = ActorType::Unknown; // 缓存类型，避免每帧 IsA
        SDK::FVector   lastWorldLoc{};
        g_ESP::BoxRect cachedRect;
        std::string    name;

        std::vector<CachedFlag> flags;
        std::vector<CachedBar>  bars;

        ImU32  boxColor = 0;
        ImU32  nameColor = 0;
        float  configBoxAlpha = 1.0f;
        float  targetAlpha = 0.0f;
        float  alpha = 0.0f;
        float  lastSeenTime = 0.0f;
        bool   aliveThisFrame = false;
        bool   isOOF = false;
        bool   isItem = false;
        float  cachedHP = 0.0f;
        float  cachedMaxHP = 0.0f;
        float  cachedTorpor = 0.0f;
        float  cachedMaxTorpor = 0.0f;

        bool shouldDrawBox = false;
        bool shouldDrawHealthBar = false;
        bool shouldDrawName = false;
        bool shouldDrawDistance = false;
        bool shouldDrawTorpor = false;
    };

    static std::unordered_map<uintptr_t, ESPEntry> s_entries;

    struct WaterCandidate {
        SDK::AActor* actor;
        float           dist;
        SDK::FVector2D  screenPos;
        SDK::FVector    surfaceLoc;
    };

    // 重用 vector，避免每帧分配/释放
    static std::vector<WaterCandidate> waterCandidates;
    static std::vector<uintptr_t>      s_toErase;

    // -----------------------------------------------------------------------
    static float ApproachAlpha(float cur, float target, float deltaSeconds, float fadeTime)
    {
        if (fadeTime <= 0.0f) return target;
        float diff = target - cur;
        float maxStep = deltaSeconds / fadeTime;
        if (fabsf(diff) <= maxStep) return target;
        return cur + (diff > 0.0f ? maxStep : -maxStep);
    }

    // -----------------------------------------------------------------------
    // 根据 className 映射颜色——用 strstr 代替 std::string::find，减少转换开销
    // -----------------------------------------------------------------------
    static ImU32 ResolveDroppedItemColor(const std::string& className, float itemRating, int quantity)
    {
        const char* cn = className.c_str();

        if (strstr(cn, "PrimalItem_WeaponEmptyCryopod"))
            return g_Util::GetU32Color(g_Config::DroppedItemCryopodColor);
        if (strstr(cn, "Egg"))
            return g_Util::GetU32Color(g_Config::DroppedItemEggColor);
        if (quantity >= 1000)
            return g_Util::GetU32Color(g_Config::DroppedItemPiledColor);
        if (strstr(cn, "PrimalItemResource_FungalWood") || strstr(cn, "PrimalItemResource_Wood"))
            return g_Util::GetU32Color(g_Config::DroppedItemWoodColor);
        if (strstr(cn, "PrimalItemResource_Thatch"))
            return g_Util::GetU32Color(g_Config::DroppedItemThatchColor);
        if (strstr(cn, "PrimalItemResource_Hide"))
            return g_Util::GetU32Color(g_Config::DroppedItemHideColor);
        if (strstr(cn, "PrimalItemResource_Pelt"))
            return g_Util::GetU32Color(g_Config::DroppedItemPeltColor);
        if (strstr(cn, "PrimalItemResource_Keratin"))
            return g_Util::GetU32Color(g_Config::DroppedItemKeratinColor);
        if (strstr(cn, "PrimalItemResource_Chitin"))
            return g_Util::GetU32Color(g_Config::DroppedItemChitinColor);
        if (strstr(cn, "PrimalItemResource_CorruptedPolymer"))
            return g_Util::GetU32Color(g_Config::DroppedItemCorruptedPolymerColor);
        if (strstr(cn, "PrimalItemResource_Polymer_Organic"))
            return g_Util::GetU32Color(g_Config::DroppedItemPolymer_OrganicColor);
        if (strstr(cn, "PrimalItemResource_Polymer"))
            return g_Util::GetU32Color(g_Config::DroppedItemPolymerColor);
        if (strstr(cn, "PrimalItemResource_ScrapMetalIngot") ||
            strstr(cn, "PrimalItemResource_MetalIngot") ||
            strstr(cn, "PrimalItemResource_ScrapMetal") ||
            strstr(cn, "PrimalItemResource_Metal"))
            return g_Util::GetU32Color(g_Config::DroppedItemMetalColor);
        if (strstr(cn, "PrimalItemResource_Stone"))
            return g_Util::GetU32Color(g_Config::DroppedItemStoneColor);
        if (strstr(cn, "PrimalItemResource_Crystal"))
            return g_Util::GetU32Color(g_Config::DroppedItemCrystalColor);
        if (strstr(cn, "PrimalItemResource_Gem_Fertile") ||
            strstr(cn, "PrimalItemResource_Gem_BioLum") ||
            strstr(cn, "PrimalItemResource_Gem_Element") ||
            strstr(cn, "PrimalItemResource_BlueSap") ||
            strstr(cn, "PrimalItemResource_RedSap"))
            return g_Util::GetU32Color(g_Config::DroppedItemGemColor);
        if (strstr(cn, "PrimalItemResource_Silicon") ||
            strstr(cn, "PrimalItemResource_BlackPearl"))
            return g_Util::GetU32Color(g_Config::DroppedItemPearlColor);
        if (strstr(cn, "PrimalItemConsumable_SpoiledMeat"))
            return g_Util::GetU32Color(g_Config::DroppedItemSpoiledMeatColor);
        if (strstr(cn, "PrimalItemConsumable_RawMeat") ||
            strstr(cn, "PrimalItemConsumable_RawPrimeMeat") ||
            strstr(cn, "PrimalItemConsumable_RawMutton") ||
            strstr(cn, "PrimalItemConsumable_RawPrimeMeat_Fish") ||
            strstr(cn, "PrimalItemConsumable_RawMeat_Fish") ||
            strstr(cn, "PrimalItemConsumable_CookedMeat") ||
            strstr(cn, "PrimalItemConsumable_CookedPrimeMeat") ||
            strstr(cn, "PrimalItemConsumable_CookedLambChop") ||
            strstr(cn, "PrimalItemConsumable_CookedPrimeMeat_Fish") ||
            strstr(cn, "PrimalItemConsumable_CookedMeat_Fish") ||
            strstr(cn, "PrimalItemConsumable_CookedMeat_Jerky") ||
            strstr(cn, "PrimalItemConsumable_CookedPrimeMeat_Jerky"))
            return g_Util::GetU32Color(g_Config::DroppedItemMeatColor);

        // 按品质评级上色
        if (itemRating >= 10.0f) return g_Util::ToImColor(0, 255, 255, 255);
        if (itemRating >= 7.0f) return g_Util::ToImColor(255, 255, 0, 255);
        if (itemRating >= 4.5f) return g_Util::ToImColor(160, 32, 240, 255);
        if (itemRating >= 2.5f) return g_Util::ToImColor(0, 191, 255, 255);
        if (itemRating >= 1.25f) return g_Util::ToImColor(50, 205, 50, 255);

        return g_Util::GetU32Color(g_Config::DroppedItemNameColor);
    }

    // -----------------------------------------------------------------------
    void DrawESP()
    {
        SDK::UWorld* World = SDK::UWorld::GetWorld();
        if (!World || !World->GameState || !World->PersistentLevel) return;

        SDK::APlayerController* LocalPC = g_Util::GetLocalPC();
        if (!LocalPC || !LocalPC->Pawn) {
            for (auto& kv : s_entries) {
                kv.second.targetAlpha = 0.0f;
                kv.second.aliveThisFrame = false;
            }
            return;
        }

        SDK::APlayerState* LocalPS = LocalPC->PlayerState;
        if (!LocalPS) {
            for (auto& kv : s_entries) {
                kv.second.targetAlpha = 0.0f;
                kv.second.aliveThisFrame = false;
            }
            return;
        }

        // 只调用一次 GetIO()
        ImGuiIO& io = ImGui::GetIO();
        const float screenW = io.DisplaySize.x;
        const float screenH = io.DisplaySize.y;
        const float deltaTime = io.DeltaTime;

        SDK::APrimalCharacter* LocalChar = static_cast<SDK::APrimalCharacter*>(LocalPC->Pawn);

        SDK::TArray<SDK::AActor*>& Actors = World->PersistentLevel->Actors;
        const int actorCount = Actors.Num(); // 缓存，避免每次循环调用

        waterCandidates.clear();

        // 预先缓存搜索过滤字符串（避免每次循环从 char[] 构造 std::string）
        const char* rawEntityFilter = g_Config::entitySearchBuf;
        const bool  hasEntityFilter = g_Config::bEnableFilter && rawEntityFilter[0] != '\0';
        const char* rawStructFilter = g_Config::structureSearchBuf;
        const bool  hasStructureFilter = g_Config::bEnableStructureFilter && rawStructFilter[0] != '\0';

        // 标记所有条目为非活跃
        for (auto& kv : s_entries)
            kv.second.aliveThisFrame = false;

        // ================================================================
        // 主 Actor 遍历
        // ================================================================
        for (int i = 0; i < actorCount; i++) {
            SDK::AActor* TargetActor = Actors[i];
            if (!TargetActor || TargetActor == LocalPC->Pawn) continue;

            uintptr_t  key = reinterpret_cast<uintptr_t>(TargetActor);
            ESPEntry& entry = s_entries[key]; // operator[] 首次访问时构造

            entry.actorKey = key;
            entry.lastSeenTime = 0.0f;

            if (TargetActor->bHidden) {
                entry.targetAlpha = 0.0f;
                entry.aliveThisFrame = false;
                continue;
            }

            // ---- 缓存 Actor 类型（首次判断后不再重复 IsA） ----
            if (entry.actorType == ActorType::Unknown) {
                if (TargetActor->IsA(SDK::APrimalCharacter::StaticClass()))  entry.actorType = ActorType::PrimalCharacter;
                else if (TargetActor->IsA(SDK::APhysicsVolume::StaticClass()))    entry.actorType = ActorType::PhysicsVolume;
                else if (TargetActor->IsA(SDK::ADroppedItem::StaticClass()))      entry.actorType = ActorType::DroppedItem;
                else if (TargetActor->IsA(SDK::APrimalStructure::StaticClass()))  entry.actorType = ActorType::PrimalStructure;
            }

            // ---- 缓存世界位置（只调用一次） ----
            const SDK::FVector actorLoc = TargetActor->K2_GetActorLocation();
            entry.lastWorldLoc = actorLoc;
            entry.aliveThisFrame = true;

            // ============================================================
            // Branch: PrimalCharacter
            // ============================================================
            if (entry.actorType == ActorType::PrimalCharacter) {
                SDK::APrimalCharacter* TargetChar = static_cast<SDK::APrimalCharacter*>(TargetActor);
                SDK::APlayerState* TargetPS = TargetChar->PlayerState;

                // 计算距离一次，后续复用
                const float dist = (LocalPC && LocalPC->Pawn && TargetActor) ? LocalPC->Pawn->GetDistanceTo(TargetActor) * 0.01f : 0.0f;
                const bool isDead = TargetChar->IsDead();

                if (isDead) {
                    g_ESP::RelationType relation = g_ESP::GetRelation(TargetChar, LocalChar);
                    bool   bShowRagdoll = false;
                    float* RagdollCol = nullptr;

                    if (relation == g_ESP::RelationType::Team) {
                        bShowRagdoll = g_Config::bDrawRagdollTeam;
                        RagdollCol = g_Config::RagdollColorTeam;
                    }
                    else {
                        bShowRagdoll = g_Config::bDrawRagdoll;
                        RagdollCol = g_Config::RagdollColor;
                    }

                    if (!bShowRagdoll) {
                        entry.targetAlpha = 0.0f;
                        entry.aliveThisFrame = false;
                        continue;
                    }

                    g_ESP::BoxRect rect = g_ESP::DrawBox(TargetActor,
                        RagdollCol[0] * 255.0f, RagdollCol[1] * 255.0f,
                        RagdollCol[2] * 255.0f, RagdollCol[3] * 255.0f, 0.5f, true);

                    if (!rect.valid) {
                        entry.targetAlpha = 0.0f;
                        continue;
                    }

                    entry.cachedRect = rect;
                    entry.boxColor = g_Util::GetU32Color(RagdollCol);
                    entry.configBoxAlpha = RagdollCol[3];
                    entry.targetAlpha = 1.0f;
                    entry.aliveThisFrame = true;

                    if (relation == g_ESP::RelationType::Team) {
                        entry.shouldDrawBox = g_Config::bDrawBoxTeam;
                        entry.shouldDrawName = g_Config::bDrawNameTeam;
                        entry.shouldDrawDistance = g_Config::bDrawDistanceTeam;
                    }
                    else {
                        entry.shouldDrawBox = g_Config::bDrawBox;
                        entry.shouldDrawName = g_Config::bDrawName;
                        entry.shouldDrawDistance = g_Config::bDrawDistance;
                    }
                    entry.shouldDrawHealthBar = false; // 尸体不需要血条
                    entry.shouldDrawTorpor = false; // 尸体不需要眩晕条
                    entry.flags.clear();
                    entry.bars.clear();

                    if (entry.shouldDrawName) {
                        std::string deadName = TargetPS
                            ? TargetPS->GetPlayerName().ToString()
                            : TargetChar->GetDescriptiveName().ToString();
                        entry.flags.push_back({ std::move(deadName), entry.boxColor, g_ESP::FlagPos::Top });
                    }
                    if (entry.shouldDrawDistance) {
                        entry.flags.push_back({
                            IntToStr((int)dist) + "m",
                            g_Util::ToImColor(200, 200, 200, 255),
                            g_ESP::FlagPos::Right
                            });
                    }
                    continue;
                }

                // 活体角色搜索过滤
                if (hasEntityFilter) {
                    std::string nameForESP = TargetPS
                        ? TargetPS->GetPlayerName().ToString()
                        : TargetChar->GetDescriptiveName().ToString();
                    if (!g_Util::IsEntityMatchMulti(nameForESP, rawEntityFilter)) {
                        entry.targetAlpha = 0.0f;
                        entry.aliveThisFrame = false;
                        continue;
                    }
                }

                g_ESP::RelationType relation = g_ESP::GetRelation(TargetChar, LocalChar);

                bool   bDrawBox = false, bDrawHealthBar = false, bDrawName = false;
                bool   bDrawGrowth = false, bDrawDistance = false, bDrawTorpor = false;
                float* BoxColor = nullptr;
                float* NameColor = nullptr;
                float* DistanceColor = nullptr;
                float* TorporColor = nullptr;

                if (relation == g_ESP::RelationType::Team) {
                    bDrawBox = g_Config::bDrawBoxTeam;
                    BoxColor = g_Config::BoxColorTeam;
                    bDrawHealthBar = g_Config::bDrawHealthBarTeam;
                    bDrawName = g_Config::bDrawNameTeam;
                    NameColor = g_Config::NameColorTeam;
                    bDrawGrowth = g_Config::bDrawGrowthTeam;
                    bDrawDistance = g_Config::bDrawDistanceTeam;
                    DistanceColor = g_Config::DistanceColorTeam;
                    bDrawTorpor = g_Config::bDrawTorporTeam;
                    TorporColor = g_Config::TorporColorTeam;
                }
                else {
                    bDrawBox = g_Config::bDrawBox;
                    BoxColor = g_Config::BoxColor;
                    bDrawHealthBar = g_Config::bDrawHealthBar;
                    bDrawName = g_Config::bDrawName;
                    NameColor = g_Config::NameColor;
                    bDrawGrowth = g_Config::bDrawGrowth;
                    bDrawDistance = g_Config::bDrawDistance;
                    DistanceColor = g_Config::DistanceColor;
                    bDrawTorpor = g_Config::bDrawTorpor;
                    TorporColor = g_Config::TorporColor;
                }

                g_ESP::BoxRect rect = g_ESP::DrawBox(TargetActor,
                    BoxColor[0] * 255.0f, BoxColor[1] * 255.0f,
                    BoxColor[2] * 255.0f, BoxColor[3] * 255.0f, 0.5f, true);

                entry.cachedRect = rect;
                entry.boxColor = g_Util::GetU32Color(BoxColor);
                entry.nameColor = g_Util::GetU32Color(NameColor);
                entry.configBoxAlpha = BoxColor[3];
                entry.isItem = false;
                entry.isOOF = false;
                entry.cachedHP = TargetChar->GetHealth();
                entry.cachedMaxHP = TargetChar->GetMaxHealth();

                SDK::UPrimalCharacterStatusComponent* StatusComp = TargetChar->GetCharacterStatusComponent();
                if (StatusComp) {
                    entry.cachedTorpor = StatusComp->CurrentStatusValues[(int)SDK::EPrimalCharacterStatusValue::Torpidity];
                    entry.cachedMaxTorpor = StatusComp->MaxStatusValues[(int)SDK::EPrimalCharacterStatusValue::Torpidity];
                }
                else {
                    entry.cachedTorpor = 0.0f;
                    entry.cachedMaxTorpor = 0.0f;
                }

                entry.shouldDrawBox = bDrawBox;
                entry.shouldDrawHealthBar = bDrawHealthBar;
                entry.shouldDrawName = bDrawName;
                entry.shouldDrawDistance = bDrawDistance;
                entry.shouldDrawTorpor = bDrawTorpor;

                // 名字字符串（只在需要时构造）
                if (bDrawName) {
                    const char* genderSuffix = TargetActor->IsFemale() ? "-F" : "-M";
                    entry.name = TargetPS
                        ? TargetPS->GetPlayerName().ToString() + genderSuffix
                        : TargetChar->GetDescriptiveName().ToString() + genderSuffix;
                }
                else {
                    entry.name.clear();
                }

                entry.flags.clear();
                entry.bars.clear();

                if (bDrawName && !entry.name.empty())
                    entry.flags.push_back({ entry.name, entry.nameColor, g_ESP::FlagPos::Top });

                if (bDrawHealthBar) {
                    const float healthPct = (entry.cachedMaxHP > 0.0f) ? (entry.cachedHP / entry.cachedMaxHP) : 0.0f;
                    const ImU32 hpCol = g_Util::GetHealthColor(healthPct);
                    entry.flags.push_back({ IntToStr((int)entry.cachedHP), hpCol, g_ESP::FlagPos::Left });
                    entry.bars.push_back({ entry.cachedHP, entry.cachedMaxHP, hpCol, g_ESP::BarPos::Left, g_ESP::BarOrientation::Vertical });
                }

                if (bDrawTorpor && entry.cachedMaxTorpor > 0.0f) {
                    const ImU32 torporCol = g_Util::GetU32Color(TorporColor);
                    entry.flags.push_back({
                        IntToStr((int)entry.cachedTorpor) + "/" + IntToStr((int)entry.cachedMaxTorpor),
                        torporCol, g_ESP::FlagPos::Bottom
                        });
                    entry.bars.push_back({ entry.cachedTorpor, entry.cachedMaxTorpor, torporCol, g_ESP::BarPos::Bottom, g_ESP::BarOrientation::Horizontal });
                }

                if (bDrawDistance)
                    entry.flags.push_back({ IntToStr((int)dist) + "m", g_Util::GetU32Color(DistanceColor), g_ESP::FlagPos::Right });

                // 屏幕空间可见性判断
                SDK::FVector2D screenPos;
                if (LocalPC->ProjectWorldLocationToScreen(actorLoc, &screenPos, false)) {
                    const bool onScreen = screenPos.X > 0 && screenPos.X < screenW
                        && screenPos.Y > 0 && screenPos.Y < screenH;
                    if (onScreen) {
                        entry.targetAlpha = entry.configBoxAlpha;
                        entry.isOOF = false;
                    }
                    else if (g_Config::bEnableOOF) {
                        entry.isOOF = true;
                        entry.targetAlpha = entry.configBoxAlpha;
                        // OOF 只保留名字与距离
                        entry.flags.clear();
                        if (bDrawName && !entry.name.empty())
                            entry.flags.push_back({ entry.name, entry.nameColor, g_ESP::FlagPos::Right });
                        if (bDrawDistance)
                            entry.flags.push_back({ IntToStr((int)dist) + "m", g_Util::GetU32Color(DistanceColor), g_ESP::FlagPos::Right });
                    }
                    else {
                        entry.targetAlpha = 0.0f;
                        entry.aliveThisFrame = false;
                    }
                }
                else {
                    entry.targetAlpha = 0.0f;
                    entry.aliveThisFrame = false;
                }
            }

            // ============================================================
            // Branch: WaterVolume
            // ============================================================
            else if (entry.actorType == ActorType::PhysicsVolume && g_Config::bDrawWater) {
                SDK::APhysicsVolume* PV = static_cast<SDK::APhysicsVolume*>(TargetActor);
                if (!PV->bWaterVolume && !PV->bDynamicWaterVolume) {
                    entry.targetAlpha = 0.0f;
                    entry.aliveThisFrame = false;
                    continue;
                }

                SDK::FVector Origin, Extend;
                TargetActor->GetActorBounds(false, &Origin, &Extend, false);
                const SDK::FVector WaterSurfaceLoc = { Origin.X, Origin.Y, Origin.Z + Extend.Z };
                const float dist = (LocalPC && LocalPC->Pawn && TargetActor) ? LocalPC->Pawn->GetDistanceTo(TargetActor) * 0.01f : 0.0f;

                entry.targetAlpha = 0.0f;
                entry.aliveThisFrame = false;

                SDK::FVector2D screenPos;
                if (LocalPC->ProjectWorldLocationToScreen(WaterSurfaceLoc, &screenPos, false)) {
                    if (screenPos.X > 0 && screenPos.X < screenW && screenPos.Y > 0 && screenPos.Y < screenH) {
                        waterCandidates.push_back({ TargetActor, dist, screenPos, WaterSurfaceLoc });
                    }
                }
            }

            // ============================================================
            // Branch: DroppedItem
            // ============================================================
            else if (entry.actorType == ActorType::DroppedItem && g_Config::bDrawDroppedItems) {
                SDK::ADroppedItem* DroppedItem = static_cast<SDK::ADroppedItem*>(TargetActor);
                SDK::UPrimalItem* Item = DroppedItem ? DroppedItem->MyItem : nullptr;

                if (!Item) {
                    entry.targetAlpha = 0.0f;
                    entry.aliveThisFrame = false;
                    continue;
                }

                const float dist = (LocalPC && LocalPC->Pawn && TargetActor) ? LocalPC->Pawn->GetDistanceTo(TargetActor) * 0.01f : 0.0f;
                if (dist > g_Config::DroppedItemMaxDistance) {
                    entry.targetAlpha = 0.0f;
                    entry.aliveThisFrame = false;
                    continue;
                }

                SDK::FVector2D screenPos;
                if (!LocalPC->ProjectWorldLocationToScreen(actorLoc, &screenPos, false)
                    || !(screenPos.X > 0 && screenPos.X < screenW && screenPos.Y > 0 && screenPos.Y < screenH))
                {
                    entry.targetAlpha = 0.0f;
                    entry.aliveThisFrame = false;
                    continue;
                }

                entry.targetAlpha = 1.0f;
                entry.isItem = true;
                entry.cachedRect.topLeft = ImVec2(screenPos.X - 5, screenPos.Y - 5);
                entry.cachedRect.bottomRight = ImVec2(screenPos.X + 5, screenPos.Y + 5);
                entry.cachedRect.valid = true;

                // 物品名（按优先级取第一个有效来源）
                std::string itemName;
                if (Item->CustomItemName.IsValid() && !Item->CustomItemName.ToString().empty()) {
                    itemName = Item->CustomItemName.ToString();
                }
                else if (Item->DescriptiveNameBase.IsValid()) {
                    itemName = Item->DescriptiveNameBase.ToString();
                }
                else {
                    itemName = Item->Class ? Item->Class->GetName() : "Unknown Item";
                }

                const std::string className = Item->Class ? Item->Class->GetName() : "";
                const int         quantity = Item->ItemQuantity;
                const ImU32       finalCol = ResolveDroppedItemColor(className, Item->ItemRating, quantity);

                entry.flags.clear();
                entry.bars.clear();

                std::string label = "[" + itemName + "]";
                if (quantity > 1)       label += " x" + IntToStr(quantity);
                if (Item->bIsBlueprint) label = "[BP] " + label;

                entry.flags.push_back({ std::move(label),  finalCol, g_ESP::FlagPos::Right });
                entry.flags.push_back({ IntToStr((int)dist) + "m",
                    g_Util::GetU32Color(g_Config::DroppedItemDistanceColor), g_ESP::FlagPos::Right });

                entry.boxColor = finalCol;
                entry.nameColor = g_Util::GetU32Color(g_Config::DroppedItemNameColor);
                entry.shouldDrawBox = false;
                entry.shouldDrawHealthBar = false;
                entry.shouldDrawName = false;
                entry.shouldDrawDistance = true;
                entry.shouldDrawTorpor = false;
            }

            // ============================================================
            // Branch: PrimalStructure
            // ============================================================
            else if (entry.actorType == ActorType::PrimalStructure && g_Config::bDrawStructures) {
                SDK::APrimalStructure* Structure = static_cast<SDK::APrimalStructure*>(TargetActor);

                if (Structure->Health <= 0.0f) {
                    entry.targetAlpha = 0.0f;
                    entry.aliveThisFrame = false;
                    continue;
                }

                if (hasStructureFilter) {
                    std::string sName = Structure->GetDescriptiveName().ToString();
                    if (sName.empty() || sName == "None") sName = "Structure";
                    if (!g_Util::IsStructureMatchMulti(sName, rawStructFilter)) {
                        entry.targetAlpha = 0.0f;
                        entry.aliveThisFrame = false;
                        continue;
                    }
                }

                const float dist = (LocalPC && LocalPC->Pawn && TargetActor) ? LocalPC->Pawn->GetDistanceTo(TargetActor) * 0.01f : 0.0f;
                if (dist > g_Config::StructureMaxDistance) {
                    entry.targetAlpha = 0.0f;
                    entry.aliveThisFrame = false;
                    continue;
                }

                SDK::FVector2D screenPos;
                if (!LocalPC->ProjectWorldLocationToScreen(actorLoc, &screenPos, false)
                    || !(screenPos.X > 0 && screenPos.X < screenW && screenPos.Y > 0 && screenPos.Y < screenH))
                {
                    entry.targetAlpha = 0.0f;
                    entry.aliveThisFrame = false;
                    continue;
                }

                entry.aliveThisFrame = true;
                entry.targetAlpha = 1.0f;
                entry.isItem = true;
                entry.cachedRect.valid = true;
                entry.cachedRect.topLeft = ImVec2(screenPos.X - 2, screenPos.Y - 2);
                entry.cachedRect.bottomRight = ImVec2(screenPos.X + 2, screenPos.Y + 2);

                std::string sName = Structure->GetDescriptiveName().ToString();
                if (sName.empty() || sName == "None") sName = "Structure";

                const float curHP = Structure->Health;
                const float maxHP = Structure->MaxHealth;
                const float healthPct = (maxHP > 0.0f) ? (curHP / maxHP) : 0.0f;
                const int   hpPctInt = (int)(healthPct * 100.0f);
                const ImU32 hpColor = g_Util::GetHealthColor(healthPct);

                std::string ownerStr = Structure->OwnerName.ToString();
                if (ownerStr.empty() || ownerStr == "None") ownerStr = "*";

                entry.flags.clear();
                entry.bars.clear();
                entry.flags.push_back({ "[" + sName + "]",
                    g_Util::GetU32Color(g_Config::StructureNameColor), g_ESP::FlagPos::Right });
                entry.flags.push_back({ std::move(ownerStr),
                    g_Util::GetU32Color(g_Config::StructureOwnerColor), g_ESP::FlagPos::Right });
                entry.flags.push_back({
                    IntToStr((int)curHP) + " (" + IntToStr(hpPctInt) + "%)",
                    hpColor, g_ESP::FlagPos::Right
                    });
                entry.flags.push_back({ IntToStr((int)dist) + "m",
                    g_Util::GetU32Color(g_Config::StructureDistanceColor), g_ESP::FlagPos::Right });

                entry.shouldDrawTorpor = false;
            }
            else {
                // actorType == Unknown 或对应功能未启用
                entry.targetAlpha = 0.0f;
                entry.aliveThisFrame = false;
            }
        } // end actor loop

        // 非活跃条目 → targetAlpha = 0
        for (auto& kv : s_entries) {
            if (!kv.second.aliveThisFrame)
                kv.second.targetAlpha = 0.0f;
        }

        // ----------------------------------------------------------------
        // 水源过滤：只显示最近的 WaterMaxCount 个
        // ----------------------------------------------------------------
        if (g_Config::bDrawWater && !waterCandidates.empty()) {
            std::sort(waterCandidates.begin(), waterCandidates.end(),
                [](const WaterCandidate& a, const WaterCandidate& b) {
                    return a.dist < b.dist;
                });

            const int showCount = ((int)waterCandidates.size() < g_Config::WaterMaxCount)
                ? (int)waterCandidates.size()
                : g_Config::WaterMaxCount;

            // 颜色预先计算，不在循环内重复调用
            const ImU32 waterColor = g_Util::GetU32Color(g_Config::WaterNameColor);
            const ImU32 waterDistColor = g_Util::GetU32Color(g_Config::WaterDistanceColor);

            // 水源标签静态缓存，避免每帧宽字符转换
            static const std::string kWaterLabel = SDK::FString(L"[水源]").ToString();

            for (int wi = 0; wi < showCount; wi++) {
                const WaterCandidate& wc = waterCandidates[wi];
                uintptr_t             wkey = reinterpret_cast<uintptr_t>(wc.actor);
                ESPEntry& wEntry = s_entries[wkey];

                wEntry.aliveThisFrame = true;
                wEntry.targetAlpha = 1.0f;
                wEntry.isItem = true;
                wEntry.cachedRect.topLeft = ImVec2(wc.screenPos.X - 2, wc.screenPos.Y - 2);
                wEntry.cachedRect.bottomRight = ImVec2(wc.screenPos.X + 2, wc.screenPos.Y + 2);
                wEntry.cachedRect.valid = true;
                wEntry.flags.clear();
                wEntry.bars.clear();

                wEntry.flags.push_back({ kWaterLabel, waterColor, g_ESP::FlagPos::Right });
                wEntry.flags.push_back({ IntToStr((int)wc.dist) + "m", waterDistColor, g_ESP::FlagPos::Right });

                wEntry.shouldDrawBox = false;
                wEntry.shouldDrawHealthBar = false;
                wEntry.shouldDrawName = false;
                wEntry.shouldDrawDistance = true;
                wEntry.shouldDrawTorpor = false;
            }
        }

        // ----------------------------------------------------------------
        // 渲染 & 淡入淡出 & 清理（一次遍历完成三件事）
        // ----------------------------------------------------------------
        s_toErase.clear();

        for (auto& kv : s_entries) {
            ESPEntry& entry = kv.second;

            const float fadeTime = (entry.targetAlpha > entry.alpha) ? FADE_IN_TIME : FADE_OUT_TIME;
            entry.alpha = ApproachAlpha(entry.alpha, entry.targetAlpha, deltaTime, fadeTime);

            if (entry.alpha <= 0.001f && entry.targetAlpha <= 0.001f && !entry.aliveThisFrame) {
                s_toErase.push_back(kv.first);
                continue;
            }

            if (entry.alpha > 0.001f) {
                const float alpha255 = entry.alpha * 255.0f;
                ImDrawList* drawList = ImGui::GetBackgroundDrawList();

                if (!entry.isItem && entry.shouldDrawBox && entry.cachedRect.valid) {
                    ImVec4 boxF = ImGui::ColorConvertU32ToFloat4(entry.boxColor);
                    float  drawA = entry.configBoxAlpha * entry.alpha;
                    ImU32  bgShadow = ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, drawA));
                    ImU32  col = ImGui::ColorConvertFloat4ToU32(ImVec4(boxF.x, boxF.y, boxF.z, drawA));
                    drawList->AddRect(
                        ImVec2(entry.cachedRect.topLeft.x - 1, entry.cachedRect.topLeft.y - 1),
                        ImVec2(entry.cachedRect.bottomRight.x + 1, entry.cachedRect.bottomRight.y + 1),
                        bgShadow, 0.0f, 0, 1.5f);
                    drawList->AddRect(entry.cachedRect.topLeft, entry.cachedRect.bottomRight, col, 0.0f, 0, 1.0f);
                }

                g_ESP::BarManager bm;
                bm.Reset();
                for (const auto& bar : entry.bars)
                    bm.AddBar(entry.cachedRect, bar.currentValue, bar.maxValue, bar.color, bar.pos, bar.orientation, alpha255);

                g_ESP::FlagManager fm;
                fm.Reset();
                for (const auto& f : entry.flags)
                    fm.AddFlag(entry.cachedRect, f.text, f.color, f.pos, entry.alpha, &bm);

                if (entry.isOOF) {
                    std::vector<g_ESP::OOFFlag> oofFlags;
                    oofFlags.reserve(entry.flags.size());
                    for (const auto& ff : entry.flags)
                        oofFlags.push_back({ ff.text, ff.color });
                    g_ESP::DrawOutOfFOV(entry.lastWorldLoc, LocalPC, oofFlags, entry.alpha);
                }
            }
        }

        for (uintptr_t k : s_toErase)
            s_entries.erase(k);
    }
} // namespace g_DrawESP