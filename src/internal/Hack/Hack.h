#pragma once
#include "../../external/SDK/SDK_Headers.hpp"

namespace g_Hack {
    void DumpServerInfo();
    void Suicide(SDK::UWorld* World);
    void UnlockExplorerNotes(SDK::UWorld* World);
    void AutoFeed(SDK::UWorld* World);
    void SuperFlyer(SDK::UWorld* World);
    void ServerRequestInventoryUseItem(SDK::AShooterPlayerController* PC, SDK::UPrimalInventoryComponent* InventoryComp, SDK::FItemNetID& ItemID);
    void ServerRequestRemoteDropAllItems(SDK::AShooterPlayerController* PC, SDK::UPrimalInventoryComponent* InventoryComp, SDK::FString& CurrentCustomFolderFilter, SDK::FString& CurrentNameFilter);

}