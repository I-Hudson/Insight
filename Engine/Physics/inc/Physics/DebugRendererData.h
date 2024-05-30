#pragma once

#include "Physics/Defines.h"

#include "Maths/Vector3.h"
#include "Maths/Vector4.h"

#include <string>
#include <mutex>

namespace Insight::Physics
{
    struct IS_PHYSICS DebugRendererData
    {
        DebugRendererData() = default;

        DebugRendererData(const DebugRendererData& other)
        {
            std::lock_guard linesLock(LinesMutex);
            std::lock_guard otherLinesLock(other.LinesMutex);

            std::lock_guard textLock(TextsMutex);
            std::lock_guard otherTextLock(other.TextsMutex);

            Lines = other.Lines;
            Texts = other.Texts;
        }
        DebugRendererData(DebugRendererData&& other)
        {
            std::lock_guard linesLock(LinesMutex);
            std::lock_guard otherLinesLock(other.LinesMutex);

            std::lock_guard textLock(TextsMutex);
            std::lock_guard otherTextLock(other.TextsMutex);

            Lines = std::move(other.Lines);
            Texts = std::move(other.Texts);

            other.Lines.clear();
            other.Texts.clear();
        }

        DebugRendererData& operator=(const DebugRendererData& other)
        {
            std::lock_guard linesLock(LinesMutex);
            std::lock_guard otherLinesLock(other.LinesMutex);

            std::lock_guard textLock(TextsMutex);
            std::lock_guard otherTextLock(other.TextsMutex);

            Lines = other.Lines;
            Texts = other.Texts;

            return *this;
        }
        DebugRendererData& operator=(DebugRendererData&& other)
        {
            std::lock_guard linesLock(LinesMutex);
            std::lock_guard otherLinesLock(other.LinesMutex);

            std::lock_guard textLock(TextsMutex);
            std::lock_guard otherTextLock(other.TextsMutex);

            Lines = std::move(other.Lines);
            Texts = std::move(other.Texts);

            other.Lines.clear();
            other.Texts.clear();

            return *this;
        }

        struct IS_PHYSICS Line
        {
            Maths::Vector3 From;
            Maths::Vector3 FromColour;
            Maths::Vector3 To;
            Maths::Vector3 ToColour;
        };

        struct IS_PHYSICS Text
        {
            Maths::Vector3 Position;
            std::string Text;
            Maths::Vector4 Colour;
            float Height;
        };

        std::vector<Line> Lines;
        mutable std::mutex LinesMutex;
    
        std::vector<Text> Texts;
        mutable std::mutex TextsMutex;
    };
}