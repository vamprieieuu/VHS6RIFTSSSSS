#pragma once
#ifndef _ENTRY_H
#define _ENTRY_H

#if defined(_MSC_VER)
	#ifndef DllExport
		#define DllExport   __declspec( dllexport )
	#endif
#else
	#ifndef DllExport
		#define DllExport   __attribute__((visibility("default")))
	#endif
#endif

#endif // _ENTRY_H
