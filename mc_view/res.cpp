#include <Windows.h>
#include <cassert>

#include "resource.h"

#define IDD_DATAFILE 200

void* loadData(unsigned long *dataSize) {
	assert(dataSize != nullptr);
	*dataSize = 0;

	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDD_DATAFILE), RT_RCDATA);
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