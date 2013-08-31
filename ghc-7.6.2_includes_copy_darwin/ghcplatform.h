#ifndef __GHCPLATFORM_H__
#define __GHCPLATFORM_H__

#define BuildPlatform_TYPE  x86_64_apple_darwin
#define HostPlatform_TYPE   x86_64_apple_darwin

#define x86_64_apple_darwin_BUILD  1
#define x86_64_apple_darwin_HOST  1

#define x86_64_BUILD_ARCH  1
#define x86_64_HOST_ARCH  1
#define BUILD_ARCH  "x86_64"
#define HOST_ARCH  "x86_64"

#define darwin_BUILD_OS  1
#define darwin_HOST_OS  1
#define BUILD_OS  "darwin"
#define HOST_OS  "darwin"

#define apple_BUILD_VENDOR  1
#define apple_HOST_VENDOR  1
#define BUILD_VENDOR  "apple"
#define HOST_VENDOR  "apple"

/* These TARGET macros are for backwards compatibily... DO NOT USE! */
#define TargetPlatform_TYPE x86_64_apple_darwin
#define x86_64_apple_darwin_TARGET  1
#define x86_64_TARGET_ARCH  1
#define TARGET_ARCH  "x86_64"
#define darwin_TARGET_OS  1
#define TARGET_OS  "darwin"
#define apple_TARGET_VENDOR  1

#endif /* __GHCPLATFORM_H__ */
