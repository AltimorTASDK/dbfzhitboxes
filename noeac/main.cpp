#include <Windows.h>

int main()
{
	ShellExecute(nullptr, "open", "RED\\Binaries\\Win64\\RED-WIN64-Shipping.exe", " -eac-nop-loaded", nullptr, SW_SHOW);

	return 0;
}
