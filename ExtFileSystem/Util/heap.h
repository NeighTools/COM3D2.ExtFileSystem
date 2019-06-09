#pragma once

#include <windows.h>

static HANDLE heap;

inline void initialize_heap()
{
	heap = GetProcessHeap();
}
