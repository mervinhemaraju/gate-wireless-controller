#pragma once

// Leveled logging macros. Header-only, shared cross-cutting glue - not one
// of the five named modules. Verbose output compiles out below LOG_LEVEL.
// Levels: 0=off, 1=error, 2=warn, 3=info, 4=debug.
//
// Works under both the esp32dev (Arduino) and native (hardware-free test)
// environments, branching on ARDUINO for the underlying print call.

#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#endif

#if defined(ARDUINO)
#include <Arduino.h>
#define LOG_RAW(fmt, ...) Serial.printf(fmt "\n", ##__VA_ARGS__)
#else
#include <cstdio>
#define LOG_RAW(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#endif

#if LOG_LEVEL >= 1
#define LOG_ERROR(fmt, ...) LOG_RAW("[E] " fmt, ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...)
#endif

#if LOG_LEVEL >= 2
#define LOG_WARN(fmt, ...) LOG_RAW("[W] " fmt, ##__VA_ARGS__)
#else
#define LOG_WARN(fmt, ...)
#endif

#if LOG_LEVEL >= 3
#define LOG_INFO(fmt, ...) LOG_RAW("[I] " fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)
#endif

#if LOG_LEVEL >= 4
#define LOG_DEBUG(fmt, ...) LOG_RAW("[D] " fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif
