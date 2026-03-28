#include <windows.h>

namespace g_CheatData {
	namespace VTable {
		namespace D3D12 {
			int Present = 8;
			int ExecuteCommandLists = 10;
			int ResizeBuffers = 13;
		}
	}

	namespace Signature {
		namespace UWorld {
			// 7FF7A935DA20 World Tick?
			// 7FF7A935DC40 World Post Tick?
			std::string Tick = "? ? ? ? ? 57 48 83 EC ? 48 8B 42 ? 49 8B E8 48 89 5C 24 ? 4C 8B E9 48 89 74 24 ? 48 8B";

			namespace NetDriver {
				namespace ServerConnection {
					// 7FF7A8D25520
					std::string HandleDisconnect = "? ? ? ? ? ? ? 55 57 48 8D 68 A1 48 81 EC D8 00 00 00 48 89 58 E8 48 8D 55 E7 48 89 70 E0";
				}
			}
		}

		namespace UEngine {
			namespace UGameViewportClient {
				namespace UConsole {
					std::string OutputTextLine = "48 8B C4 41 54 48 83 EC ? 48 89 58 ? 48 8B DA 48 89 68 ?";
				}
			}
		}
	}
}