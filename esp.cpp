#include "headers/esp.h"
#include "headers/memory.h"
#include "headers/offsets.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdint>


double const PI = 3.14159265358979323846;

float dot(Vector3 a, Vector3 b)
{
	return a.x * b.x +
		a.y * b.y +
		a.z * b.z;
}

void draw_esp(Memory& mem, uint32_t localPlayer)
{
	auto moduleBase = mem.getModuleBaseAddress("ac_client.exe");
	uint32_t entityList, entityCount, game_mode;
	try {
		entityList = mem.Read<uint32_t>(moduleBase + offsets::entity_list);
		entityCount = mem.Read<uint32_t>(moduleBase + offsets::entity_count);
		game_mode = mem.Read<uint32_t>(moduleBase + offsets::game_mode);
	}
	catch (const std::runtime_error& e) {
		return;
	}

	const Vector2 screenSize = GetScreenSize();
	const Vector2 line_start_position(screenSize.x * 0.5f, screenSize.y);

	const std::vector<uint32_t> team_game_modes{7, 20, 21};

	// Iterate through the entity list and draw ESP for each entity
	for (uint32_t i = 1; i <= entityCount; ++i) {
		uint32_t entityAddress, health, team_entity, team_player;
		try {
			entityAddress = mem.Read<uint32_t>(entityList + i * sizeof(uint32_t));
			health = mem.Read<uint32_t>(entityAddress + offsets::health);
			team_entity = mem.Read<uint32_t>(entityAddress + offsets::team);
			team_player = mem.Read<uint32_t>(localPlayer + offsets::team);
		}
		catch (const std::runtime_error& e) {
			continue;
		}
		if (entityAddress == 0 || health <= 0) {
			continue;
		}

		// Skip entities in the same team for team-based game modes
		if (std::find(team_game_modes.begin(), team_game_modes.end(), game_mode) != team_game_modes.end() &&
			team_entity == team_player) {
			continue; // Skip entities in the same team
		}

		Vector2 screenPos = world_to_screen(
			mem,
			moduleBase,
			entityAddress,
			localPlayer,
			static_cast<int>(screenSize.x),
			static_cast<int>(screenSize.y));
		if (screenPos.x < 0 || screenPos.y < 0) {
			continue; // Entity is not on screen
		}
		DrawLine(line_start_position, screenPos, Colors::Green, 1.0f);
	}
}

Vector3 get_entity_pos(Memory& mem, uint32_t entityAddress) {
	try {
		float x = mem.Read<float>(entityAddress + offsets::head_coord_x);
		float y = mem.Read<float>(entityAddress + offsets::head_coord_y);
		float z = mem.Read<float>(entityAddress + offsets::head_coord_z);
		return Vector3(x, y, z);
	}
	catch (const std::runtime_error& e) {
		// Handle the error, e.g., log it or return a default value
		return Vector3(0.0f, 0.0f, 0.0f); // Return a default value
	}
}

Vector2 world_to_screen(Memory& mem, uint32_t moduleBase, uint32_t entityAddress, uint32_t localPlayer, int screenWidth, int screenHeight)
{
	Vector3 worldPos = get_entity_pos(mem, entityAddress);
	if (worldPos.x == 0.0f && worldPos.y == 0.0f && worldPos.z == 0.0f) {
		return Vector2(-1.0f, -1.0f); // Invalid position
	}

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

	auto yaw_rad = cam_yaw * (PI / 180.0f);
	auto pitch_rad = cam_pitch * (PI / 180.0f);

	// Camera direction vectors
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

	// World Space -> View Space
	Vector3 delta = worldPos - Vector3(cam_x, cam_y, cam_z);

	float viewX = dot(delta, camRight);
	float viewY = dot(delta, camUp);
	float viewZ = dot(delta, camForward);

	if (viewZ <= 0.01f)
		return Vector2(-1.0f, -1.0f);

	// View Space -> Clip Space
	auto horizontal_fov_rad = static_cast<float>(fov) * (PI / 180.0f);
	float horizontal_tan = tanf(horizontal_fov_rad / 2.0f);
	if (!std::isfinite(horizontal_tan) || horizontal_tan <= 0.0f) {
		return Vector2(-1.0f, -1.0f);
	}
	float vertical_tan = horizontal_tan / aspectRatio;

	float clipX = viewX / horizontal_tan;
	float clipY = viewY / vertical_tan;
	float clipW = viewZ;

	// Clip Space -> NDC
	float ndcX = clipX / clipW;
	float ndcY = clipY / clipW;

	// Controllo visibilità 2D
	if (ndcX < -1.0f || ndcX > 1.0f ||
		ndcY < -1.0f || ndcY > 1.0f)
		return Vector2(-1.0f, -1.0f);

	// NDC -> Screen Space
	auto x = (ndcX + 1.0f) * 0.5f * screenWidth;
	auto y = (1.0f - ndcY) * 0.5f * screenHeight;

	return Vector2(x, y);

}

