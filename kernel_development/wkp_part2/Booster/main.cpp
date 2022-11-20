/* Client Code */
#include <windows.h>
#include <stdio.h>

#include "..\PriorityBooster\PriorityBoosterCommon.h"
int Error(const char* message) {
	printf("%s (error=%d)\n", message, GetLastError());
	return 1;
}

int main(int argc, const char* argv[])
{
	if (argc < 3) {
		printf("Usage: Booster <threadid> <priority>\n");
		return 0;
	}

	// Open handle with same symbolic link
	HANDLE hDevice = CreateFile(L"\\\\.\\PriorityBooster", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE) {
		Error("CreateFile(); failed to open handle to \\\\.\\PriorityBooster\n");
	}

	ThreadData data;
	data.ThreadId = atoi(argv[1]);
	data.Priority = atoi(argv[2]);

	DWORD dwReturned;
	BOOL success = DeviceIoControl(
		hDevice, 
		IOCTL_PRIORITY_BOOSTER_SET_PRIORITY, // control code
		&data, sizeof(data), // input buffer + len
		nullptr, 0, 
		&dwReturned, nullptr // out buffer + len
	);

	if (success)
		printf("Priority change succeeded %d, %d\n", data.Priority, data.ThreadId);
	else {
		printf("DATA: %d, %d\n", data.Priority, data.ThreadId);
		Error("Priority change failed...\n");
	}

	CloseHandle(hDevice);
	return 0;
}