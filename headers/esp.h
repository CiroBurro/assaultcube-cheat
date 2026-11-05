#pragma once

#include "process.h"
#include "Overlay.h"

void draw_ecs(Process& process, uint32_t entityList, uint32_t localPlayer);
Vector3 get_entity_pos(Process& process, uint32_t entityAddress);
Vector2 world_to_screen(Process& process, uint32_t entityAddress, uint32_t localPlayer, int screenWidth, int screenHeight);