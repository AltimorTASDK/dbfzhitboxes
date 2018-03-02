#include "arcsys.h"
#include "sigscan.h"
#include <Windows.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <map>
#include <functional>

int asw_entity::get_pos_x() const
{
	using func_t = int(*)(const asw_entity*);
	static sigscan sig("RED-Win64-Shipping.exe");
	static const auto func = (func_t)(sig.sig("\x03\x83\xA0\x03\x00\x00\x48\x83\xC4\x20", "xxxxxxxxxx") - 0x3F);
	return func(this);
}

int asw_entity::get_pos_y() const
{
	using func_t = int(*)(const asw_entity*);
	static sigscan sig("RED-Win64-Shipping.exe");
	static const auto func = (func_t)(sig.sig("\x03\x83\xA4\x03\x00\x00\x48\x83\xC4\x20", "xxxxxxxxxx") - 0x26);
	return func(this);
}

int asw_entity::pushbox_width() const
{
	using func_t = int(*)(const asw_entity*);
	static sigscan sig("RED-Win64-Shipping.exe");
	static const auto func = (func_t)(sig.sig("\x8B\x81\x18\x05\x00\x00\x48\x8B\xD9", "xxxxxxxxx") - 6);
	return func(this);
}

int asw_entity::pushbox_top() const
{
	using func_t = int(*)(const asw_entity*);
	static sigscan sig("RED-Win64-Shipping.exe");
	static const auto func = (func_t)(sig.sig("\x8B\x81\x1C\x05\x00\x00\x48\x8B\xD9", "xxxxxxxxx") - 6);
	return func(this);
}

int asw_entity::pushbox_bottom() const
{
	using func_t = int(*)(const asw_entity*);
	static sigscan sig("RED-Win64-Shipping.exe");
	static const auto func = (func_t)(sig.sig("\x8B\x81\x20\x05\x00\x00\x48\x8B\xD9", "xxxxxxxxx") - 6);
	return func(this);
}

bool asw_entity::is_active() const
{
	using func_t = bool(*)(const asw_entity*, int);
	static sigscan sig("RED-Win64-Shipping.exe");
	static const auto func = (func_t)(sig.sig("\xC1\xE8\x08\xA8\x01\x74\x3B", "xxxxxxx") - 0x6C);

	// Still show as active after a hit occurs
	const auto hit = *(int*)((char*)(this) + 0x728);
	*(int*)((char*)(this) + 0x728) = 0;
	const auto result = func(this, 0);
	*(int*)((char*)(this) + 0x728) = hit;

	return result;
}

int asw_entity::damage_scaling() const
{
	const int table[] =
	{
		0, 10, 20, 30, 40, 50, 60, 70, 70, 70, 70, 75, 75, 75, 80, 80, 80, 85, 85, 85, 85
	};

	const auto hit_count = abs(scaling_hits() / 500);
	return min(max(initial_proration() - table[min(hit_count, 20)], 10), 100);
}

int asw_entity::hitstun_penalty() const
{
	return
		combo_timer() < hitstun_penalty_start() ? 0 :
		combo_timer() < hitstun_penalty_start() + 120 ? -5 :
		combo_timer() < hitstun_penalty_start() + 240 ? -10 :
		combo_timer() < hitstun_penalty_start() + 360 ? -15 :
		-10000;
}

int asw_entity::untechable_penalty() const
{
	return
		combo_timer() < untechable_penalty_start() ? 0 :
		combo_timer() < untechable_penalty_start() + 120 ? -5 :
		combo_timer() < untechable_penalty_start() + 240 ? -10 :
		combo_timer() < untechable_penalty_start() + 360 ? -15 :
		-10000;
}

static short *cmd_sizes()
{
	static sigscan sig("RED-Win64-Shipping.exe");
	static const auto cmd_sizes = (short*)(sig.sig("\x24\x00\x04\x00\x28\x00", "xxxxxx"));

	return cmd_sizes;
}

class asw_script
{
	const asw_entity *entity;
	const char *script_start, *script;
	std::vector<const char*> returns, jumps;

