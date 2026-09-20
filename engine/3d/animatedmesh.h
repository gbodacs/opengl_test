#pragma once

#include "mesh.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

enum class AnimationInterpolation3D
{
    Step,
    Linear
};

enum class AnimationTarget3D
{
    Translation,
    Rotation,
    Scale
};

struct SkinVertex3D
{
    MeshVertex bindVertex;
    std::array<std::uint16_t, 4> joints{};
    std::array<float, 4> weights{};
};

struct SkeletonJoint3D
{
    int parentIndex = -1;
    Vec3f3D translation{};
    Quaternion3D rotation{};
    Vec3f3D scale{ 1.0f, 1.0f, 1.0f };
    Mat4f3D inverseBindMatrix{};
};

struct AnimationSampler3D
{
    AnimationInterpolation3D interpolation = AnimationInterpolation3D::Linear;
    std::vector<float> times;
    std::vector<Vec4f3D> values;
};

struct AnimationChannel3D
{
    AnimationTarget3D target = AnimationTarget3D::Translation;
    std::uint16_t jointIndex = 0;
    AnimationSampler3D sampler;
};

struct AnimationClip3D
{
    std::string name;
    float duration = 0.0f;
    std::vector<AnimationChannel3D> channels;
};

class AnimatedMesh3D : public Mesh3D
{
public:
    std::vector<SkinVertex3D> skinVertices;
    std::vector<SkeletonJoint3D> joints;
    std::vector<AnimationClip3D> clips;

    bool PlayClip(const std::string& name, bool loop, float fadeSeconds = 0.0f)
    {
        const int targetClipIndex = FindClip(name);
        if (targetClipIndex < 0)
        {
            return false;
        }

        if (targetClipIndex == activeClipIndex)
        {
            activeLooping = loop;
            return true;
        }

        previousClipIndex = activeClipIndex;
        previousClipTime = activeClipTime;
        previousLooping = activeLooping;
        activeClipIndex = targetClipIndex;
        activeClipTime = 0.0f;
        activeLooping = loop;
        transitionDuration = fadeSeconds;
        transitionTime = 0.0f;

        if (fadeSeconds <= 0.0f || previousClipIndex < 0)
        {
            previousClipIndex = -1;
        }
        return true;
    }

    bool PlayClip(int clipIndex, bool loop, float fadeSeconds = 0.0f)
    {
        if (clipIndex < 0 || clipIndex >= static_cast<int>(clips.size()))
        {
            return false;
        }
        return PlayClip(clips[clipIndex].name, loop, fadeSeconds);
    }

    bool IsClipFinished() const
    {
        if (activeClipIndex < 0 || activeLooping)
        {
            return false;
        }
        return activeClipTime >= clips[activeClipIndex].duration;
    }

    int FindClip(const std::string& name) const
    {
        for (size_t index = 0; index < clips.size(); ++index)
        {
            if (clips[index].name == name)
            {
                return static_cast<int>(index);
            }
        }
        return -1;
    }

    bool IsValidSkin() const
    {
        if (skinVertices.empty() || joints.empty() || skinVertices.size() != vertices.size())
        {
            return false;
        }

        for (size_t jointIndex = 0; jointIndex < joints.size(); ++jointIndex)
        {
            const int parentIndex = joints[jointIndex].parentIndex;
            if (parentIndex >= static_cast<int>(jointIndex) || parentIndex < -1)
            {
                return false;
            }
        }

        for (const SkinVertex3D& vertex : skinVertices)
        {
            float weightTotal = 0.0f;
            for (size_t influenceIndex = 0; influenceIndex < vertex.weights.size(); ++influenceIndex)
            {
                if (vertex.weights[influenceIndex] < 0.0f ||
                    vertex.joints[influenceIndex] >= joints.size())
                {
                    return false;
                }
                weightTotal += vertex.weights[influenceIndex];
            }
            if (weightTotal <= 0.0f)
            {
                return false;
            }
        }
        return true;
    }

    void Update(float deltaSeconds) override
    {
        if (!IsValidSkin() || activeClipIndex < 0)
        {
            return;
        }

        activeClipTime = AdvanceTime(activeClipTime, clips[activeClipIndex].duration, activeLooping, deltaSeconds);
        if (previousClipIndex >= 0)
        {
            previousClipTime = AdvanceTime(
                previousClipTime, clips[previousClipIndex].duration, previousLooping, deltaSeconds);
            transitionTime += deltaSeconds;
        }

        std::vector<JointPose> pose = EvaluatePose(activeClipIndex, activeClipTime);
        if (previousClipIndex >= 0)
        {
            const float blend = std::min(transitionTime / transitionDuration, 1.0f);
            pose = BlendPoses(EvaluatePose(previousClipIndex, previousClipTime), pose, blend);
            if (blend >= 1.0f)
            {
                previousClipIndex = -1;
            }
        }

        SkinPose(pose);
    }

private:
    struct JointPose
    {
        Vec3f3D translation{};
        Quaternion3D rotation{};
        Vec3f3D scale{ 1.0f, 1.0f, 1.0f };
    };

    int activeClipIndex = -1;
    int previousClipIndex = -1;
    float activeClipTime = 0.0f;
    float previousClipTime = 0.0f;
    float transitionTime = 0.0f;
    float transitionDuration = 0.0f;
    bool activeLooping = true;
    bool previousLooping = true;

