#include "Plugin.h"
#include "resource.h"
#include <string>
#include "GetHandle.h"

BOOL PLUGIN_CC PluginInit(PRECLASS_PLUGIN_INFO lpRCInfo) {
	wcscpy_s(lpRCInfo->Name, L"Use Existing Handle");
	wcscpy_s(lpRCInfo->Version, L"0.1");
	wcscpy_s(lpRCInfo->About, L"When attaching to a process, this plugin first looks through the existing process HANDLEs of ReClass. If a process handle already exist to the target process, it is used and OpenProcess is not used, otherwise, a classic OpenProcess is called.");
	lpRCInfo->DialogID = IDD_SETTINGS_DLG;

	if (!ReClassIsOpenProcessOverriden()) {
		if (ReClassOverrideOpenProcessOperation((tOpenProcessOperation)(void*)UseExistingHandle) == FALSE) {
			ReClassPrintConsole(L"[UseExistingHandle] Failed to open process operation, failing PluginInit");
			return FALSE;
		}
	}

	return TRUE;
}

VOID PLUGIN_CC PluginStateChange(BOOL state) {
	ReClassPrintConsole((state == FALSE) ? L"[UseExistingHandle] Disabled!" : L"[UseExistingHandle] Enabled!");
}

INT_PTR CALLBACK PluginSettingsDlg(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg)
	{

	case WM_INITDIALOG:
	{
		/*SendMessage(
			GetDlgItem(hWnd, IDC_CHECK_READ_MEMORY_OVERRIDE),
			BM_SETCHECK,
			MAKEWPARAM(ReClassIsReadMemoryOverriden() ? BST_CHECKED : BST_UNCHECKED, 0),
			0
		);
		SendMessage(
			GetDlgItem(hWnd, IDC_CHECK_WRITE_MEMORY_OVERRIDE),
			BM_SETCHECK,
			MAKEWPARAM(ReClassIsWriteMemoryOverriden() ? BST_CHECKED : BST_UNCHECKED, 0),
			0
		);*/
	}
	return TRUE;

	case WM_COMMAND:
	{
		WORD NotificationCode = HIWORD(wParam);
		WORD ControlId = LOWORD(wParam);
		HWND hControlWnd = reinterpret_cast<HWND>(lParam);

		if (NotificationCode == BN_CLICKED)
		{
			BOOLEAN bChecked = (SendMessage(hControlWnd, BM_GETCHECK, 0, 0) == BST_CHECKED);

			/*if (ControlId == IDC_CHECK_READ_MEMORY_OVERRIDE)
			{
				if (bChecked)
					ReClassOverrideReadMemoryOperation(ReadCallback);
				else
					ReClassRemoveReadMemoryOverride();
			}
			else if (ControlId == IDC_CHECK_WRITE_MEMORY_OVERRIDE)
			{
				if (bChecked)
					ReClassOverrideWriteMemoryOperation(WriteCallback);
				else
					ReClassRemoveWriteMemoryOverride();
			}*/
		}
	}
	break;

	case WM_CLOSE:
	{
		EndDialog(hWnd, 0);
	}
	break;

	}
	return FALSE;
}

HANDLE UseExistingHandle(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessID) {
	// When clicking on the icon to attach, the OpenProcess procedure is called requesting a HANDLE with permission 0x410 (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ)
	// Therefore, we look for an existing handle only when a PROCESS_ALL_ACCESS is requested
	if (dwDesiredAccess == PROCESS_ALL_ACCESS) {
		HANDLE existingHandle = GetHandleTo(dwProcessID);
		if (existingHandle > 0)
			return existingHandle;
	}

	//std::wstring message = L"dwDesiredAccess = " + std::to_wstring(dwDesiredAccess) + L", bInheritHandle = " + std::to_wstring(bInheritHandle) + L", dwProcessID = " + std::to_wstring(dwProcessID);
	//MessageBox(NULL, message.c_str(), L"Plugin triggered", MB_OK);
	return OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessID);
}