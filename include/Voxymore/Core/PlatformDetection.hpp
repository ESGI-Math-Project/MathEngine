//#pragma once
//
//// Main file use from Hazel from the cherno (https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/PlatformDetection.h)
//// cf LICENSE : (https://github.com/TheCherno/Hazel/blob/master/LICENSE)
//
//// Platform detection using predefined macros
//#ifdef _WIN32
///* Windows x64/x86 */
//	#ifdef _WIN64
//		/* Windows x64  */
//		#define VXM_PLATFORM_WINDOWS
//	#else
//		/* Windows x86 */
//		#error "x86 Builds are not supported!"
//    #endif
//
//#elif defined(__APPLE__) || defined(__MACH__)
//#include <TargetConditionals.h>
//	/* TARGET_OS_MAC exists on all the platforms
//	 * so we must check all of them (in this order)
//	 * to ensure that we're running on MAC
//	 * and not some other Apple platform */
//	#if TARGET_IPHONE_SIMULATOR == 1
//		#error "IOS simulator is not supported!"
//	#elif TARGET_OS_IPHONE == 1
//		#define VXM_PLATFORM_IOS
//		#error "IOS is not supported!"
//	#elif TARGET_OS_MAC == 1
//		#define VXM_PLATFORM_MACOS
//	#else
//		#error "Unknown Apple platform!"
//    #endif
//
///* We also have to check __ANDROID__ before __linux__
// * since android is based on the linux kernel
// * it has __linux__ defined */
//#elif defined(__ANDROID__)
//#define VXM_PLATFORM_ANDROID
//	#error "Android is not supported!"
//#elif defined(__linux__)
//#define VXM_PLATFORM_LINUX
//#else
///* Unknown compiler/platform */
//#error "Unknown platform!"
//#endif // End of platform detection