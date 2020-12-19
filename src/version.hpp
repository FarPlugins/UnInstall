#ifdef _WIN64
#define PLATFORM " x64"
#elif defined _WIN32
#define PLATFORM " x86"
#else
#define PLATFORM ""
#endif

#define PLUGIN_VER_MAJOR 1
#define PLUGIN_VER_MINOR 10
#define PLUGIN_VER_PATCH 18
#define PLUGIN_DESC L"UnInstall for Far Manager 3" PLATFORM
#define PLUGIN_NAME L"UnInstall"
#define PLUGIN_FILENAME L"UnInstall.dll"
#define PLUGIN_COPYRIGHT L"https://github.com/FarPlugins/UnInstall"

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define PLUGIN_VERSION STRINGIZE(PLUGIN_VER_MAJOR) "." STRINGIZE(PLUGIN_VER_MINOR) "." STRINGIZE(PLUGIN_VER_PATCH)
