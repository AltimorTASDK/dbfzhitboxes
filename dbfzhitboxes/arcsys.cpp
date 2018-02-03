#include "arcsys.h"
#include "sigscan.h"
#include <Windows.h>

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