#include <Windows.h>
#include <cassert>

#include "resource.h"

#define IDD_DATAFILE 200

void* loadViewerData(unsigned long *dataSize) {
	assert(dataSize != nullptr);
	*dataSize = 0;

	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_VIEWER), RT_RCDATA);
	if (hRes != NULL) {
		unsigned long size = SizeofResource(NULL, hRes);
		if (size != NULL) {
			HGLOBAL hResData = LoadResource(NULL, hRes);
			if (hResData != NULL) {
				void *pRes = LockResource(hResData);
				if (pRes != NULL) {
					*dataSize = size;
					return pRes;
				}
			}
		}
	}

	return nullptr;
}

bool updateViewerResources(const char* filename, void *data, unsigned long dataSize) {
	HANDLE hUpdateRes = BeginUpdateResource(filename, FALSE);
	if (hUpdateRes == NULL) {
		return false;
	}

	if (UpdateResource(hUpdateRes, RT_RCDATA, MAKEINTRESOURCE(IDD_DATAFILE), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), data, dataSize) == FALSE) {
		return false;
	}

	if (EndUpdateResource(hUpdateRes, FALSE) == FALSE) {
		return false;
	}

	return true;
}