    static float AdvanceTime(float time, float duration, bool loop, float deltaSeconds)
    {
        if (duration <= 0.0f)
        {
            return 0.0f;
        }

        time += std::max(0.0f, deltaSeconds);
        if (loop)
        {
            return std::fmod(time, duration);
        }
        return std::min(time, duration);
    }

    static Vec4f3D Sample(const AnimationSampler3D& sampler, float time)
    {
        if (sampler.times.empty() || sampler.times.size() != sampler.values.size())
        {
            return {};
        }
        if (time <= sampler.times.front())
        {
            return sampler.values.front();
        }
        if (time >= sampler.times.back())
        {
            return sampler.values.back();
        }

        for (size_t index = 1; index < sampler.times.size(); ++index)
        {
            if (time > sampler.times[index])
            {
                continue;
            }

            if (sampler.interpolation == AnimationInterpolation3D::Step)
            {
                return sampler.values[index - 1];
            }

            const float interval = sampler.times[index] - sampler.times[index - 1];
            const float amount = interval > 0.0f ? (time - sampler.times[index - 1]) / interval : 0.0f;
            const Vec4f3D& from = sampler.values[index - 1];
            const Vec4f3D& to = sampler.values[index];
            return {
                from.x + (to.x - from.x) * amount,
                from.y + (to.y - from.y) * amount,
                from.z + (to.z - from.z) * amount,
                from.w + (to.w - from.w) * amount
            };
        }
        return sampler.values.back();
    }

    std::vector<JointPose> EvaluatePose(int clipIndex, float time) const
    {
        std::vector<JointPose> pose(joints.size());
        for (size_t index = 0; index < joints.size(); ++index)
        {
            pose[index] = { joints[index].translation, joints[index].rotation, joints[index].scale };
        }

        const AnimationClip3D& clip = clips[clipIndex];
        for (const AnimationChannel3D& channel : clip.channels)
        {
            if (channel.jointIndex >= pose.size())
            {
                continue;
            }

            const Vec4f3D value = Sample(channel.sampler, time);
            switch (channel.target)
            {
                case AnimationTarget3D::Translation:
                    pose[channel.jointIndex].translation = { value.x, value.y, value.z };
                    break;
                case AnimationTarget3D::Rotation:
                    pose[channel.jointIndex].rotation = NormalizeQuaternion({ value.x, value.y, value.z, value.w });
                    break;
                case AnimationTarget3D::Scale:
                    pose[channel.jointIndex].scale = { value.x, value.y, value.z };
                    break;
            }
        }
        return pose;
    }

    static std::vector<JointPose> BlendPoses(
        const std::vector<JointPose>& from, const std::vector<JointPose>& to, float amount)
    {
        std::vector<JointPose> result = to;
        const size_t count = std::min(from.size(), to.size());
        for (size_t index = 0; index < count; ++index)
        {
            result[index].translation = LerpVec3(from[index].translation, to[index].translation, amount);
            result[index].rotation = NlerpQuaternion(from[index].rotation, to[index].rotation, amount);
            result[index].scale = LerpVec3(from[index].scale, to[index].scale, amount);
        }
        return result;
    }

    void SkinPose(const std::vector<JointPose>& pose)
    {
        std::vector<Mat4f3D> globalMatrices(joints.size());
        std::vector<Mat4f3D> skinMatrices(joints.size());
        for (size_t jointIndex = 0; jointIndex < joints.size(); ++jointIndex)
        {
            const Mat4f3D local = ComposeTRS(
                pose[jointIndex].translation, pose[jointIndex].rotation, pose[jointIndex].scale);
            globalMatrices[jointIndex] = joints[jointIndex].parentIndex < 0
                ? local
                : MultiplyMat4(globalMatrices[joints[jointIndex].parentIndex], local);
            skinMatrices[jointIndex] = MultiplyMat4(globalMatrices[jointIndex], joints[jointIndex].inverseBindMatrix);
        }

        for (size_t vertexIndex = 0; vertexIndex < skinVertices.size(); ++vertexIndex)
        {
            const SkinVertex3D& source = skinVertices[vertexIndex];
            MeshVertex& destination = vertices[vertexIndex];
            destination.position = {};
            destination.normal = {};

            float weightTotal = 0.0f;
            for (size_t influenceIndex = 0; influenceIndex < source.weights.size(); ++influenceIndex)
            {
                const float weight = source.weights[influenceIndex];
                if (weight <= 0.0f)
                {
                    continue;
                }
                const Mat4f3D& skinMatrix = skinMatrices[source.joints[influenceIndex]];
                destination.position = AddVec3(destination.position, ScaleVec3(TransformPoint(skinMatrix, source.bindVertex.position), weight));
                destination.normal = AddVec3(destination.normal, ScaleVec3(TransformVector(skinMatrix, source.bindVertex.normal), weight));
                weightTotal += weight;
            }

            if (weightTotal > 0.0f && weightTotal != 1.0f)
            {
                destination.position = ScaleVec3(destination.position, 1.0f / weightTotal);
                destination.normal = ScaleVec3(destination.normal, 1.0f / weightTotal);
            }
            destination.normal = NormalizeVec3(destination.normal);
            destination.uv = source.bindVertex.uv;
        }
    }
};