	using handler_t = bool(const asw_entity*, const char*);
	std::map<int, std::function<handler_t>> handlers;


public:
	asw_script() = delete;
	asw_script(const asw_entity *entity, const char *script_start, const char *script) :
		entity(entity), script_start(script_start) , script(script) {}

	const char *get_subroutine(const char *name) const;
	const char *get_label(const char *name) const;
	void set_handler(int id, const std::function<handler_t> &handler);
	bool run();
};

const char *asw_script::get_label(const char *name) const
{
	const auto *s = script_start;

	while (true)
	{
		const auto id = *(int*)(s);
		if (id == 0xB)
		{
			if (strcmp(s + 4, name) == 0)
				return s;
		}
		else if (id == 1)
		{
			return nullptr;
		}

		s += cmd_sizes()[id];
	}
}

const char *asw_script::get_subroutine(const char *name) const
{
	using find_subroutine_t = const char*(*)(void*, const char*);
	static sigscan sig("RED-Win64-Shipping.exe");
	static const auto find_subroutine = (find_subroutine_t)(sig.sig("\x49\x8B\x45\x28\x8B\x70\x10", "xxxxxxx") - 0x2D);

	// From sub_1402B51C0
	if (memcmp(name, "cmn", 3) == 0)
		return find_subroutine((char*)(asw_engine::get()) + 0x560, name);

	const auto result = find_subroutine(*(char**)((char*)(entity) + 0xE48), name);
	if (result != nullptr)
		return result;

	if (!*(bool*)((char*)(this) + 0x18))
		return nullptr;

	const auto unknown1 = *(int*)((char*)(entity) + 0x54);
	const auto unknown2 = *(int*)((char*)(entity) + 0x44);
	return find_subroutine((char*)(asw_engine::get()) + 0x320 + 0x60 * (unknown1 + 3 * unknown2), name);
}

void asw_script::set_handler(int id, const std::function<handler_t> &handler)
{
	handlers[id] = handler;
}

bool asw_script::run()
{
	const auto id = *(int*)(script);

	// Run custom handlers
	const auto handler = handlers.find(id);
	if (handler != handlers.end())
		if (!handler->second(entity, script))
			return false;

	if (id == 0xC)
	{
		// Unconditional jump
		const auto *label = get_label(script + 4);
		if (label != nullptr)
		{
			// Infinite loop protection
			if (std::find(jumps.begin(), jumps.end(), label) != jumps.end())
				return false;

			script = label;
			jumps.push_back(script);
			return true;
		}
	}
	else if (id == 0x18)
	{
		// Conditional jump
		const auto condition = *(int64_t*)(script + 0x24);

		using check_condition_t = int(*)(const asw_entity*, int64_t);
		static sigscan sig("RED-Win64-Shipping.exe");
		static const auto check_condition = (check_condition_t)(sig.sig("\x48\x8B\xF9\x85\xD2\x75\x0C", "xxxxxxx") - 0xA);

		if (check_condition(entity, condition))
		{
			const auto *label = get_label(script + 4);
			if (label != nullptr)
			{
				// Infinite loop protection
				if (std::find(jumps.begin(), jumps.end(), label) != jumps.end())
					return false;

				script = label;
				jumps.push_back(script);
				return true;
			}
		}

	}
	else if (id == 0x11)
	{
		// Execute subroutine
		const auto *subroutine = get_subroutine(script + 4);
		if (subroutine)
		{
			returns.push_back(script + cmd_sizes()[id]);
			script = subroutine;
			return true;
		}
	}
	else if (id == 0x10)
	{
		// Return from subroutine
		script = returns.back();
		returns.pop_back();
		return true;
	}
	else if (id == 1 || id == 0x12)
	{
		return false;
	}

	script += cmd_sizes()[id];
	return true;
}

