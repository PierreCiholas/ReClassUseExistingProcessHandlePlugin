#pragma once
#include <Windows.h>
#include <Winternl.h>
#include <string>
#include <ntstatus.h>
#include <Psapi.h>

#define SYSTEMHANDLEINFORMATION 16

#pragma comment (lib, "ntdll.lib")

using namespace std;

typedef struct _SYSTEM_HANDLE {
	ULONG ProcessId;
	UCHAR ObjectTypeNumber;
	UCHAR Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION {
	ULONG HandleCount;
	SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO {
	DWORD UniqueProcessId;
	WORD HandleType;
	USHORT HandleValue;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _OBJECT_TYPE_INFORMATION {
	UNICODE_STRING TypeName;
	ULONG TotalNumberOfObjects;
	ULONG TotalNumberOfHandles;
	ULONG TotalPagedPoolUsage;
	ULONG TotalNonPagedPoolUsage;
	ULONG TotalNamePoolUsage;
	ULONG TotalHandleTableUsage;
	ULONG HighWaterNumberOfObjects;
	ULONG HighWaterNumberOfHandles;
	ULONG HighWaterPagedPoolUsage;
	ULONG HighWaterNonPagedPoolUsage;
	ULONG HighWaterNamePoolUsage;
	ULONG HighWaterHandleTableUsage;
	ULONG InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	ULONG ValidAccessMask;
	BOOLEAN SecurityRequired;
	BOOLEAN MaintainHandleCount;
	UCHAR TypeIndex;
	CHAR ReservedByte;
	ULONG PoolType;
	ULONG DefaultPagedPoolCharge;
	ULONG DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

HANDLE GetHandleTo(DWORD pid = 0);

HANDLE GetHandleTo(DWORD pid) {
	if (pid == 0)
		return (HANDLE)0x0; // Invalid PID

	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PVOID buffer = NULL;
	ULONG buffersize = 0;
	while (true) {
		status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SYSTEMHANDLEINFORMATION, buffer, buffersize, &buffersize);
		if (!NT_SUCCESS(status)) {
			if (status == STATUS_INFO_LENGTH_MISMATCH) {
				if (buffer != NULL)
					VirtualFree(buffer, 0, MEM_RELEASE);
				buffer = VirtualAlloc(NULL, buffersize, MEM_COMMIT, PAGE_READWRITE);
			}
			continue;
		}
		else
			break;
	}

	// Enumerate all handles on system
	PSYSTEM_HANDLE_INFORMATION handleInfo = (PSYSTEM_HANDLE_INFORMATION)buffer;

	PVOID buffer2 = NULL;
	ULONG buffersize2 = 0;
	for (ULONG i = 0; i < handleInfo->HandleCount; i++) {
		PSYSTEM_HANDLE_TABLE_ENTRY_INFO Handle = (PSYSTEM_HANDLE_TABLE_ENTRY_INFO)&handleInfo->Handles[i];
		if (!Handle)
			continue;
		if (!Handle->HandleValue)
			continue;
		if (Handle->UniqueProcessId != GetCurrentProcessId())
			continue; // Modify that if duplicating handles to enable looking for handles in other processes

		int trys = 0;
		while (true) {
			if (trys == 20)
				break;
			trys += 1;

			// NtQueryObject can be called on handles that were closed between the snapshot and this call, careful when working on process with strict handle policy
			status = NtQueryObject((HANDLE)Handle->HandleValue, ObjectTypeInformation, buffer2, buffersize2, &buffersize2); // Return objecttypeinfo into buffer
			if (!NT_SUCCESS(status)) {
				if (buffer2 != NULL)
					VirtualFree(buffer2, 0, MEM_RELEASE); // If buffer filled with anything, but call didnt succeed, assume its bullshit, so clear it
				buffer2 = VirtualAlloc(NULL, buffersize2, MEM_COMMIT, PAGE_READWRITE); // Allocate with new mem
			}
			else {
				if (wcsncmp(((POBJECT_TYPE_INFORMATION)buffer2)->TypeName.Buffer, L"Process", ((POBJECT_TYPE_INFORMATION)buffer2)->TypeName.Length + 1) == 0) {
					DWORD handleOnPID = GetProcessId((HANDLE)Handle->HandleValue);
					if (pid == handleOnPID /*&& Handle->GrantedAccess == PROCESS_ALL_ACCESS*/) {
						HANDLE handleFound = (HANDLE)Handle->HandleValue;
						VirtualFree(buffer, 0, MEM_RELEASE); // Empties buffers to avoid memory leaks
						VirtualFree(buffer2, 0, MEM_RELEASE); // Empties buffers to avoid memory leaks
						return handleFound;
					} else
						break;
				} else
					break;
			}
		}
		continue;
	}

	VirtualFree(buffer, 0, MEM_RELEASE); // Empties buffers to avoid memory leaks
	VirtualFree(buffer2, 0, MEM_RELEASE); // Empties buffers to avoid memory leaks
	return (HANDLE)0x0;
}