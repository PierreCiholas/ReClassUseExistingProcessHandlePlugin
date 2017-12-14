#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <sstream>
#include <memory>

#include "ReClassAPI.h"

HANDLE UseExistingHandle(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessID);