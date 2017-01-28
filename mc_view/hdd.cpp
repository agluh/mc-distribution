#include <QtCore\QString>
#include <QtCore\QDebug>

#define _WIN32_DCOM

#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>

#include "utils.h"

// Code of this function based on http://forum.sources.ru/index.php?showtopic=329214
QString getHDDInfo() {
	HRESULT hres;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;
	VARIANT vtProp;

	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) {
		if (hres != RPC_E_CHANGED_MODE) {
			qDebug() << "Failed to initialize COM library. Error code = 0x" << QString::number(hres, 16);
			return QString();
		}
		else {
			qDebug() << "COM already initialized.";
		}
	}

	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------
	// Note: If you are using Windows 2000, you need to specify -
	// the default authentication credentials for a user by using
	// a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
	// parameter of CoInitializeSecurity ------------------------

	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities
		NULL                         // Reserved
	);


	if (FAILED(hres)) {
		qDebug() << "Failed to initialize security. Error code = 0x" << QString::number(hres, 16);
		CoUninitialize();
		return QString();
	}

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres)) {
		qDebug() << "Failed to create IWbemLocator object. Err code = 0x" << QString::number(hres, 16);
		CoUninitialize();
		return QString();
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	IWbemServices *pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (e.g. Kerberos)
		0,                       // Context object
		&pSvc                    // pointer to IWbemServices proxy
	);

	if (FAILED(hres)) {
		qDebug() << "Could not connect. Error code = 0x" << QString::number(hres, 16);
		pLoc->Release();
		CoUninitialize();
		return QString();
	}

	qDebug() << "Connected to ROOT\\CIMV2 WMI namespace";


	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities
	);

	if (FAILED(hres)) {
		qDebug() << "Could not set proxy blanket. Error code = 0x" << QString::number(hres, 16);
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return QString();
	}

	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	// For example, get the model names and serial numbers of the hard drives
	pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_DiskDrive"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres)) {
		qDebug() << "Query for operating system name failed. Error code = 0x" << QString::number(hres, 16);
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return QString();
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	pclsObj = NULL;
	uReturn = 0;

	QString text;
	while (pEnumerator) {
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

		if (uReturn == 0) {
			break;
		}

		VariantClear(&vtProp);

		hr = pclsObj->Get(L"MediaType", 0, &vtProp, 0, 0);
		QString type = QString((QChar*)vtProp.bstrVal, ::SysStringLen(vtProp.bstrVal));
		VariantClear(&vtProp);

		if (type == "Fixed hard disk media") {
			// Get the value of the Name property
			hr = pclsObj->Get(L"Model", 0, &vtProp, 0, 0);
			QString model = QString((QChar*)vtProp.bstrVal, ::SysStringLen(vtProp.bstrVal));
			VariantClear(&vtProp);

			hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
			QString serialNumber = QString((QChar*)vtProp.bstrVal, ::SysStringLen(vtProp.bstrVal));
			VariantClear(&vtProp);

			if (model != serialNumber) {
				text = model + ":" + serialNumber + ";"; // Get only first HDD
				break;
			}
		}

		pclsObj->Release();
	}

	// Cleanup
	// ========
	pEnumerator->Release();

	// Cleanup
	// ========
	pSvc->Release();
	pLoc->Release();
	CoUninitialize();

	return text;
}