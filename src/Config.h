/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
// Build constants

#ifndef CONFIG_H
#define CONFIG_H

#include "BuildConfig.h"
#ifndef BUILD_ID
#define BUILD_ID "0-ffffffff"
#endif
#ifndef BUILD_DATE
#define BUILD_DATE __DATE__
#endif

#ifndef PLATFORM_LINUX
#define PLATFORM_LINUX 0
#endif
#ifndef PLATFORM_MACOS
#define PLATFORM_MACOS 0
#endif
#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS 0
#endif
#if !(PLATFORM_LINUX || PLATFORM_MACOS || PLATFORM_WINDOWS)
#undef PLATFORM_LINUX
#define PLATFORM_LINUX 1
#endif

#define CONFIG_FLOAT_EPSILON (0.000001f)
#define FLOAT_EQUALS(F1,F2) (fabs (F1 - F2) <= CONFIG_FLOAT_EPSILON)
#define BOOL_STRING(VAL) ((VAL) ? "true" : "false")
#define TOSTRING_STRING(VAL) ((VAL) ? (VAL)->toString ().c_str () : "NULL")

#if PLATFORM_LINUX
#define CONFIG_NEWLINE "\n"
#define CONFIG_APPDATA_DIRNAME ".membrane"
#endif

#if PLATFORM_MACOS
#define CONFIG_NEWLINE "\n"
#define CONFIG_APPDATA_DIRNAME APPLICATION_NAME
#endif

#if PLATFORM_WINDOWS
#include <Windows.h>
#include "SDL2/SDL_config_windows.h"
#define __attribute__(X)
#define __PRETTY_FUNCTION__ ""
#define S_ISDIR(X) ((X) && _S_IFDIR)
#define SHUT_RDWR SD_BOTH
#define CONFIG_NEWLINE "\r\n"
#define CONFIG_APPDATA_DIRNAME APPLICATION_NAME
#endif

#endif
