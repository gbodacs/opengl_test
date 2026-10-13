#include "animatedmesh.h"
#include "example_animated_mesh.h"

#include <cassert>
#include <cmath>

namespace
{
    bool NearlyEqual(float left, float right)
    {
        return std::fabs(left - right) < 0.0001f;
    }

    AnimationClip3D MakeTranslationClip(const char* name, float start, float end,
                                        AnimationInterpolation3D interpolation = AnimationInterpolation3D::Linear)
    {
        AnimationChannel3D channel;
        channel.target = AnimationTarget3D::Translation;
        channel.jointIndex = 0;
        channel.sampler.interpolation = interpolation;
        channel.sampler.times = { 0.0f, 1.0f };
        channel.sampler.values = { { start, 0.0f, 0.0f, 1.0f }, { end, 0.0f, 0.0f, 1.0f } };

        AnimationClip3D clip;
        clip.name = name;
        clip.duration = 1.0f;
        clip.channels.push_back(channel);
        return clip;
    }

    AnimatedMesh3D MakeAnimatedMesh()
    {
        AnimatedMesh3D mesh;
        mesh.vertices.resize(1);
        mesh.skinVertices.resize(1);
        mesh.skinVertices[0].bindVertex = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
        mesh.skinVertices[0].weights[0] = 1.0f;
        mesh.vertices[0] = mesh.skinVertices[0].bindVertex;
        mesh.indices = { 0, 0, 0 };
        mesh.joints.resize(1);
        mesh.clips.push_back(MakeTranslationClip("Move", 0.0f, 2.0f));
        mesh.clips.push_back(MakeTranslationClip("Step", 4.0f, 8.0f, AnimationInterpolation3D::Step));
        mesh.clips.push_back(MakeTranslationClip("Hold", 4.0f, 4.0f));
        return mesh;
    }
}

int main()
{
    Mesh3D staticMesh;
    staticMesh.Update(1.0f);

    AnimatedMesh3D mesh = MakeAnimatedMesh();
    assert(mesh.IsValidSkin());
    assert(mesh.PlayClip("Move", false));
    mesh.Update(0.5f);
    assert(NearlyEqual(mesh.vertices[0].position.x, 1.0f));
    assert(!mesh.IsClipFinished());

    AnimatedMesh3D example = CreateExampleAnimatedMesh();
    assert(example.IsValidSkin());
    assert(example.PlayClip("Wave", true));
    example.Update(0.25f);
    assert(example.vertices[0].position.x >= -1.0f);

    mesh.Update(1.0f);
    assert(NearlyEqual(mesh.vertices[0].position.x, 2.0f));
    assert(mesh.IsClipFinished());

    assert(mesh.PlayClip("Step", false));
    mesh.Update(0.5f);
    assert(NearlyEqual(mesh.vertices[0].position.x, 4.0f));

    assert(mesh.PlayClip("Move", true));
    mesh.Update(1.25f);
    assert(NearlyEqual(mesh.vertices[0].position.x, 0.5f));

    assert(mesh.PlayClip("Hold", true, 1.0f));
    mesh.Update(0.5f);
    assert(NearlyEqual(mesh.vertices[0].position.x, 2.75f));
    mesh.Update(0.5f);
    assert(NearlyEqual(mesh.vertices[0].position.x, 4.0f));
    return 0;
}