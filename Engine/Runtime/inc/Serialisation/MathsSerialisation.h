#pragma once

#if 1

#include "Serialisation/PropertySerialiser.h"

#include "Maths/Vector2.h"
#include "Maths/Vector3.h"
#include "Maths/Vector4.h"


#include "Maths/Matrix2.h"
#include "Maths/Matrix3.h"
#include "Maths/Matrix4.h"

#include <string>

namespace Insight
{
    namespace Serialisation
    {
#pragma region PropertySerialiser
        //================================================================
        // PropertySerialiser
        //================================================================
        template<>
        struct PropertySerialiser<Maths::Vector2>
        {
            std::string operator()(Maths::Vector2 const& v)
            {
                std::string str;
                str += "(" + std::to_string(v.x);
                str += ",";
                str += std::to_string(v.y) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<Maths::Vector3>
        {
            std::string operator()(Maths::Vector3 const& v)
            {
                std::string str;
                str += "(" + std::to_string(v.x);
                str += ",";
                str += std::to_string(v.y);
                str += ",";
                str += std::to_string(v.z) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<Maths::Vector4>
        {
            std::string operator()(Maths::Vector4 const& v)
            {
                std::string str;
                str += "(" + std::to_string(v.x);
                str += ",";
                str += std::to_string(v.y);
                str += ",";
                str += std::to_string(v.z);
                str += ",";
                str += std::to_string(v.w) + ")";
                return str;
            }
        };

        template<>
        struct PropertySerialiser<Maths::Matrix2>
        {
            std::string operator()(const Maths::Matrix2& mat)
            {
                PropertySerialiser<Maths::Vector2> vecSerialiser;
                std::string str;
                str += vecSerialiser(mat[0]);
                str += vecSerialiser(mat[1]);
                return str;
            }
        };
        template<>
        struct PropertySerialiser<Maths::Matrix3>
        {
            std::string operator()(const Maths::Matrix3& mat)
            {
                PropertySerialiser<Maths::Vector3> vecSerialiser;
                std::string str;
                str += vecSerialiser(mat[0]);
                str += vecSerialiser(mat[1]);
                str += vecSerialiser(mat[2]);
                return str;
            }
        };
        template<>
        struct PropertySerialiser<Maths::Matrix4>
        {
            std::string operator()(const Maths::Matrix4& mat)
            {
                PropertySerialiser<Maths::Vector4> vecSerialiser;
                std::string str;
                str += vecSerialiser(mat[0]);
                str += vecSerialiser(mat[1]);
                str += vecSerialiser(mat[2]);
                str += vecSerialiser(mat[3]);
                return str;
            }
        };
#pragma endregion

#pragma region PropertyDeserialiser
        //================================================================
        // PropertyDeserialiser
        //================================================================
        static Maths::Vector4 GetVectorComponents(const std::string& data)
        {
            Maths::Vector4 vec;
            int idx = 0;
            std::string numString;
            for (size_t i = 0; i < data.size(); ++i)
            {
                const char c = data.at(i);

                if (c == '(' || c == ')')
                {
                    continue;
                }
                else if (c == ',')
                {
                    vec[idx] = std::stof(numString);
                    numString.clear();
                    ++idx;
                }
                else if (c >= '0' && c <= '9'
                    || c == '.' || c == '-')
                {
                    numString += c;
                }
                else
                {
                    assert(false && "Ill-formatted string.");
                }
            }
            vec[idx] = std::stof(numString);
            return vec;
        }

        template<>
        struct PropertyDeserialiser<Maths::Vector2>
        {
            using InType = std::string;
            Maths::Vector2 operator()(std::string const& data) const
            {
                Maths::Vector4 vec = GetVectorComponents(data);
                return Maths::Vector2(vec[0], vec[1]);
            }
        };
        template<>
        struct PropertyDeserialiser<Maths::Vector3>
        {
            using InType = std::string;
            Maths::Vector3 operator()(std::string const& data) const
            {
                Maths::Vector4 vec = GetVectorComponents(data);
                return Maths::Vector3(vec[0], vec[1], vec[2]);
            }
        };
        template<>
        struct PropertyDeserialiser<Maths::Vector4>
        {
            using InType = std::string;
            Maths::Vector4 operator()(std::string const& data) const
            {
                return GetVectorComponents(data);
            }
        };

        static Maths::Matrix4 GetMatrixComponents(const std::string& data)
        {
            PropertyDeserialiser<Maths::Vector4> vecDeserialiser;
            Maths::Matrix4 mat;

            int idx = 0;
            std::string numString;

            for (size_t i = 0; i < data.size(); ++i)
            {
                if (data.at(i) == ')')
                {
                    mat[idx] = vecDeserialiser(numString);
                    numString.clear();
                    ++idx;
                }
                else if (data.at(i) != '(' && data.at(i) != ')')
                {
                    numString += data.at(i);
                }
            }
            return mat;
        }

        template<>
        struct PropertyDeserialiser<Maths::Matrix2>
        {
            using InType = std::string;
            Maths::Matrix2 operator()(std::string const& data) const
            {
                Maths::Matrix4 mat = GetMatrixComponents(data);
                return Maths::Matrix2(
                    mat[0][0], mat[0][1],
                    mat[1][0], mat[1][1]);
            }
        };
        template<>
        struct PropertyDeserialiser<Maths::Matrix3>
        {
            using InType = std::string;
            Maths::Matrix3 operator()(std::string const& data) const
            {
                Maths::Matrix4 mat = GetMatrixComponents(data);
                return Maths::Matrix3(
                    mat[0][0], mat[0][1], mat[0][2],
                    mat[1][0], mat[1][1], mat[1][2], 
                    mat[2][0], mat[2][1], mat[2][2]);
            }
        };
        template<>
        struct PropertyDeserialiser<Maths::Matrix4>
        {
            using InType = std::string;
            Maths::Matrix4 operator()(std::string const& data) const
            {
                return GetMatrixComponents(data);
            }
        };
#pragma endregion
    }
}

#endif