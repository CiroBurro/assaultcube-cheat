#include "headers/esp.h"
#include "headers/memory.h"
#include "headers/offsets.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <format>
#include <string>


double const PI = 3.14159265358979323846;

// Computes the dot product of two 3D vectors.
float dot(Vector3 a, Vector3 b)
{
	return a.x * b.x +
		a.y * b.y +
		a.z * b.z;
}

void draw_esp(Memory& mem, uint32_t localPlayer)
{
	// Retrive necessary data from memory
	auto moduleBase = mem.getModuleBaseAddress("ac_client.exe");
	uint32_t entityList, entityCount, game_mode;
	try {
		entityList = mem.Read<uint32_t>(moduleBase + offsets::entity_list);
		entityCount = mem.Read<uint32_t>(moduleBase + offsets::entity_count);
		game_mode = mem.Read<uint32_t>(moduleBase + offsets::game_mode);
	}
	catch (const std::runtime_error& e) {
		return; // If any read operation fails, simply return without drawing ESP
	}

	const Vector2 screenSize = GetScreenSize();
	const Vector2 line_start_position(screenSize.x * 0.5f, screenSize.y);

	/* List of game modes that are team-based.
	* 7: team deathmatch
	* 20: team survivor
	* 21: team one shot one kill
	*/
	const std::vector<uint32_t> team_game_modes{7, 20, 21};

	// Iterate through the entity list and draw ESP for each entity
	for (uint32_t i = 1; i <= entityCount; ++i) {
		uint32_t entityAddress, health, armor, team_entity, team_player;
		try {
			entityAddress = mem.Read<uint32_t>(entityList + i * sizeof(uint32_t));
			health = mem.Read<uint32_t>(entityAddress + offsets::health);
			armor = mem.Read<uint32_t>(entityAddress + offsets::armor);
			team_entity = mem.Read<uint32_t>(entityAddress + offsets::team);
			team_player = mem.Read<uint32_t>(localPlayer + offsets::team);
		}
		catch (const std::runtime_error& e) {
			continue; // If any read operation fails for this entity, skip to the next one
		}
		if (entityAddress == 0 || health <= 0) {
			continue; // Skip invalid or dead entities
		}

		if (std::find(team_game_modes.begin(), team_game_modes.end(), game_mode) != team_game_modes.end() &&
			team_entity == team_player) {
			continue; // Skip entities in the same team
		}

		Vector3 worldPos_head = get_entity_head_pos(mem, entityAddress);
		Vector3 worldPos_feet = get_entity_feet_pos(mem, entityAddress);
		if ((worldPos_head.x == 0.0f && worldPos_head.y == 0.0f && worldPos_head.z == 0.0f) ||
			(worldPos_feet.x == 0.0f && worldPos_feet.y == 0.0f && worldPos_feet.z == 0.0f)) {
			continue;
		}

		// Retrive the screen position of the entity
		Vector2 screenPos_head = world_to_screen(
			mem,
			moduleBase,
			worldPos_head,
			localPlayer,
			static_cast<int>(screenSize.x),
			static_cast<int>(screenSize.y));

		Vector2 screenPos_feet = world_to_screen(
			mem,
			moduleBase,
			worldPos_feet,
			localPlayer,
			static_cast<int>(screenSize.x),
			static_cast<int>(screenSize.y));

		if (screenPos_head.x < 0 || screenPos_head.y < 0 || screenPos_feet.x < 0 || screenPos_feet.y < 0) {
			continue; // Entity is not on screen
		}

		DrawLine(line_start_position, screenPos_feet, Colors::Green, 1.0f);
		Vector2 corner(screenPos_head.x - 20.0f, screenPos_head.y - 30.0f);
		DrawBox(corner, 40.0f, screenPos_feet.y - screenPos_head.y + 30.0f, Colors::Green, 1.0f);
		std::string text = std::format("HP: {}\nArmor: {}", health, armor);
		DrawString(corner - Vector2(0.0f, 30.0f), text.c_str(), Colors::Green);
	}
}

Vector3 get_entity_head_pos(Memory& mem, uint32_t entityAddress) noexcept {
	// Attempt to read the entity's head position from memory
	try {
		float x = mem.Read<float>(entityAddress + offsets::head_coord_x);
		float y = mem.Read<float>(entityAddress + offsets::head_coord_y);
		float z = mem.Read<float>(entityAddress + offsets::head_coord_z);
		return Vector3(x, y, z);
	}
	catch (const std::runtime_error& e) {
		return Vector3(0.0f, 0.0f, 0.0f); // Return a default value
	}
}

Vector3 get_entity_feet_pos(Memory& mem, uint32_t entityAddress) noexcept {
	// Attempt to read the entity's position from memory
	try {
		float x = mem.Read<float>(entityAddress + offsets::coord_x);
		float y = mem.Read<float>(entityAddress + offsets::coord_y);
		float z = mem.Read<float>(entityAddress + offsets::coord_z);
		return Vector3(x, y, z);
	}
	catch (const std::runtime_error& e) {
		return Vector3(0.0f, 0.0f, 0.0f); // Return a default value
	}
}

