#pragma once

struct ThreadData
{
	ULONG ThreadId;
	int Priority;
};

/*
CTL_CODE( DeviceType, Function, Method, Access ) ( \
	((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
DeviceType - identifies type of device
Function - Ascending number indicating a specific operation.
Method - Indicates how the input and output buffers provided by the client pass to the driver
Access - Indicates whether this operation is to the driver (FILE_WRITE_ACCESS),
		 from the driver (FILE_READ_ACCESS), or both ways (FILE_ANY_ACCESS). Typical drivers use FILE_ANY_ACCESS
*/

// MSD specifies that values for 3rd parties should start with **0x8000**
#define PRIORITY_BOOSTER_DEVICE 0x8000

#define IOCTL_PRIORITY_BOOSTER_SET_PRIORITY CTL_CODE(PRIORITY_BOOSTER_DEVICE, \
												0x800, METHOD_NEITHER, FILE_ANY_ACCESS)