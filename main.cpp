#include <iostream>
#include <chrono>
#include <thread>
#include <cstdint>
#include "imgui.h"
#include "headers/imgui_backend.h"
#include "headers/process.h"
#include "headers/offsets.h"
#include "headers/memory.h"
#include "headers/Overlay.h"
#include "headers/esp.h"


bool health_hack = false;
bool armor_hack = false;
bool ammo_hack = false;
bool esp_hack = false;
bool show_menu = false;

void RenderMenu()
{
	if (show_menu) {
		ImGui::Begin("Menu");

		ImGui::Checkbox("Enable infinite health", &health_hack);
		ImGui::Checkbox("Enable infinite armor", &armor_hack);
		ImGui::Checkbox("Enable infinite ammo", &ammo_hack);
		ImGui::Checkbox("Enable ESP", &esp_hack);

		ImGui::End();
	}
}
void godMode(Memory& mem, uint32_t localPlayerAddress) {
	if (localPlayerAddress != 0) {
		uint32_t health = mem.Read<uint32_t>(localPlayerAddress + offsets::health);
		uint32_t armor = mem.Read<uint32_t>(localPlayerAddress + offsets::armor);
		uint32_t mpt_57_ammo = mem.Read<uint32_t>(localPlayerAddress + offsets::mpt_57_ammo);
		uint32_t mk_77_ammo = mem.Read<uint32_t>(localPlayerAddress + offsets::mk_77_ammo);

		if (health_hack && health < 1337) {
			mem.Write<uint32_t>(localPlayerAddress + offsets::health, 1337);
		}
		if (armor_hack && armor < 1337) {
			mem.Write<uint32_t>(localPlayerAddress + offsets::armor, 1337);
		}
		if (ammo_hack && (mpt_57_ammo < 100 || mk_77_ammo < 100)) {
			mem.Write<uint32_t>(localPlayerAddress + offsets::mpt_57_ammo, 9999);
			mem.Write<uint32_t>(localPlayerAddress + offsets::mk_77_ammo, 9999);
		}
	}
}
void esp(Memory& mem, uint32_t localPlayerAddress) {
	if (esp_hack && localPlayerAddress != 0) {
		draw_esp(mem, localPlayerAddress);
	}
}

int main()
{

	// Initialize ImGui backend
	ImGuiBackend backend(1920, 1080, L"AssaultCube Cheat");

	//Attach to the AssaultCube process and get the necessary addresses
	Process process("ac_client.exe");
	auto moduleBase = process.getModuleBaseAddress("ac_client.exe");
	Memory mem(process);
	uint32_t localPlayerAddress = mem.Read<uint32_t>(moduleBase + offsets::local_player);
	uint32_t entityListAddress = mem.Read<uint32_t>(moduleBase + offsets::entity_list);
	
	// Main loop
	while (backend.ProcessMessages() && process) {
		backend.NewFrame();
		RenderMenu();

		// Hack!
		esp(mem, localPlayerAddress);
		godMode(mem, localPlayerAddress);

		backend.Render();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return 0;
}

