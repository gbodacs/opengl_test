#pragma once

#include "animatedmesh.h"

#include <string>
#include <vector>

struct AnimatedMeshLoadResult
{
    std::string diffuseTexturePath;
    std::vector<unsigned char> embeddedDiffuseImage;
};

bool LoadGLTFAnimatedMesh(
    const std::string& filename,
    AnimatedMesh3D& mesh,
    AnimatedMeshLoadResult& result,
    std::string* error = nullptr);