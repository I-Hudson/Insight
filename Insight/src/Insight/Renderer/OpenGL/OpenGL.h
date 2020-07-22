#pragma once
#ifdef IS_OPENGL
// Include all the headers files for the Vulkan SDK

#ifdef _WIN32
#include "Insight/Log.h"
#include "Insight/Memory/MemoryManager.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define TRACK_OBJECT(x) Insight::Memory::MemoryManager::TrackObject(x);
#define UNTRACK_OBJECT(x) Insight::Memory::MemoryManager::UnTrackObject(x);

#endif
#endif