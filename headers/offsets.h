#pragma once

namespace offsets {
	// Entities
	const auto local_player = 0x18AC00;
	const auto entity_list = 0x18AC04;

	// Entity's fields
	const auto health = 0xEC;
	const auto armor = 0xF0;

	const auto coord_x = 0x28;
	const auto coord_y = 0x2C;
	const auto coord_z = 0x30;

	const auto head_coord_x = 0x4;
	const auto head_coord_y = 0x8;
	const auto head_coord_z = 0xC;

	const auto cam_yaw = 0x34;
	const auto cam_pitch = 0x38;

	const auto mpt_57_ammo = 0x140;
	const auto mk_77_ammo = 0x12C;

	const auto mpt_57_magazine = 0x11C;
	const auto mk_77_magazine = 0x108;

	const auto team = 0x30c;

	const auto fov_read_only = 0x334;
	const auto scope_fov_read_only = 0x338;

	// Other
	const auto game_mode = 0x18ABF8;
	const auto entity_count = 0x191FD4;
	const auto fov = 0x18A7CC;
}	const auto scope_fov = 0x18A834;