int asw_entity::script_frames_left(const char *script_start, const char *script) const
{
	auto frames = 0;
	auto cmd_frames = 0;

	asw_script s(this, script_start, script);
	s.set_handler(2, [&](const asw_entity *entity, const char *script) -> bool
	{
		// Set sprite
		if (cmd_frames == 0x7FFFFFFF)
		{
			// This means the script is waiting on the engine to jump to a label
			frames = -1;
			cmd_frames = 0;
			return false;
		}

		frames += cmd_frames;
		cmd_frames = *(int*)(script + 0x24);

		return true;
	});
	s.set_handler(0x543, [&](const asw_entity *entity, const char *script) -> bool
	{
		// Ability to attack regained
		if (*(char*)(script + 4))
		{
			cmd_frames = 0;
			return false;
		}

		return true;
	});

	while (s.run());

	return frames + cmd_frames;
}

int asw_entity::landing_recovery(const char *script_start) const
{
	auto landing_frames = max(1, next_stiff_landing_frames());
	const char *handler = nullptr;

	asw_script s(this, script_start, script_start);
	s.set_handler(0x23, [&](const asw_entity *entity, const char *script) -> bool
	{
		// Sets an event handler
		const auto type = *(int*)(script + 4);
		if (type == 2)
		{
			const auto label = s.get_label(script + 8);
			if (label != nullptr)
				handler = label;
		}

		return true;
	});
	s.set_handler(0x720, [&](const asw_entity *entity, const char *script) -> bool
	{
		// Set stiff landing frames
		landing_frames = *(int*)(script + 4);

		return true;
	});

	while (s.run());

	if (handler != nullptr)
		return script_frames_left(script_start, handler);

	return landing_frames;
}

int asw_entity::check_landing(const int recovery) const
{
	if (get_pos_y() <= 0 && prev_pos_y() <= 0)
		return recovery;

	// You stay at a y coordinate of 0 on the last frame, so + 1
	auto frames_to_land = 1;
	auto pos = get_pos_y();
	auto vel = vel_y();

	while (pos > 0)
	{
		pos += vel;
		vel -= gravity();
		frames_to_land++;

		if (vel >= 0 && gravity() <= 0)
			return recovery;
	}

	if (recovery != -1 && frames_to_land >= recovery)
		return recovery;

	const auto landing_frames = landing_recovery(first_script_cmd());
	if (landing_frames == -1)
		return -1;

	return frames_to_land + landing_frames;
}

int asw_entity::check_landing_script(const char *script_start, const char *script, const int recovery) const
{
	// + 1 because an extra frame is spent at y pos 0
	auto frames_to_land = 1;
	auto pos = get_pos_y();
	auto vel = vel_y();
	auto grav = gravity();
	auto stored_vel = stored_vel_y();
	auto stored_grav = stored_gravity();
	auto cmd_frames = 0;
	auto in_air = get_pos_y() > 0 || prev_pos_y() > 0;
	auto first_cmd = true;

	const auto simulate = [&](bool script_over) -> bool
	{
		for (auto i = 0; i < (script_over ? 1 : cmd_frames); i++)
		{
			if (pos <= 0 && in_air)
				return false;

			if (recovery != -1 && frames_to_land > recovery)
				return false;

			if ((script_over || cmd_frames == 0x7FFFFFFF) && ((vel >= 0 && grav >= 0) || !in_air))
			{
				frames_to_land = -1;
				return false;
			}

			pos += vel;
			vel -= grav;
			frames_to_land++;
			in_air = in_air || pos > 0;
		}

		return true;
	};

	asw_script s(this, script_start, script);
	s.set_handler(0x70, [&](const asw_entity *entity, const char *script) -> bool
	{
		vel = *(int*)(script + 4);
		return true;
	});
	s.set_handler(0x71, [&](const asw_entity *entity, const char *script) -> bool
	{
		vel += *(int*)(script + 4);
		return true;
	});
	s.set_handler(0x72, [&](const asw_entity *entity, const char *script) -> bool
	{
		stored_vel = vel;
		return true;
	});
	s.set_handler(0x73, [&](const asw_entity *entity, const char *script) -> bool
	{
		vel = stored_vel;
		return true;
	});
	s.set_handler(0x74, [&](const asw_entity *entity, const char *script) -> bool
	{
		vel = (vel * *(int*)(script + 4)) / 100;
		return true;
	});
	s.set_handler(0x7F, [&](const asw_entity *entity, const char *script) -> bool
	{
		grav = *(int*)(script + 4);
		return true;
	});
	s.set_handler(0x80, [&](const asw_entity *entity, const char *script) -> bool
	{
		grav += *(int*)(script + 4);
		return true;
	});
	s.set_handler(0x81, [&](const asw_entity *entity, const char *script) -> bool
	{
		stored_grav = grav;
		return true;
	});
	s.set_handler(0x82, [&](const asw_entity *entity, const char *script) -> bool
	{
		grav = stored_grav;
		return true;
	});
	s.set_handler(0x83, [&](const asw_entity *entity, const char *script) -> bool
	{
		grav = (grav * *(int*)(script + 4)) / 100;
		return true;
	});
	s.set_handler(2, [&](const asw_entity *entity, const char *script) -> bool
	{
		if (!simulate(false))
			return false;

		cmd_frames = *(int*)(script + 0x24);

		if (first_cmd && cmd_frames != 0x7FFFFFFF)
		{
			cmd_frames -= cmd_frames_elapsed();
			first_cmd = false;
		}

		return true;
	});

	while (s.run());
	while (simulate(true));

	if (!in_air || (recovery != -1 && frames_to_land > recovery) || (vel > 0 && grav > 0) || frames_to_land == -1)
		return recovery;

	const auto landing_frames = landing_recovery(first_script_cmd());
	if (landing_frames == -1)
		return -1;

	return frames_to_land + landing_frames;
}

