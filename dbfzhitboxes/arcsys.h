#pragma once

#include <vector>

struct asw_hithurtbox
{
	int type;
	float x, y, w, h;
};

namespace asw_control_flags
{
	constexpr auto can_crouch = 2;
	constexpr auto can_walk_f = 4;
	constexpr auto can_dash_f = 8;
	constexpr auto can_walk_b = 0x20;
	constexpr auto can_dash_b = 0x40;
	constexpr auto can_jump = 0x100;
	constexpr auto can_attack = 0x1000;
	constexpr auto can_special_only = 0x2000;
	constexpr auto can_block = 0x10000;
};

class asw_entity
{
public:
	int angle() const
	{
		return *(int*)((char*)(this) + 0x3AC);
	}

	float angle_rads() const
	{
		return (float)(angle()) / 1000.F * 3.1415926F / 180.F;
	}

	int scale_x() const
	{
		return *(int*)((char*)(this) + 0x3B8);
	}

	int scale_y() const
	{
		return *(int*)((char*)(this) + 0x3BC);
	}

	int direction() const
	{
		return *(int*)((char*)(this) + 0x39C);
	}

	bool is_counterhit() const
	{
		return (*(int*)((char*)(this) + 0x388) & 256) != 0;
	}

	bool is_invuln() const
	{
		return (*(int*)((char*)(this) + 0xD28) & 2) != 0;
	}

	int hithurtbox_count() const
	{
		return *(int*)((char*)(this) + 0x104) + *(int*)((char*)(this) + 0x108);
	}

	asw_hithurtbox *hithurtbox_data() const
	{
		return *(asw_hithurtbox**)((char*)(this) + 0x78);
	}

	asw_entity *owner() const
	{
		return *(asw_entity**)((char*)(this) + 0x258);
	}

	asw_entity *attached() const
	{
		return *(asw_entity**)((char*)(this) + 0x2C0);
	}

	int pushbox_front_offset() const
	{
		return *(int*)((char*)(this) + 0x524);
	}

	int hitstun() const
	{
		return *(int*)((char*)(this) + 0x93FC);
	}

	int blockstun() const
	{
		return *(int*)((char*)(this) + 0x3730);
	}

	int hitstop() const
	{
		return *(int*)((char*)(this) + 0x230);
	}

	int control_flags() const
	{
		return *(int*)((char*)(this) + 0x3710);
	}

	int cmd_frames_elapsed() const
	{
		return *(int*)((char*)(this) + 0xE88);
	}

	int cmd_frames_total() const
	{
		return *(int*)((char*)(this) + 0xE90);
	}

	char *next_script_cmd() const
	{
		return *(char**)((char*)(this) + 0xE58);
	}

	char *first_script_cmd() const
	{
		return *(char**)((char*)(this) + 0xE60);
	}

	int cmn_state_id() const
	{
		return *(int*)((char*)(this) + 0x9E18);
	}

	int airdashf_frames() const
	{
		return *(int*)((char*)(this) + 0x948C);
	}

	int airdashb_frames() const
	{
		return *(int*)((char*)(this) + 0x9490);
	}

	int state_frames_elapsed() const
	{
		return *(int*)((char*)(this) + 0x1B4);
	}

	int vel_x() const
	{
		return *(int*)((char*)(this) + 0x4F4);
	}

	int vel_y() const
	{
		return *(int*)((char*)(this) + 0x4F8);
	}

	int stored_vel_x() const
	{
		return *(int*)((char*)(this) + 0x3F0);
	}

	int stored_vel_y() const
	{
		return *(int*)((char*)(this) + 0x3F4);
	}

	int gravity() const
	{
		return *(int*)((char*)(this) + 0x4FC);
	}

	int stored_gravity() const
	{
		return *(int*)((char*)(this) + 0x3FC);
	}

	int stiff_landing_frames() const
	{
		return *(int*)((char*)(this) + 0x9530);
	}

	int next_stiff_landing_frames() const
	{
		return *(int*)((char*)(this) + 0x952C);
	}

	int prev_pos_y() const
	{
		return *(int*)((char*)(this) + 0x4D8);
	}

	int initial_proration() const
	{
		return *(int*)((char*)(this) + 0x35B0);
	}

	int scaling_hits() const
	{
		return *(int*)((char*)(this) + 0x24634);
	}

	int combo_timer() const
	{
		return *(int*)((char*)(this) + 0x9D58);
	}

	int hitstun_penalty_start() const
	{
		return *(int*)((char*)(this) + 0x9578);
	}

	int untechable_penalty_start() const
	{
		return *(int*)((char*)(this) + 0x957C);
	}

	int get_pos_x() const;
	int get_pos_y() const;
	int pushbox_width() const;
	int pushbox_top() const;
	int pushbox_bottom() const;
	bool is_active() const;
	int damage_scaling() const;
	int hitstun_penalty() const;
	int untechable_penalty() const;

private:
	int script_frames_left(const char *script_start, const char *script) const;
	int landing_recovery(const char *script_start) const;
	int check_landing(const int recovery) const;
	int check_landing_script(const char *script_start, const char *script, const int recovery) const;

public:
	// Frames until actionable
	int recovery_frames() const;
};

class asw_engine
{
public:
	static constexpr float coord_scale = .43F;

	int entity_count() const
	{
		return *(int*)((char*)(this) + 0x590);
	}

	asw_entity **entity_list() const
	{
		return (asw_entity**)((char*)(this) + 0x1260);
	}

	static asw_engine *get();
};