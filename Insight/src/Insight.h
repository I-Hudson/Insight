#pragma once

// For use by apps only. No core system should directly be here.

#include "Insight/Core.h"
#include "Insight/Application.h"
#include "Insight/Log.h"
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

// --Maths--------------------------
#include "Insight/Maths/Vector2.h"
#include "Insight/Maths/Vector3.h"
#include "Insight/Maths/Vector4.h"
#include "Insight/Maths/Matrix3.h"
#include "Insight/Maths/Matrix4.h"

// --Entry Point--------------------------
#include "InSight/EntryPoint.h"
// --Entry Point--------------------------