Vector2 world_to_screen(Memory& mem, uint32_t moduleBase, Vector3 worldPos, uint32_t localPlayer, int screenWidth, int screenHeight) noexcept
{
	/* World to screen conversion:
	* 1. World Space (3d coordinates) -> View Space (3D coordinates in the camera's reference frame)
	* 2. View Space -> Clip Space (Prospettic projection of the 3D coordinates on the screen)
	* 3. Clip Space -> NDC (Normalized Device Coordinates)
	* 4. NDC -> Screen Space (2D coordinates on the screen)
	*
	* I don't have the view and projection matrices to compute the transformation.
	* I'll manually do the conversion using the camera's orientation, position and fov.
	*/

	/* Get the world position of the entity
	Vector3 worldPos = get_entity_head_pos(mem, entityAddress);
	if (worldPos.x == 0.0f && worldPos.y == 0.0f && worldPos.z == 0.0f) {
		return Vector2(-1.0f, -1.0f); // Invalid position
	}*/

	// Retrive camera parameters to perform the transformation from memory
	float cam_x, cam_y, cam_z, cam_yaw, cam_pitch, fov;
	try {
		cam_x = mem.Read<float>(localPlayer + offsets::head_coord_x);
		cam_y = mem.Read<float>(localPlayer + offsets::head_coord_y);
		cam_z = mem.Read<float>(localPlayer + offsets::head_coord_z);
		cam_yaw = mem.Read<float>(localPlayer + offsets::cam_yaw);
		cam_pitch = mem.Read<float>(localPlayer + offsets::cam_pitch);
		fov = mem.Read<float>(moduleBase + offsets::fov);
	}
	catch (const std::runtime_error& e) {
		return Vector2(-1.0f, -1.0f); // Failed to read camera data
	}


	float aspectRatio = (float)screenWidth / screenHeight;

	// Degrees to radians conversion
	auto yaw_rad = cam_yaw * (PI / 180.0f);
	auto pitch_rad = cam_pitch * (PI / 180.0f);

	/* Camera direction vectors
	* Trigonomtric functions are used to compute the camera's forward, right and up vectors
	* based on its yaw and pitch angles.
	*/
	Vector3 camForward(
		cosf(pitch_rad) * sinf(yaw_rad),
		cosf(pitch_rad) * (-cosf(yaw_rad)),
		sinf(pitch_rad)
	);

	Vector3 camRight(
		cosf(yaw_rad),
		sinf(yaw_rad),
		0.0f
	);

	Vector3 camUp = Vector3(
		-sinf(pitch_rad) * sinf(yaw_rad),
		sinf(pitch_rad) * cosf(yaw_rad),
		cosf(pitch_rad)
	);

	// 1. World Space->View Space
	Vector3 delta = worldPos - Vector3(cam_x, cam_y, cam_z); // Distance from camera to entity 

	/* Entity coordinates relative to camera orientation
	* X: The right direction of the camera. Positive X is to the right of the camera. (Horizontal axis of the screen)
	* Y: The up direction of the camera. Positive Y is above the camera. (Vertical axis of the screen)
	* Z: The forward direction of the camera. Positive Z is in the direction the camera is facing. (Depth axis in the game)
	*/ 
	float viewX = dot(delta, camRight); 
	float viewY = dot(delta, camUp);
	float viewZ = dot(delta, camForward);

	if (viewZ <= 0.01f)
		return Vector2(-1.0f, -1.0f); // Entity is behind the camera, so just return an invalid position

	// 2. View Space -> Clip Space
	auto horizontal_fov_rad = static_cast<float>(fov) * (PI / 180.0f);
	float horizontal_fov_tan = tanf(horizontal_fov_rad / 2.0f);
	if (!std::isfinite(horizontal_fov_tan) || horizontal_fov_tan <= 0.0f) {
		return Vector2(-1.0f, -1.0f); // Invalid FOV, return an invalid position
	}
	float vertical_fov_tan = horizontal_fov_tan / aspectRatio;

	/* Apply the perspective projection to X and Y coordinates.
	* X is divided by tan(fov_h/2), this tells us how much to scale the X coordinate based on the horizontal FOV.
	* Y is divided by tan(fov_v/2), this tells us how much to scale the Y coordinate based on the vertical FOV.
	* 
	* The screen has no depth, so we don't need to apply any transformation to the Z coordinate,
	* but we still need to keep it for the perspective division later:
	* the more distant the object, the smaller it will appear.
	*/
	float clipX = viewX / horizontal_fov_tan;
	float clipY = viewY / vertical_fov_tan;
	float clipW = viewZ;

	// 3. Clip Space -> NDC
	float ndcX = clipX / clipW; // perspective division, divide by W to get normalized device coordinates
	float ndcY = clipY / clipW;

	if (ndcX < -1.0f || ndcX > 1.0f ||
		ndcY < -1.0f || ndcY > 1.0f)
		return Vector2(-1.0f, -1.0f); // Entity is outside the screen bounds, again return an invalid position

	// 4. NDC -> Screen Space
	auto x = (ndcX + 1.0f) * 0.5f * screenWidth;
	auto y = (1.0f - ndcY) * 0.5f * screenHeight;

	return Vector2(x, y);

}

