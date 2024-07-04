#ifndef PLATFORM_H

/**
 * https://stackoverflow.com/questions/142508/how-do-i-check-os-with-a-preprocessor-directive
 * Determination a platform of an operation system
 * Fully supported supported only GNU GCC/G++, partially on Clang/LLVM
 */

#if defined(_WIN32)
    #define PLATFORM_NAME "windows" // Windows
    #define PLATFORM_WINDOWS
#elif defined(_WIN64)
    #define PLATFORM_NAME "windows" // Windows
    #define PLATFORM_WINDOWS
#elif defined(__CYGWIN__) && !defined(_WIN32)
    #define PLATFORM_NAME "windows-cygwin" // Windows (Cygwin POSIX under Microsoft Window)
    #define PLATFORM_CYGWIN
#elif defined(__ANDROID__)
    #define PLATFORM_NAME "android" // Android (implies Linux, so it must come first)
    #define PLATFORM_ANDROID
#elif defined(__linux__)
    #define PLATFORM_NAME "linux" // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
    #define PLATFORM_LINUX
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
    #include <sys/param.h>
    #if defined(BSD)
        #define PLATFORM_NAME "bsd" // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
        #define PLATFORM_BSD
    #endif
#elif defined(__hpux)
    #define PLATFORM_NAME "hp-ux" // HP-UX
    #define PLATFORM_HP_UX
#elif defined(_AIX)
    #define PLATFORM_NAME "aix" // IBM AIX
    #define PLATFORM_AIX
#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR == 1
        #define PLATFORM_NAME "ios" // Apple iOS
        #define PLATFORM_IOS
    #elif TARGET_OS_IPHONE == 1
        #define PLATFORM_NAME "ios" // Apple iOS
        #define PLATFORM_IOS
    #elif TARGET_OS_MAC == 1
        #define PLATFORM_NAME "osx" // Apple OSX
        #define PLATFORM_OSX
    #endif
#elif defined(__sun) && defined(__SVR4)
    #define PLATFORM_NAME "solaris" // Oracle Solaris, Open Indiana
    #define PLATFORM_SOLARIS
#else
    #define PLATFORM_NAME NULL
#endif

#if defined(PLATFORM_WINDOWS)
    #define PLATFORM_CH_SUBDIR  '\\'
#else
    #define PLATFORM_CH_SUBDIR  '/'
#endif

#define ERR_PLATFORM_COLORPRINT_INIT "failed enabling color prints"
int platform_colorprint_init(void);

int platform_getch(void);
void platform_clear(void);
void platform_trace(void);

#define PLATFORM_H
#endif


