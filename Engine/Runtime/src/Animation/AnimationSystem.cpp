#include "Animation/AnimationSystem.h"
#include "World/WorldSystem.h"

#include "Core/Profiler.h"
#include "Threading/TaskSystem.h"
#include <execution>

namespace Insight
{
    namespace Runtime
    {
#define ANIMATION_THREADING 1
#define PARALLEL_FOR 1

        AnimationSystem::AnimationSystem()
        {
        }

        AnimationSystem::~AnimationSystem()
        {
            std::lock_guard lock(m_animationsLock);
            m_animations.clear();
        }

        void AnimationSystem::Update(const float deltaTime)
        {
            TObjectPtr<World> activeWorld = WorldSystem::Instance().GetActiveWorld();
            if (!activeWorld
                || (activeWorld && activeWorld->GetWorldState() == WorldStates::Paused))
            {
                return;
            }

            std::lock_guard lock(m_animationsLock);

#if PARALLEL_FOR
            Threading::ParallelFor<AnimationInstance>(16, m_animations, [deltaTime](AnimationInstance& instance)
            {
                instance.Animator.Update(deltaTime);
            });
#else
#if ANIMATION_THREADING
            std::vector<std::shared_ptr<Threading::Task>> aniationTasks;
#endif
            for (size_t animIdx = 0; animIdx < m_animations.size(); ++animIdx)
            {
                AnimationInstance& instance = m_animations[animIdx];
#if ANIMATION_THREADING
                aniationTasks.push_back(Threading::TaskSystem::Instance().CreateTask([&]()
                    {
                        instance.Animator.Update(deltaTime);
                    }));
#else
                instance.Animator.Update(deltaTime);
#endif
            }

#if ANIMATION_THREADING
            for (size_t taskIdx = 0; taskIdx < aniationTasks.size(); ++taskIdx)
            {
                IS_PROFILE_SCOPE("Anin wait");
                std::shared_ptr<Threading::Task>& task = aniationTasks[taskIdx];
                task->Wait();
            }
#endif
#endif
        }

        AnimationInstance* AnimationSystem::AddAnimationInstance(const Core::GUID& guid)
        {
            if (AnimationInstance* instance = GetAnimationInstance(guid))
            {
                return instance;
            }

            std::lock_guard lock(m_animationsLock);
            m_animations.push_back(AnimationInstance
            {
                Animator(),
                guid
            });
            return &m_animations.back();
        }

        void AnimationSystem::RemoveAnimationInstance(const Core::GUID& guid)
        {
            std::lock_guard lock(m_animationsLock);
            for (size_t animIdx = 0; animIdx < m_animations.size(); ++animIdx)
            {
                const AnimationInstance& instance = m_animations[animIdx];
                if (guid == instance.OwnerObject)
                {
                    m_animations.erase(m_animations.begin() + animIdx);
                    return;
                }
            }
        }

        AnimationInstance* AnimationSystem::GetAnimationInstance(const Core::GUID& guid)
        {
            std::lock_guard lock(m_animationsLock);
            for (size_t animIdx = 0; animIdx < m_animations.size(); ++animIdx)
            {
                AnimationInstance& instance = m_animations[animIdx];
                if (guid == instance.OwnerObject)
                {
                    return &instance;
                }
            }
            return nullptr;
        }

        const AnimationInstance* AnimationSystem::GetAnimationInstance(const Core::GUID& guid) const
        {
            std::lock_guard lock(m_animationsLock);
            for (size_t animIdx = 0; animIdx < m_animations.size(); ++animIdx)
            {
                const AnimationInstance& instance = m_animations[animIdx];
                if (guid == instance.OwnerObject)
                {
                    return &instance;
                }
            }
            return nullptr;
        }
    }
}
