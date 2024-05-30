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
            LinesDraw = other.LinesDraw;

            Texts = other.Texts;
        }
        DebugRendererData(DebugRendererData&& other)
        {
            std::lock_guard linesLock(LinesMutex);
            std::lock_guard otherLinesLock(other.LinesMutex);

            std::lock_guard textLock(TextsMutex);
            std::lock_guard otherTextLock(other.TextsMutex);

            Lines = std::move(other.Lines);
            LinesDraw = std::move(other.LinesDraw);
            other.Lines.clear();
            other.LinesDraw.clear();

            Texts = std::move(other.Texts);
            other.Texts.clear();
        }

        DebugRendererData& operator=(const DebugRendererData& other)
        {
            std::lock_guard linesLock(LinesMutex);
            std::lock_guard otherLinesLock(other.LinesMutex);

            std::lock_guard textLock(TextsMutex);
            std::lock_guard otherTextLock(other.TextsMutex);

            Lines = other.Lines;
            LinesDraw = other.LinesDraw;

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
            LinesDraw = std::move(other.LinesDraw);
            other.Lines.clear();
            other.LinesDraw.clear();

            Texts = std::move(other.Texts);
            other.Texts.clear();

            return *this;
        }

        struct IS_PHYSICS Line
        {
            Maths::Vector4 From;
            Maths::Vector4 FromColour;
            Maths::Vector4 To;
            Maths::Vector4 ToColour;
        };
        struct IS_PHYSICS LineDraw
        {
            u64 StartIndex = 0;
            u64 Size = 0;
        };

        struct IS_PHYSICS Text
        {
            Maths::Vector4 Position;
            std::string Text;
            Maths::Vector4 Colour;
            float Height;
        };

        std::vector<Line> Lines;
        std::vector<LineDraw> LinesDraw;
        mutable std::mutex LinesMutex;
    
        std::vector<Text> Texts;
        mutable std::mutex TextsMutex;

    };
}