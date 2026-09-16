#pragma once

#include "mesh.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace objloader
{
    struct FaceVertexKey
    {
        int positionIndex = 0;
        int texCoordIndex = 0;
        int normalIndex = 0;
    };

    inline std::string Trim(const std::string& value)
    {
        const std::string whitespace = " \t\r\n";
        const size_t start = value.find_first_not_of(whitespace);
        if (start == std::string::npos)
        {
            return "";
        }

        const size_t end = value.find_last_not_of(whitespace);
        return value.substr(start, end - start + 1);
    }

    inline int ResolveIndex(int index, size_t count)
    {
        if (index > 0)
        {
            return index - 1;
        }

        if (index < 0)
        {
            return static_cast<int>(count) + index;
        }

        return -1;
    }

    inline bool ParseFaceVertex(const std::string& token, FaceVertexKey& faceVertex)
    {
        size_t firstSlash = token.find('/');
        if (firstSlash == std::string::npos)
        {
            faceVertex.positionIndex = std::stoi(token);
            return true;
        }

        const std::string positionPart = token.substr(0, firstSlash);
        size_t secondSlash = token.find('/', firstSlash + 1);
        const std::string texCoordPart = (secondSlash == std::string::npos)
            ? token.substr(firstSlash + 1)
            : token.substr(firstSlash + 1, secondSlash - firstSlash - 1);
        const std::string normalPart = (secondSlash == std::string::npos)
            ? ""
            : token.substr(secondSlash + 1);

        if (positionPart.empty())
        {
            return false;
        }

        faceVertex.positionIndex = std::stoi(positionPart);
        if (!texCoordPart.empty())
        {
            faceVertex.texCoordIndex = std::stoi(texCoordPart);
        }

        if (!normalPart.empty())
        {
            faceVertex.normalIndex = std::stoi(normalPart);
        }

        return true;
    }

    inline bool LoadMaterialTextures(
        const std::filesystem::path& materialPath,
        std::unordered_map<std::string, std::string>& materialTextures,
        std::string& firstTexturePath)
    {
        std::ifstream materialFile(materialPath);
        if (!materialFile.is_open())
        {
            return false;
        }

        std::string currentMaterial;
        std::string line;
        while (std::getline(materialFile, line))
        {
            line = Trim(line);
            if (line.empty() || line[0] == '#')
            {
                continue;
            }

            std::istringstream stream(line);
            std::string prefix;
            stream >> prefix;

            if (prefix == "newmtl")
            {
                stream >> currentMaterial;
                continue;
            }

            if (prefix == "map_Kd")
            {
                std::string texturePath;
                std::getline(stream, texturePath);
                texturePath = Trim(texturePath);
                if (texturePath.empty())
                {
                    continue;
                }

                const std::filesystem::path resolvedPath = materialPath.parent_path() / texturePath;
                if (firstTexturePath.empty())
                {
                    firstTexturePath = resolvedPath.string();
                }

                if (!currentMaterial.empty())
                {
                    materialTextures[currentMaterial] = resolvedPath.string();
                }
            }
        }

        return true;
    }
}

inline bool LoadOBJMesh(const std::string& filename, Mesh3D& mesh, std::string* diffuseTexturePath = nullptr)
{
    mesh.vertices.clear();
    mesh.indices.clear();

    const std::filesystem::path objectPath(filename);
    std::ifstream objectFile(objectPath);
    if (!objectFile.is_open())
    {
        return false;
    }

    std::vector<Vec3f3D> positions;
    std::vector<Vec2f3D> texCoords;
    std::vector<Vec3f3D> normals;
    std::unordered_map<std::string, unsigned int> uniqueVertices;
    std::unordered_map<std::string, std::string> materialTextures;

    std::string materialLibrary;
    std::string selectedMaterial;
    std::string firstTexturePath;
    std::string line;

    while (std::getline(objectFile, line))
    {
        line = objloader::Trim(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::istringstream stream(line);
        std::string prefix;
        stream >> prefix;

        if (prefix == "v")
        {
            Vec3f3D position;
            stream >> position.x >> position.y >> position.z;
            positions.push_back(position);
            continue;
        }

        if (prefix == "vt")
        {
            Vec2f3D texCoord;
            stream >> texCoord.x >> texCoord.y;
            texCoords.push_back(texCoord);
            continue;
        }

        if (prefix == "vn")
        {
            Vec3f3D normal;
            stream >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
            continue;
        }

        if (prefix == "mtllib")
        {
            std::getline(stream, materialLibrary);
            materialLibrary = objloader::Trim(materialLibrary);
            continue;
        }

        if (prefix == "usemtl")
        {
            stream >> selectedMaterial;
            continue;
        }

        if (prefix != "f")
        {
            continue;
        }

        std::vector<unsigned int> faceIndices;
        std::string token;
        while (stream >> token)
        {
            objloader::FaceVertexKey faceVertex;
            if (!objloader::ParseFaceVertex(token, faceVertex))
            {
                return false;
            }

            const int positionIndex = objloader::ResolveIndex(faceVertex.positionIndex, positions.size());
            const int texCoordIndex = objloader::ResolveIndex(faceVertex.texCoordIndex, texCoords.size());
            const int normalIndex = objloader::ResolveIndex(faceVertex.normalIndex, normals.size());

            if (positionIndex < 0 || positionIndex >= static_cast<int>(positions.size()))
            {
                return false;
            }

            const std::string vertexKey = std::to_string(positionIndex) + "/" +
                std::to_string(texCoordIndex) + "/" + std::to_string(normalIndex);

            auto it = uniqueVertices.find(vertexKey);
            if (it == uniqueVertices.end())
            {
                MeshVertex vertex;
                vertex.position = positions[positionIndex];
                if (texCoordIndex >= 0 && texCoordIndex < static_cast<int>(texCoords.size()))
                {
                    vertex.uv = texCoords[texCoordIndex];
                }
                if (normalIndex >= 0 && normalIndex < static_cast<int>(normals.size()))
                {
                    vertex.normal = normals[normalIndex];
                }
                else
                {
                    vertex.normal = { 0.0f, 0.0f, 1.0f };
                }

                const unsigned int newIndex = static_cast<unsigned int>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);
                uniqueVertices[vertexKey] = newIndex;
                faceIndices.push_back(newIndex);
            }
            else
            {
                faceIndices.push_back(it->second);
            }
        }

        if (faceIndices.size() < 3)
        {
            continue;
        }

        for (size_t vertexIndex = 1; vertexIndex + 1 < faceIndices.size(); ++vertexIndex)
        {
            mesh.indices.push_back(faceIndices[0]);
            mesh.indices.push_back(faceIndices[vertexIndex]);
            mesh.indices.push_back(faceIndices[vertexIndex + 1]);
        }
    }

    if (!materialLibrary.empty() && diffuseTexturePath != nullptr)
    {
        const std::filesystem::path materialPath = objectPath.parent_path() / materialLibrary;
        objloader::LoadMaterialTextures(materialPath, materialTextures, firstTexturePath);

        auto selectedTexture = materialTextures.find(selectedMaterial);
        if (selectedTexture != materialTextures.end())
        {
            *diffuseTexturePath = selectedTexture->second;
        }
        else
        {
            *diffuseTexturePath = firstTexturePath;
        }
    }

    return mesh.IsValid();
}