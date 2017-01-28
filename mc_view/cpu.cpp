#include <QtCore\QString>
#include <intrin.h>
#include <array>
#include <vector>
#include <string>

#include "utils.h"

// Code of this function based on https://msdn.microsoft.com/ru-ru/library/hskdteyh.aspx
QString getCPUInfo() {
	int nIds_;
	int nExIds_;
	std::array<int, 4> cpui;
	std::vector<std::array<int, 4>> data_;
	std::vector<std::array<int, 4>> extdata_;
	std::string vendor_;
	std::string brand_;

	// Calling __cpuid with 0x0 as the function_id argument  
	// gets the number of the highest valid function ID.  
	__cpuid(cpui.data(), 0);
	nIds_ = cpui[0];

	for (int i = 0; i <= nIds_; ++i) {
		__cpuidex(cpui.data(), i, 0);
		data_.push_back(cpui);
	}

	// Capture vendor string  
	char vendor[0x20];
	memset(vendor, 0, sizeof(vendor));
	*reinterpret_cast<int*>(vendor) = data_[0][1];
	*reinterpret_cast<int*>(vendor + 4) = data_[0][3];
	*reinterpret_cast<int*>(vendor + 8) = data_[0][2];
	vendor_ = vendor;

	// Calling __cpuid with 0x80000000 as the function_id argument  
	// gets the number of the highest valid extended ID.  
	__cpuid(cpui.data(), 0x80000000);
	nExIds_ = cpui[0];

	char brand[0x40];
	memset(brand, 0, sizeof(brand));

	for (int i = 0x80000000; i <= nExIds_; ++i) {
		__cpuidex(cpui.data(), i, 0);
		extdata_.push_back(cpui);
	}

	// Interpret CPU brand string if reported  
	if (nExIds_ >= 0x80000004) {
		memcpy(brand, extdata_[2].data(), sizeof(cpui));
		memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
		memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
		brand_ = brand;
	}

	return QString::fromStdString(vendor_).trimmed() + ":" + QString::fromStdString(brand_).trimmed() + ";";
}