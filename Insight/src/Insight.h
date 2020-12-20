#pragma once

// For use by apps only. No core system should directly be here.

#include "Insight/Core/Core.h"
#include "Insight/Core/Application.h"
#include "Insight/Core/Log.h"
#include "Insight/Time/Stopwatch.h"
#include "Insight/Time/Time.h"
#include "Insight/Instrumentor/Instrumentor.h"
#include "Insight/Input/Input.h"
#include "Insight/Library/Library.h"
#include "Insight/Library/ModelLibrary.h"
#include "../vendor/imgui/imgui.h"

#include "Insight/Entitiy/Entity.h"
#include "Insight/Model/Model.h"
#include "Insight/Model/Mesh.h"
#include "Insight/Scene/Scene.h"

#include "Insight/Component/TransformComponent.h"
#include "Insight/Component/MeshComponent.h"
#include "Insight/Component/CameraComponent.h"

// --Entry Point--------------------------
#include "InSight/Core/EntryPoint.h"
// --Entry Point--------------------------