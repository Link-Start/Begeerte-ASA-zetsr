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
		namespace AActor {
			// push rbp | push rbx | push rsi | push rdi | push r12 | push r13 | push r14 | push r15 | lea rbp, [rsp+offset] | sub rsp, alloc | movaps [rsp+offset], xmm11
			std::string TakeDamage = "40 55 53 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F 29 AC 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49";
		}

		namespace UNetConnection {
			// 7FF7A8D25520
			std::string HandleDisconnect = "? ? ? ? ? ? ? 55 57 48 8D 68 A1 48 81 EC D8 00 00 00 48 89 58 E8 48 8D 55 E7 48 89 70 E0";
		}

		namespace UMovementComponent {
			// 48 89 5C 24 ? 48 89 7C 24 ? 55 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F 29 BC 24 过期了
			// 2026/5/20 @zetsr
			// mov rax, rsp | push rbx | sub rsp, alloc | movaps [rax+offset], xmm6 | mov rbx, rcx | movaps [rax+offset], xmm7 | movaps
			// 48 8B C4 53 48 81 EC 00 01 00 00 0F 29 70 E8 48 8B D9 0F 29 78 D8 0F 28
			std::string PhysicsRotation = "48 8B C4 53 48 81 EC ? ? ? ? 0F 29 70 ? 48 8B D9 0F 29 78 ? 0F 28 F3";
		}

		namespace UWorld {
			// 7FF7A935DA20 World Tick?
			// 7FF7A935DC40 World Post Tick?
			// 2026/4/3 @zetsr
			// ? ? ? ? ? 57 48 83 EC ? 48 8B 42 ? 49 8B E8 48 89 5C 24 ? 4C 8B E9 48 89 74 24 ? 48 8B 2026/4/3 过期了

			// 2026/4/3 @zetsr
			// push rbp push r14 push r15 sub rsp, offset mov rax, [rdx + offset] mov eax, [rdx + offset] mov [rsp + offset], rbx mov ebp, ecx mov [rsp + offset], rsi mov rax, [rdx]

			// 2026/4/26 @zetsr & @Lynneren
			// push rbx; push rbp; push rsi; push rdi; push r12; push r14; push r15; sub rsp, sz; mov rax, [cookie]; xor rax, rsp; mov [rsp+sz], rax; mov r14, rcx; mov r15, r8; lea rcx, [rsp+30h]
			// 40 53 55 56 57 41 54 41 56 41 57 48 81 EC ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 84 24 ?? ?? ?? ?? 4C 8B F1 4D 8B F8 48 8D 4C 24 30 48

			// 2026/5/2 @zetsr
			// mov r1x, [mem] | push rbp | push r14 | push r15 | sub rsp, alloc | mov rax, [rip+offset]
			// 4C 8B ? 55 41 56 41 57 48 81 EC ? ? 00 00 48 8B 05

			// 2026/5/11 @zetsr
			// push rbp; push r13; push r15; sub rsp, 0x40; mov rax, [rdx+0x20]; mov r11, [r8]
			// 40 55 41 55 41 57 48 83 EC ? 48 8B ? ? 49 8B

			// 2026/5/20 @zetsr
			// mov r11, rsp | push rbp | push r14 | push r15 | sub rsp, alloc | mov rax, [rip+offset]
			// 4C 8B DC 55 41 56 41 57 48 81 EC 80 08 00 00 48 8B 05 CA 9B D4 08

			// 2026/5/23 @zetsr
			// push rbx | push rbp | push rsi | push rdi | push r12 | push r14 | push r15 | sub rsp, alloc | mov rax, [rip+offset] | xor rax, rsp | mov [rsp+offset], rax | mov r14, rcx | mov r15, r8 | lea rcx, [rsp+offset]
			// 40 53 55 56 57 41 54 41 56 41 57 48 81 EC 60 08 00 00 48 8B 05 27 B8 AC 08 48 33 C4 48 89 84 24 50 08 00 00 4C 8B F1 4D 8B F8 48 8D 4C 24 30

			// 2026/6/6 @zetsr
			// mov r11, rsp | push rbp | push r14 | push r15 | sub rsp, alloc | mov rax
			// 4C 8B DC 55 41 56 41 57 48 81 EC 80 08 00 00 48 8B 05

			// 2026/6/10 @zetsr
			// push rbx | push rbp | push rsi | push rdi | push r12 | push r14 | push r15 | sub rsp, alloc | mov rax, [rip+offset] | xor rax, rsp | mov [rsp+offset], rax | mov r14, rcx
			std::string Tick = "4C 8B DC 55 41 56 41 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 49";
		}

		namespace UGameViewportClient {
			// https://github.com/vsylva/ark-asa-internal
			// VTable = 122，不知道这个虚表会不会过期，可能还是需要做特征码

			// 8B C2 35 DE C5 F3 59 44 8B C0 69 C0 B1 79 37 9E 41 C1 E8 0D 44 33 C0 41 69 C0 6B CA EB 85
			// mov eax, edx | xor eax, imm32 | mov r8d, eax | imul eax, eax, imm32 | shr r8d, imm8 | xor r8d, eax | imul r8d, r8d, imm32
			std::string PostRender = "8B C2 35 ? ? ? ? 44 69";
		}

		namespace UConsole {
			// 2026/4/3 @zetsr
			// 48 8B C4 ? ? 48 83 EC ? 48 89 58 ? 48 8B DA 2026/4/3 过期了

			// mov [rsp + offset], rbx push rbp push rsi push rdi push r12 push r13 push r14 push r15 mov rax, [rcx] sub rsp, offset movsxd rax, [rdx + offset] xor r14d, r14d mov rax, [rcx]
			// 48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8B ? 48 83 EC ? 48 63 ? ? 45 33 F6 48 8B

			// 2026/5/11 @zetsr
			// mov rax, rsp; push r12; sub rsp, 0x60; mov [rax+0x08], rbx; mov rbx, rdx
			// 48 8B C4 41 54 48 83 EC ? 48 89 58 ? 48 8B ?

			// 2026/5/20 @zetsr
			// mov [rsp+8], rbx | push rbp | push rsi | push rdi | push r12 | push r13 | push r14 | push r15 | mov rbp, rsp | sub rsp, alloc | movsxd rsi, [rdx+offset] | xor r14d, r14d
			// 48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 40 48 63 72 08 45 33 F6

			// 2026/5/23 @zetsr
			// mov rax, rsp | push r12 | sub rsp, alloc | mov [rax+8], rbx | mov rbx, rdx
			// 48 8B C4 41 54 48 83 EC 60 48 89 58 08 48 8B DA

			// 2026/6/6 @zetsr
			// mov [rsp+8], rbx | push rbp | push rsi | push rdi | push r12 | push r13 | push r14 | push r15 | mov rbp, rsp | sub rsp, alloc | movsxd rsi, [rdx+offset]
			// 48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 40 48 63 72 08

			// 2026/6/10 @zetsr
			// mov rax, rsp | push r12 | sub rsp, alloc | mov [rax+8], rbx | mov
			std::string OutputTextLine = "48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC ? 48 63 72";
		}
	}
}