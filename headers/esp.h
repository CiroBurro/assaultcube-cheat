#pragma once

#include <cstdint>
#include "memory.h"
#include "process.h"
#include "Overlay.h"

void draw_esp(Memory& mem, std::uint32_t localPlayer);
Vector3 get_entity_pos(Memory& mem, std::uint32_t entityAddress);
Vector2 world_to_screen(Memory& mem, std::uint32_t moduleBase, std::uint32_t entityAddress, std::uint32_t localPlayer, int screenWidth, int screenHeight);
