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


void RenderMenu(Process& process, uint32_t entityListAddress, uint32_t localPlayerAddress)
{
	if (show_menu) {
		ImGui::Begin("Menu");

		ImGui::Checkbox("Enable infinite health", &health_hack);
		ImGui::Checkbox("Enable infinite armor", &armor_hack);
		ImGui::Checkbox("Enable infinite ammo", &ammo_hack);
		ImGui::Checkbox("Enable ESP", &esp_hack);

		ImGui::End();
	} 
	if (esp_hack) {
		draw_ecs(process, entityListAddress, localPlayerAddress);
	}
}

int main()
{
	// Initialize ImGui backend
	ImGuiBackend backend(1920, 1080, L"AssaultCube Cheat");

	// Attach to the AssaultCube process and get the necessary addresses
	Process process("ac_client.exe");
	auto moduleBase = process.getModuleBaseAddress("ac_client.exe");
	uint32_t localPlayerAddress{};
	uint32_t entityListAddress{};

	if (!mem::read(process, moduleBase + offsets::local_player, localPlayerAddress) ||
		!mem::read(process, moduleBase + offsets::entity_list, entityListAddress)) {
		std::cerr << "Failed to read game addresses" << std::endl;
		return 1;
	}

	/* debug prints
	std::cout << "PID: " << process.getPid() << std::endl;
	std::cout << "Module Base: " << std::hex << moduleBase << std::endl;
	std::cout << "Local Player: " << std::hex << localPlayerAddress << std::endl;
	std::cout << "Entity List: " << std::hex << entityListAddress << std::endl;
	*/

	std::uint32_t health{}, armor{}, mpt_57_ammo{}, mk_77_ammo{};

	// Main loop
	while (backend.ProcessMessages() && process) {
		backend.NewFrame();
		RenderMenu(process, entityListAddress, localPlayerAddress);

		// Cheat logic: read and modify player values based on the enabled hacks
		if (mem::read(process, moduleBase + offsets::local_player, localPlayerAddress) && localPlayerAddress != 0) {
			const bool valuesRead =
				mem::read(process, localPlayerAddress + offsets::health, health) &&
				mem::read(process, localPlayerAddress + offsets::armor, armor) &&
				mem::read(process, localPlayerAddress + offsets::mpt_57_ammo, mpt_57_ammo) &&
				mem::read(process, localPlayerAddress + offsets::mk_77_ammo, mk_77_ammo);

			if (valuesRead) {
				if (health_hack && health < 100) {
					mem::write(process, localPlayerAddress + offsets::health, std::uint32_t{ 1337 });
				}
				if (armor_hack && armor < 100) {
					mem::write(process, localPlayerAddress + offsets::armor, std::uint32_t{ 1337 });
				}
				if (ammo_hack && (mpt_57_ammo < 100 || mk_77_ammo < 100)) {
					mem::write(process, localPlayerAddress + offsets::mpt_57_ammo, std::uint32_t{ 9999 });
					mem::write(process, localPlayerAddress + offsets::mk_77_ammo, std::uint32_t{ 9999 });
				}
			}
		}

		backend.Render();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return 0;
}

