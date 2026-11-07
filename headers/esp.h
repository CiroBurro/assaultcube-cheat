#pragma once

#include <cstdint>
#include "memory.h"
#include "process.h"
#include "headers/utils.h"
/* @brief Draws ESP (Extra Sensory Perception) for entities in the game.
* 
* @param mem Reference to the Memory object for reading game memory.
* @param localPlayer The address of the local player in memory.
* 
* @throws std::runtime_error If reading memory fails.
*/
void draw_esp(Memory& mem, std::uint32_t localPlayer);

/*
* @brief Gets the position of an entity in the game world.
* 
* @param mem Reference to the Memory object for reading game memory.
* @param entityAddress The address of the entity in memory.
* 
* @return The position of the entity in the game world.
*/
Vector3 get_entity_pos(Memory& mem, std::uint32_t entityAddress) noexcept;

/* @brief Converts world coordinates to screen coordinates for rendering.
* 
* @param mem Reference to the Memory object for reading game memory.
* @param moduleBase The base address of the game module in memory.
* @param entityAddress The address of the entity in memory.
* @param localPlayer The address of the local player in memory.
* @param screenWidth The width of the game screen.
* @param screenHeight The height of the game screen.
* 
* @return The screen coordinates corresponding to the world coordinates of the entity.
*/
Vector2 world_to_screen(Memory& mem, std::uint32_t moduleBase, std::uint32_t entityAddress, std::uint32_t localPlayer, int screenWidth, int screenHeight) noexcept;
