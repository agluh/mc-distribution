#ifndef VERSION_H 
#define VERSION_H 
#pragma once 
#define Q(x) #x 
#define QUOTE(x) Q(x) 
#define _FILE_VERSION_MAJOR 1 
#define _FILE_VERSION_MINOR 1 
#define _FILE_VERSION_BAGFIX 0 
#define _FILE_VERSION_BUILD 32 
#define _FILE_VERSION _FILE_VERSION_MAJOR,_FILE_VERSION_MINOR,_FILE_VERSION_BAGFIX,_FILE_VERSION_BUILD                                  
#define _FILE_VERSION_STRING QUOTE(_FILE_VERSION_MAJOR._FILE_VERSION_MINOR._FILE_VERSION_BAGFIX._FILE_VERSION_BUILD)                                  
#define _PRODUCT_VERSION 1,0,0,0                                  
#define _PRODUCT_VERSION_STRING "1.0.0.0"                                  
#endif // VERSION_H                                  
