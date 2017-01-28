#include <QtCore\QString>
#include <Windows.h>
#include <Iphlpapi.h>

#include "utils.h"

// Code of this function based on http://stackoverflow.com/questions/221894/c-get-mac-address-of-network-adapters-on-vista
QString getMacAddress() {
	IP_ADAPTER_INFO AdapterInfo[32];       // Allocate information for up to 32 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_SUCCESS) {
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
		while (pAdapterInfo) {
			if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET) {
				// Get only first MAC address
				return QString::asprintf("%.2x-%.2x-%.2x-%.2x-%.2x-%.2x;",
					pAdapterInfo->Address[0],
					pAdapterInfo->Address[1],
					pAdapterInfo->Address[2],
					pAdapterInfo->Address[3],
					pAdapterInfo->Address[4],
					pAdapterInfo->Address[5]);
			}
			pAdapterInfo = pAdapterInfo->Next;
		}
	}

	return QString();
}