#pragma once

#include "Physics/Defines.h"
#include "Core/TypeAlias.h"

#include "Maths/Vector2.h"
#include "Maths/Vector3.h"
#include "Maths/Vector4.h"

#include <string>
#include <mutex>
#include <vector>

namespace Insight::Physics
{
    struct IS_PHYSICS DebugRendererData
    {
        DebugRendererData() = default;

        DebugRendererData(const DebugRendererData& other)
        {
            std::lock_guard lock(Mutex);

            Vertices = std::move(other.Vertices);
            Indices = std::move(other.Indices);

            Lines = other.Lines;

            Triangles = std::move(other.Triangles);

            Texts = other.Texts;
        }
        DebugRendererData(DebugRendererData&& other)
        {
            std::lock_guard lock(Mutex);

            Vertices = std::move(other.Vertices);
            other.Vertices.clear();
            Indices = std::move(other.Indices);
            other.Indices.clear();

            Lines = std::move(other.Lines);
            other.Lines.clear();

            Triangles = std::move(other.Triangles);
            other.Triangles.clear();

            Texts = std::move(other.Texts);
            other.Texts.clear();
        }

        DebugRendererData& operator=(const DebugRendererData& other)
        {
            std::lock_guard lock(Mutex);

            Vertices = std::move(other.Vertices);
            Indices = std::move(other.Indices);

            Lines = other.Lines;

            Triangles = std::move(other.Triangles);

            Texts = other.Texts;

            return *this;
        }
        DebugRendererData& operator=(DebugRendererData&& other)
        {
            std::lock_guard lock(Mutex);

            Vertices = std::move(other.Vertices);
            other.Vertices.clear();
            Indices = std::move(other.Indices);
            other.Indices.clear();

            Lines = std::move(other.Lines);
            other.Lines.clear();

            Triangles = std::move(other.Triangles);
            other.Triangles.clear();

            Texts = std::move(other.Texts);
            other.Texts.clear();

            return *this;
        }

        struct IS_PHYSICS Vertex
        {
            Maths::Vector4 Position;
            Maths::Vector4 Colour;
            Maths::Vector2 UV;
        };

        struct IS_PHYSICS Line
        {
            u64 StartIndex = 0;
            u64 Size = 0;
        };

        struct IS_PHYSICS Triangle
        {
            u64 VertexStartIndex = 0;
            u64 IndexStartIndex = 0;
            u64 IndexCount = 0;
        };

        struct IS_PHYSICS Text
        {
            Maths::Vector4 Position;
            std::string Text;
            Maths::Vector4 Colour;
            float Height;
        };

        mutable std::mutex Mutex;

        std::vector<Vertex> Vertices;
        std::vector<u16> Indices;

        std::vector<Line> Lines;
        std::vector<Triangle> Triangles;
        std::vector<Text> Texts;

    };
}