#pragma once

#include <cmath>

struct FVector2D
{
	float X, Y;

	FVector2D() : X(0), Y(0) {}
	FVector2D(float X, float Y) : X(X), Y(Y) {}

	FVector2D Rotate(const float angle)
	{
		const auto ca = cosf(angle);
		const auto sa = sinf(angle);
		return FVector2D(X * ca + Y * -sa, X * -sa + Y * -ca);
	}
};

struct FVector
{
	float X, Y, Z;

	FVector() : X(0), Y(0), Z(0) {}
	FVector(float X, float Y, float Z) : X(X), Y(Y), Z(Z) {}
};

struct FLinearColor
{
	float R, G, B, A;

	FLinearColor() : R(0), G(0), B(0), A(0) {}
	FLinearColor(float R, float G, float B, float A) : R(R), G(G), B(B), A(A) {}
};

class UCanvas
{
public:
	void K2_DrawLine(FVector2D ScreenPositionA, FVector2D ScreenPositionB, float Thickness, const FLinearColor &RenderColor);
	FVector K2_Project(const FVector &WorldPosition);
};

class AHud
{
	char pad01[0x3D8];
public:
	UCanvas *Canvas;
};