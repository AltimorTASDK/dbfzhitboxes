#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

int main()
{
	const auto *exe = "RED-Win64-Shipping.exe";
	const auto *dll = "dbfzhitboxes.dll";

	const auto snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);

	Process32First(snap, &entry);
	do 
	{
		const auto proc = OpenProcess(PROCESS_ALL_ACCESS, false, entry.th32ProcessID);

		char path[MAX_PATH];
		if (GetModuleFileNameEx(proc, nullptr, path, MAX_PATH) == 0)
		{
			CloseHandle(proc);
			continue;
		}

		if (strcmp(path + strlen(path) - strlen(exe), exe) != 0)
		{
			CloseHandle(proc);
			continue;
		}

		char dll_path[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, dll_path);
		strcat_s(dll_path, MAX_PATH, "/");
		strcat_s(dll_path, MAX_PATH, dll);

		const auto size = strlen(dll_path) + 1;
		auto *buf = VirtualAllocEx(proc, nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		WriteProcessMemory(proc, buf, dll_path, size, nullptr);
		CreateRemoteThread(proc, nullptr, 0, (LPTHREAD_START_ROUTINE)(LoadLibrary), buf, 0, nullptr);

		CloseHandle(proc);
		CloseHandle(snap);
		break;
	}
	while (Process32Next(snap, &entry));

	return 0;
}
