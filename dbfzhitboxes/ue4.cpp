#include "ue4.h"
#include "sigscan.h"
#include <Windows.h>

using K2_DrawLine_t = void(*)(UCanvas*, FVector2D, FVector2D, float, const FLinearColor&);
K2_DrawLine_t K2_DrawLine_internal;

using K2_Project_t = void(*)(UCanvas*, FVector*, const FVector&);
K2_Project_t K2_Project_internal;

using K2_DrawText_t = void(*)(UCanvas*, UFont*, const FString&, FVector2D, const FLinearColor&, float, const FLinearColor&, FVector2D, bool, bool, bool, const FLinearColor&);
K2_DrawText_t K2_DrawText_internal;

struct get_functions
{
	get_functions()
	{
		sigscan sig("RED-Win64-Shipping.exe");
		K2_DrawLine_internal = (K2_DrawLine_t)(sig.sig("\x0F\x2F\xC8\x76\x72", "xxxxx") - 0x53);
		K2_Project_internal = (K2_Project_t)(sig.sig("\x48\x83\xEC\x30\xF2\x41\x0F\x10\x00\x48\x8B\xDA", "xxxxxxxxxxxx") - 2);
		K2_DrawText_internal = (K2_DrawText_t)(sig.sig("\x41\x83\x78\x08\x01\x48\x8B\xFA", "xxxxxxxx") - 0x1F);
	}
} get_functions_;

void UCanvas::K2_DrawLine(FVector2D ScreenPositionA, FVector2D ScreenPositionB, float Thickness, const FLinearColor &RenderColor)
{
	K2_DrawLine_internal(this, ScreenPositionA, ScreenPositionB, Thickness, RenderColor);
}

FVector UCanvas::K2_Project(const FVector &WorldPosition)
{
	FVector out;
	K2_Project_internal(this, &out, WorldPosition);
	return out;
}

void UCanvas::K2_DrawText(
	UFont *RenderFont,
	const FString &RenderText,
	FVector2D ScreenPosition,
	const FLinearColor &RenderColor,
	float Kerning,
	const FLinearColor &ShadowColor,
	FVector2D ShadowOffset,
	bool bCentreX,
	bool bCentreY,
	bool bOutlined,
	const FLinearColor &OutlineColor)
{
	K2_DrawText_internal(this, RenderFont, RenderText, ScreenPosition, RenderColor, Kerning, ShadowColor, ShadowOffset, bCentreX, bCentreY, bOutlined, OutlineColor);
}