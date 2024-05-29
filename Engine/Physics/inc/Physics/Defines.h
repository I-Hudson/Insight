#pragma once

#ifdef IS_EXPORT_DLL
#ifdef IS_EXPORT_PHYSICS_DLL
#define IS_PHYSICS __declspec(dllexport)
#else
#define IS_PHYSICS __declspec(dllimport)
#endif
#else
#define IS_PHYSICS
#endif

#if defined(IS_PHYSICS_JOLT) && defined(IS_PHYSICS_PHYSX)
#error Define either 'IS_PHYSICS_JOLT' or 'IS_PHYSICS_PHYSX', you can't have both defined at once'.
#endif

/// If objects are closer than this distance, they are considered to be colliding (used for GJK) (unit: meter)
constexpr float cDefaultCollisionTolerance = 1.0e-4f;

/// A factor that determines the accuracy of the penetration depth calculation. If the change of the squared distance is less than tolerance * current_penetration_depth^2 the algorithm will terminate. (unit: dimensionless)
constexpr float cDefaultPenetrationTolerance = 1.0e-4f; ///< Stop when there's less than 1% change

/// How much padding to add around objects
constexpr float cDefaultConvexRadius = 0.05f;

/// Used by (Tapered)CapsuleShape to determine when supporting face is an edge rather than a point (unit: meter)
static constexpr float cCapsuleProjectionSlop = 0.02f;

/// Maximum amount of jobs to allow
constexpr int cMaxPhysicsJobs = 2048;

/// Maximum amount of barriers to allow
constexpr int cMaxPhysicsBarriers = 8;

//#pragma warning( disable : 4251 )