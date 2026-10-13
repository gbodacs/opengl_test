#include "example_animated_mesh.h"

AnimatedMesh3D CreateExampleAnimatedMesh()
{
    AnimatedMesh3D mesh;

    mesh.vertices = {
        { { -0.75f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
        { {  0.75f, 0.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
        { {  0.75f, 1.5f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
        { { -0.75f, 1.5f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } }
    };

    mesh.indices = { 0, 1, 2, 0, 2, 3 };
    mesh.skinVertices.resize(mesh.vertices.size());

    for (size_t index = 0; index < mesh.skinVertices.size(); ++index)
    {
        mesh.skinVertices[index].bindVertex = mesh.vertices[index];
        mesh.skinVertices[index].joints[0] = 0;
        mesh.skinVertices[index].weights[0] = 1.0f;
    }

    mesh.joints.resize(1);
    mesh.joints[0].translation = { 0.0f, 0.0f, 0.0f };
    mesh.joints[0].rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    mesh.joints[0].scale = { 1.0f, 1.0f, 1.0f };

    AnimationClip3D clip;
    clip.name = "Wave";
    clip.duration = 2.0f;

    AnimationChannel3D channel;
    channel.target = AnimationTarget3D::Translation;
    channel.jointIndex = 0;
    channel.sampler.interpolation = AnimationInterpolation3D::Linear;
    channel.sampler.times = { 0.0f, 0.5f, 1.0f, 1.5f, 2.0f };
    channel.sampler.values = {
        { -0.5f, 0.0f, 0.0f, 1.0f },
        {  0.2f, 0.0f, 0.0f, 1.0f },
        {  0.8f, 0.0f, 0.0f, 1.0f },
        {  0.2f, 0.0f, 0.0f, 1.0f },
        { -0.5f, 0.0f, 0.0f, 1.0f }
    };
    clip.channels.push_back(channel);
    mesh.clips.push_back(clip);

    return mesh;
}
