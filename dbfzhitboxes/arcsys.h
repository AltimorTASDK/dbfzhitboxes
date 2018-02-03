#pragma once

struct asw_hithurtbox
{
	int type;
	float x, y, w, h;
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

	int get_pos_x() const;
	int get_pos_y() const;
	int pushbox_width() const;
	int pushbox_top() const;
	int pushbox_bottom() const;
	bool is_active() const;
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