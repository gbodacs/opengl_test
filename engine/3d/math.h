#pragma once

#include <cmath>

struct Vec2f3D
{
    float x = 0.0f;
    float y = 0.0f;
};

struct Vec3f3D
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Vec4f3D
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

struct Quaternion3D
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

struct Mat4f3D
{
    float values[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    static Mat4f3D Identity()
    {
        return {};
    }
};

inline Vec3f3D AddVec3(const Vec3f3D& left, const Vec3f3D& right)
{
    return { left.x + right.x, left.y + right.y, left.z + right.z };
}

inline Vec3f3D SubtractVec3(const Vec3f3D& left, const Vec3f3D& right)
{
    return { left.x - right.x, left.y - right.y, left.z - right.z };
}

inline Vec3f3D ScaleVec3(const Vec3f3D& value, float scale)
{
    return { value.x * scale, value.y * scale, value.z * scale };
}

inline float DotVec3(const Vec3f3D& left, const Vec3f3D& right)
{
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

inline Vec3f3D NormalizeVec3(const Vec3f3D& value)
{
    const float lengthSquared = DotVec3(value, value);
    if (lengthSquared <= 0.0f)
    {
        return {};
    }

    return ScaleVec3(value, 1.0f / std::sqrt(lengthSquared));
}

inline Vec3f3D LerpVec3(const Vec3f3D& from, const Vec3f3D& to, float amount)
{
    return AddVec3(from, ScaleVec3(SubtractVec3(to, from), amount));
}

inline Quaternion3D NormalizeQuaternion(const Quaternion3D& value)
{
    const float lengthSquared = value.x * value.x + value.y * value.y +
        value.z * value.z + value.w * value.w;
    if (lengthSquared <= 0.0f)
    {
        return {};
    }

    const float inverseLength = 1.0f / std::sqrt(lengthSquared);
    return {
        value.x * inverseLength,
        value.y * inverseLength,
        value.z * inverseLength,
        value.w * inverseLength
    };
}

inline Quaternion3D NlerpQuaternion(Quaternion3D from, Quaternion3D to, float amount)
{
    const float dot = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
    if (dot < 0.0f)
    {
        to.x = -to.x;
        to.y = -to.y;
        to.z = -to.z;
        to.w = -to.w;
    }

    return NormalizeQuaternion({
        from.x + (to.x - from.x) * amount,
        from.y + (to.y - from.y) * amount,
        from.z + (to.z - from.z) * amount,
        from.w + (to.w - from.w) * amount
    });
}

inline Mat4f3D MultiplyMat4(const Mat4f3D& left, const Mat4f3D& right)
{
    Mat4f3D result{};
    for (int column = 0; column < 4; ++column)
    {
        for (int row = 0; row < 4; ++row)
        {
            result.values[column * 4 + row] = 0.0f;
            for (int index = 0; index < 4; ++index)
            {
                result.values[column * 4 + row] +=
                    left.values[index * 4 + row] * right.values[column * 4 + index];
            }
        }
    }
    return result;
}

inline Mat4f3D ComposeTRS(const Vec3f3D& translation, const Quaternion3D& rotation, const Vec3f3D& scale)
{
    const Quaternion3D normalized = NormalizeQuaternion(rotation);
    const float xx = normalized.x * normalized.x;
    const float yy = normalized.y * normalized.y;
    const float zz = normalized.z * normalized.z;
    const float xy = normalized.x * normalized.y;
    const float xz = normalized.x * normalized.z;
    const float yz = normalized.y * normalized.z;
    const float wx = normalized.w * normalized.x;
    const float wy = normalized.w * normalized.y;
    const float wz = normalized.w * normalized.z;

    Mat4f3D result{};
    result.values[0] = (1.0f - 2.0f * (yy + zz)) * scale.x;
    result.values[1] = 2.0f * (xy + wz) * scale.x;
    result.values[2] = 2.0f * (xz - wy) * scale.x;
    result.values[4] = 2.0f * (xy - wz) * scale.y;
    result.values[5] = (1.0f - 2.0f * (xx + zz)) * scale.y;
    result.values[6] = 2.0f * (yz + wx) * scale.y;
    result.values[8] = 2.0f * (xz + wy) * scale.z;
    result.values[9] = 2.0f * (yz - wx) * scale.z;
    result.values[10] = (1.0f - 2.0f * (xx + yy)) * scale.z;
    result.values[12] = translation.x;
    result.values[13] = translation.y;
    result.values[14] = translation.z;
    return result;
}

inline Vec3f3D TransformPoint(const Mat4f3D& matrix, const Vec3f3D& point)
{
    return {
        matrix.values[0] * point.x + matrix.values[4] * point.y + matrix.values[8] * point.z + matrix.values[12],
        matrix.values[1] * point.x + matrix.values[5] * point.y + matrix.values[9] * point.z + matrix.values[13],
        matrix.values[2] * point.x + matrix.values[6] * point.y + matrix.values[10] * point.z + matrix.values[14]
    };
}

inline Vec3f3D TransformVector(const Mat4f3D& matrix, const Vec3f3D& vector)
{
    return {
        matrix.values[0] * vector.x + matrix.values[4] * vector.y + matrix.values[8] * vector.z,
        matrix.values[1] * vector.x + matrix.values[5] * vector.y + matrix.values[9] * vector.z,
        matrix.values[2] * vector.x + matrix.values[6] * vector.y + matrix.values[10] * vector.z
    };
}