#include <iostream>
#include <string>
#include <Windows.h>
#include <ShlObj.h>
#include <Psapi.h>

using namespace std;
constexpr auto appName = "helper";
constexpr auto targetClass = "YBMVIEW1";

inline void msg(const string& text, UINT type = 0) {
	MessageBox(NULL, text.c_str(), appName, type);
}

DWORD getModuleBaseAddress(HANDLE hProcess) {
	HMODULE hMods[1024];
	DWORD cbNeeded;
	char modName[255];
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
		for (auto i = 0ul; i < (cbNeeded / sizeof(HMODULE)); i++) {
			GetModuleBaseName(hProcess, hMods[i], modName, 255);
			if (_strnicmp(modName, "printersetup.dll", 16) == 0) {
				return (DWORD)hMods[i];
			}
		}
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	if (IsUserAnAdmin() == FALSE) {
		msg("관리자 권한으로 실행해 주세요.", MB_ICONERROR);
		return 1;
	}

	auto hWnd = FindWindow(targetClass, NULL);
	if (hWnd == NULL) {
		msg("YBM VIEWER를 찾을 수 없습니다.", MB_ICONERROR);
		return 1;
	}

	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);

	shared_ptr<void> hProcess(
		OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid),
		&CloseHandle
	);
	
	if (hProcess.get() == NULL) {
		msg("YBM VIEWER 프로세스를 찾을 수 없습니다.", MB_ICONERROR);
		return 1;
	}

	auto baseAddress = getModuleBaseAddress(hProcess.get());
	if (baseAddress == 0) {
		msg("Module을 찾을 수 없습니다.", MB_ICONERROR);
		return 1;
	}

	auto targetAddress = baseAddress + 0x5b35;
	DWORD flOldProtect;
	DWORD numberOfBytesWritten;
	BYTE buffer = 0;
	VirtualProtectEx(hProcess.get(), (LPVOID)targetAddress, 1, PAGE_EXECUTE_READWRITE, &flOldProtect);
	WriteProcessMemory(hProcess.get(), (LPVOID)targetAddress, &buffer, 1, &numberOfBytesWritten);
	VirtualProtectEx(hProcess.get(), (LPVOID)targetAddress, 1, flOldProtect, &flOldProtect);

	SetWindowText(hWnd, "YBM VIEWER *_*");
	msg("완료되었습니다.", MB_ICONINFORMATION);

	return 0;
}