int asw_entity::recovery_frames() const
{
	if (control_flags() & asw_control_flags::can_attack)
		return 0;
	else if (blockstun() > 0)
		return blockstun();
	else if (hitstun() > 0)
		return check_landing(hitstun());
	else if (cmn_state_id() == 8)
		return state_frames_elapsed() > 0 ? 1 : 0;
	else if (cmn_state_id() == 9)
		return stiff_landing_frames() - state_frames_elapsed() + 1;
	else if (cmn_state_id() == 0x10)
		return airdashf_frames() - state_frames_elapsed() + 2;
	else if (cmn_state_id() == 0x11)
		return airdashb_frames() - state_frames_elapsed() + 2;
	else if (cmn_state_id() == 0x87 && hitstop() > 0)
		return 6; // Superdash
	else if (cmn_state_id() == 0x87)
		return -1;
	else if (first_script_cmd() == nullptr || next_script_cmd() == nullptr)
		return 0;

	const char *current_cmd = first_script_cmd();
	const auto *next_cmd = first_script_cmd();
	do
	{
		if (*(int*)(next_cmd) == 2)
			current_cmd = next_cmd;

		next_cmd += cmd_sizes()[*(int*)(next_cmd)];
	} while (next_cmd != next_script_cmd());

	const auto frames_after_current = script_frames_left(first_script_cmd(), current_cmd);
	if (frames_after_current == -1 || cmd_frames_total() == 0x7FFFFFFF)
		return check_landing_script(first_script_cmd(), current_cmd, -1);

	return check_landing_script(first_script_cmd(), next_cmd, frames_after_current - cmd_frames_elapsed());
}

asw_engine *asw_engine::get()
{
		using func_t = asw_engine*(*)();
		static sigscan sig("RED-Win64-Shipping.exe");
		static const auto func = (func_t)(sig.sig("\x75\x0D\x48\x8B\x83\x60\x0B\x00\x00", "xxxxxxxxx") - 0x48);

		// .xtext:000000014047D5D7                 xor     eax, eax
		// .xtext:000000014047D5D9                 mov     rax, [rax + 0B60h]
		// What the hell
		// Patch out that mov
		static auto once = false;
		if (!once)
		{
			auto *patch = (char*)(func) + 0x59;
			DWORD old_protect;
			VirtualProtect(patch, 7, PAGE_EXECUTE_READWRITE, &old_protect);
			memset(patch, 0x90, 7);
			VirtualProtect(patch, 7, old_protect, &old_protect);
			once = true;
		}

		// Returns [[[143CBBBD8]+F8]+B60]
		return func();
}