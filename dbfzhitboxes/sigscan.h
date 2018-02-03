#pragma once

#include <cstdint>

class sigscan
{
	uintptr_t start, end;

public:
	sigscan(const char *name);
	uintptr_t sig(const char *sig, const char *mask);
};