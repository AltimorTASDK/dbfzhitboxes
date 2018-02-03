#include "sigscan.h"
#include "ue4.h"
#include "arcsys.h"
#include <Windows.h>
#include <Psapi.h>
#include <stdexcept>
#include <array>

extern "C"
{
	uintptr_t hud_postrender_orig;
	void hud_postrender_hook();
}

void draw_hithurtboxes(AHud *hud, const asw_entity *entity, const asw_entity *parent = nullptr)
{
	for (auto boxidx = 0; boxidx < entity->hithurtbox_count(); boxidx++)
	{
		const auto &box = entity->hithurtbox_data()[boxidx];

		// Don't show inactive hitboxes
		if (box.type == 1 && (parent != nullptr ? !parent->is_active() : !entity->is_active()))
			continue;
		else if (box.type == 0 && entity->is_invuln())
			continue;

		std::array<FVector2D, 4> corners =
		{
			FVector2D(box.x, box.y),
			FVector2D(box.x + box.w, box.y),
			FVector2D(box.x + box.w, box.y + box.h),
			FVector2D(box.x, box.y + box.h)
		};

		for (auto &pos : corners)
		{
			pos.X *= entity->scale_x();
			pos.Y *= entity->scale_y();

			pos = pos.Rotate(entity->angle_rads());

			if (entity->direction() == 0)
				pos.X *= -1.F;

			pos.X += entity->get_pos_x();
			pos.Y += entity->get_pos_y();

			pos.X *= asw_engine::coord_scale / 1000.F;
			pos.Y *= asw_engine::coord_scale / 1000.F;

			const auto proj = hud->Canvas->K2_Project(FVector(pos.X, 0.F, pos.Y));
			pos = FVector2D(proj.X, proj.Y);
		}

		for (auto i = 0; i < 4; i++)
		{
			if (box.type == 1)
				hud->Canvas->K2_DrawLine(corners[i], corners[(i + 1) % 4], 2.F, FLinearColor(1.F, 0.F, 0.F, 1.F));
			else if (entity->is_counterhit())
				hud->Canvas->K2_DrawLine(corners[i], corners[(i + 1) % 4], 2.F, FLinearColor(0.F, 1.F, .5F, 1.F));
			else
				hud->Canvas->K2_DrawLine(corners[i], corners[(i + 1) % 4], 2.F, FLinearColor(0.F, 1.F, 0.F, 1.F));
		}
	}
}

void draw_pushbox(AHud *hud, const asw_entity *entity)
{
	const auto width = (float)(entity->pushbox_width());
	const auto top = (float)(entity->pushbox_top());
	const auto bottom = (float)(entity->pushbox_bottom());
	const auto front_offset = (float)(entity->pushbox_front_offset());

	std::array<FVector2D, 4> corners =
	{
		FVector2D(-width / 2 - front_offset, top),
		FVector2D(width / 2, top),
		FVector2D(width / 2, -bottom),
		FVector2D(-width / 2 - front_offset, -bottom)
	};

	for (auto &pos : corners)
	{
		if (entity->direction() == 0)
			pos.X *= -1.F;

		pos.X += entity->get_pos_x();
		pos.Y += entity->get_pos_y();

		pos.X *= asw_engine::coord_scale / 1000.F;
		pos.Y *= asw_engine::coord_scale / 1000.F;

		const auto proj = hud->Canvas->K2_Project(FVector(pos.X, 0.F, pos.Y));
		pos = FVector2D(proj.X, proj.Y);
	}

	for (auto i = 0; i < 4; i++)
		hud->Canvas->K2_DrawLine(corners[i], corners[(i + 1) % 4], 2.F, FLinearColor(1.F, 1.F, 0.F, 1.F));
}

extern "C" void draw_overlay(AHud *hud)
{
	if (hud->Canvas == nullptr)
		return;

	const auto *engine = asw_engine::get();
	if (engine == nullptr)
		return;

	for (auto entidx = 0; entidx < engine->entity_count(); entidx++)
	{
		const auto *entity = engine->entity_list()[entidx];
		draw_pushbox(hud, entity);
		draw_hithurtboxes(hud, entity);

		if (entity->attached() != nullptr)
		{
			draw_pushbox(hud, entity->attached());
			draw_hithurtboxes(hud, entity->attached(), entity);
		}
	}
}

uintptr_t insert_jmp(uintptr_t addr, void *dest)
{
	constexpr auto patch_size = 12;

	DWORD old_protect;
	VirtualProtect((void*)(addr), patch_size, PAGE_EXECUTE_READWRITE, &old_protect);

	*(WORD*)(addr) = 0xB848; // mov rax, dest
	*(void**)(addr + 2) = dest;
	*(WORD*)(addr + 10) = 0xE0FF; // jmp rax

	VirtualProtect((void*)(addr), patch_size, old_protect, &old_protect);

	return addr;
}

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, void *reserved)
{
	if (reason != DLL_PROCESS_ATTACH)
		return false;

	sigscan sig("RED-Win64-Shipping.exe");
	hud_postrender_orig = insert_jmp(sig.sig("\xA8\x04\x74\x7A", "xxxx") - 0x77, hud_postrender_hook);

	return true;
}