#pragma once

// For use by apps only. No core system should directly be here.

#include "Engine/Core/Core.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/Log.h"
#include "Engine/Module/ModuleManager.h"

#include "Engine/FileSystem/FileSystem.h"

#include "Engine/Input/Input.h"

#include "Engine/Library/Library.h"
#include "Engine/Library/ModelLibrary.h"

#include "Engine/Time/Stopwatch.h"
#include "Engine/Time/Time.h"

#include "Engine/Instrumentor/Instrumentor.h"

#include "../vendor/imgui/imgui.h"

#include "Engine/Graphics/Model/Model.h"
#include "Engine/Model/Model.h"
#include "Engine/Model/Mesh.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Material.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Entity/Entity.h"

#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/MeshComponent.h"
#include "Engine/Component/CameraComponent.h"

// --Entry Point--------------------------
#include "Engine/Core/EntryPoint.h"
// --Entry Point--------------------------