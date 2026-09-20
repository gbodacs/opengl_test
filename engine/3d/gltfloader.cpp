#define CGLTF_IMPLEMENTATION
#include "../../third_party/cgltf/cgltf.h"

#include "gltfloader.h"

#include <algorithm>
#include <filesystem>
#include <unordered_map>

namespace
{
    void SetError(std::string* error, const std::string& message)
    {
        if (error != nullptr)
        {
            *error = message;
        }
    }

    const cgltf_attribute* FindAttribute(const cgltf_primitive& primitive, cgltf_attribute_type type)
    {
        for (cgltf_size index = 0; index < primitive.attributes_count; ++index)
        {
            if (primitive.attributes[index].type == type && primitive.attributes[index].index == 0)
            {
                return &primitive.attributes[index];
            }
        }
        return nullptr;
    }

    bool ReadVec(const cgltf_accessor* accessor, cgltf_size index, float* values, cgltf_size count)
    {
        return accessor != nullptr && cgltf_accessor_read_float(accessor, index, values, count) != 0;
    }

    bool LoadTexture(const cgltf_material* material, const std::filesystem::path& modelPath,
                     AnimatedMeshLoadResult& result, std::string* error)
    {
        if (material == nullptr || material->has_pbr_metallic_roughness == 0)
        {
            return true;
        }

        const cgltf_texture_view& textureView = material->pbr_metallic_roughness.base_color_texture;
        if (textureView.texture == nullptr || textureView.texture->image == nullptr)
        {
            return true;
        }

        const cgltf_image* image = textureView.texture->image;
        if (image->buffer_view != nullptr)
        {
            const cgltf_buffer_view* view = image->buffer_view;
            if (view->buffer == nullptr || view->buffer->data == nullptr)
            {
                SetError(error, "Animated GLB contains an image without loaded buffer data.");
                return false;
            }
            const unsigned char* bytes = static_cast<const unsigned char*>(view->buffer->data) + view->offset;
            result.embeddedDiffuseImage.assign(bytes, bytes + view->size);
            return true;
        }

        if (image->uri != nullptr)
        {
            result.diffuseTexturePath = (modelPath.parent_path() / image->uri).lexically_normal().string();
            return true;
        }

        SetError(error, "Animated GLB base-color texture has no image data.");
        return false;
    }

    bool LoadClip(const cgltf_animation& source, const std::unordered_map<const cgltf_node*, std::uint16_t>& jointIndices,
                  AnimationClip3D& destination, std::string* error)
    {
        destination.name = source.name != nullptr ? source.name : "Animation";
        for (cgltf_size channelIndex = 0; channelIndex < source.channels_count; ++channelIndex)
        {
            const cgltf_animation_channel& sourceChannel = source.channels[channelIndex];
            if (sourceChannel.target_node == nullptr || sourceChannel.sampler == nullptr)
            {
                continue;
            }

            const auto joint = jointIndices.find(sourceChannel.target_node);
            if (joint == jointIndices.end())
            {
                continue;
            }

            const cgltf_animation_sampler& sourceSampler = *sourceChannel.sampler;
            if (sourceSampler.input == nullptr || sourceSampler.output == nullptr ||
                sourceSampler.input->count != sourceSampler.output->count)
            {
                SetError(error, "Animated GLB has an animation sampler with mismatched input/output data.");
                return false;
            }
            if (sourceSampler.interpolation == cgltf_interpolation_type_cubic_spline)
            {
                SetError(error, "Animated GLB uses CUBICSPLINE interpolation, which is not supported yet.");
                return false;
            }

            AnimationChannel3D channel;
            channel.jointIndex = joint->second;
            channel.sampler.interpolation = sourceSampler.interpolation == cgltf_interpolation_type_step
                ? AnimationInterpolation3D::Step : AnimationInterpolation3D::Linear;
            switch (sourceChannel.target_path)
            {
                case cgltf_animation_path_type_translation:
                    channel.target = AnimationTarget3D::Translation;
                    break;
                case cgltf_animation_path_type_rotation:
                    channel.target = AnimationTarget3D::Rotation;
                    break;
                case cgltf_animation_path_type_scale:
                    channel.target = AnimationTarget3D::Scale;
                    break;
                default:
                    continue;
            }

            channel.sampler.times.resize(sourceSampler.input->count);
            channel.sampler.values.resize(sourceSampler.output->count);
            for (cgltf_size keyIndex = 0; keyIndex < sourceSampler.input->count; ++keyIndex)
            {
                float time = 0.0f;
                float values[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                if (!ReadVec(sourceSampler.input, keyIndex, &time, 1) ||
                    !ReadVec(sourceSampler.output, keyIndex, values, 4))
                {
                    SetError(error, "Animated GLB contains unreadable animation key data.");
                    return false;
                }
                channel.sampler.times[keyIndex] = time;
                channel.sampler.values[keyIndex] = { values[0], values[1], values[2], values[3] };
                destination.duration = std::max(destination.duration, time);
            }
            destination.channels.push_back(std::move(channel));
        }
        return true;
    }
}

bool LoadGLTFAnimatedMesh(
    const std::string& filename,
    AnimatedMesh3D& mesh,
    AnimatedMeshLoadResult& result,
    std::string* error)
{
    cgltf_options options{};
    cgltf_data* data = nullptr;
    if (cgltf_parse_file(&options, filename.c_str(), &data) != cgltf_result_success || data == nullptr)
    {
        SetError(error, "Could not parse animated glTF file: " + filename);
        return false;
    }

    if (cgltf_load_buffers(&options, data, filename.c_str()) != cgltf_result_success ||
        cgltf_validate(data) != cgltf_result_success)
    {
        cgltf_free(data);
        SetError(error, "Animated glTF has missing buffers or is invalid: " + filename);
        return false;
    }

    const cgltf_node* skinnedNode = nullptr;
    for (cgltf_size index = 0; index < data->nodes_count; ++index)
    {
        if (data->nodes[index].mesh != nullptr && data->nodes[index].skin != nullptr)
        {
            skinnedNode = &data->nodes[index];
            break;
        }
    }
    if (skinnedNode == nullptr || skinnedNode->mesh->primitives_count != 1)
    {
        cgltf_free(data);
        SetError(error, "Animated glTF must contain exactly one skinned mesh with one primitive.");
        return false;
    }

    const cgltf_primitive& primitive = skinnedNode->mesh->primitives[0];
    const cgltf_attribute* positions = FindAttribute(primitive, cgltf_attribute_type_position);
    const cgltf_attribute* normals = FindAttribute(primitive, cgltf_attribute_type_normal);
    const cgltf_attribute* texCoords = FindAttribute(primitive, cgltf_attribute_type_texcoord);
    const cgltf_attribute* joints = FindAttribute(primitive, cgltf_attribute_type_joints);
    const cgltf_attribute* weights = FindAttribute(primitive, cgltf_attribute_type_weights);
    if (primitive.type != cgltf_primitive_type_triangles || primitive.indices == nullptr || positions == nullptr ||
        normals == nullptr || texCoords == nullptr || joints == nullptr || weights == nullptr ||
        positions->data->count > 65535)
    {
        cgltf_free(data);
        SetError(error, "Animated glTF requires indexed triangles with POSITION, NORMAL, TEXCOORD_0, JOINTS_0, WEIGHTS_0 and at most 65535 vertices.");
        return false;
    }

    mesh.vertices.resize(positions->data->count);
    mesh.skinVertices.resize(positions->data->count);
    for (cgltf_size vertexIndex = 0; vertexIndex < positions->data->count; ++vertexIndex)
    {
        float position[3]{};
        float normal[3]{};
        float uv[2]{};
        float vertexWeights[4]{};
        cgltf_uint vertexJoints[4]{};
        if (!ReadVec(positions->data, vertexIndex, position, 3) || !ReadVec(normals->data, vertexIndex, normal, 3) ||
            !ReadVec(texCoords->data, vertexIndex, uv, 2) || !ReadVec(weights->data, vertexIndex, vertexWeights, 4) ||
            cgltf_accessor_read_uint(joints->data, vertexIndex, vertexJoints, 4) == 0)
        {
            cgltf_free(data);
            SetError(error, "Animated glTF contains unreadable mesh attributes.");
            return false;
        }
        SkinVertex3D& destination = mesh.skinVertices[vertexIndex];
        destination.bindVertex = { { position[0], position[1], position[2] }, { uv[0], uv[1] }, { normal[0], normal[1], normal[2] } };
        for (size_t influenceIndex = 0; influenceIndex < destination.weights.size(); ++influenceIndex)
        {
            if (vertexJoints[influenceIndex] > 65535)
            {
                cgltf_free(data);
                SetError(error, "Animated glTF joint index exceeds engine limits.");
                return false;
            }
            destination.joints[influenceIndex] = static_cast<std::uint16_t>(vertexJoints[influenceIndex]);
            destination.weights[influenceIndex] = vertexWeights[influenceIndex];
        }
        mesh.vertices[vertexIndex] = destination.bindVertex;
    }

    mesh.indices.resize(primitive.indices->count);
    for (cgltf_size index = 0; index < primitive.indices->count; ++index)
    {
        const cgltf_size value = cgltf_accessor_read_index(primitive.indices, index);
        if (value >= mesh.vertices.size())
        {
            cgltf_free(data);
            SetError(error, "Animated glTF contains an out-of-range index.");
            return false;
        }
        mesh.indices[index] = static_cast<unsigned int>(value);
    }

    const cgltf_skin& skin = *skinnedNode->skin;
    if (skin.joints_count == 0 || skin.inverse_bind_matrices == nullptr ||
        skin.inverse_bind_matrices->count != skin.joints_count)
    {
        cgltf_free(data);
        SetError(error, "Animated glTF requires inverse bind matrices for every joint.");
        return false;
    }

    std::unordered_map<const cgltf_node*, std::uint16_t> jointIndices;
    mesh.joints.resize(skin.joints_count);
    for (cgltf_size index = 0; index < skin.joints_count; ++index)
    {
        const cgltf_node* node = skin.joints[index];
        if (node == nullptr || node->has_matrix)
        {
            cgltf_free(data);
            SetError(error, "Animated glTF joints must use TRS transforms, not matrix transforms.");
            return false;
        }
        jointIndices[node] = static_cast<std::uint16_t>(index);
    }

    for (cgltf_size index = 0; index < skin.joints_count; ++index)
    {
        const cgltf_node* node = skin.joints[index];
        SkeletonJoint3D& destination = mesh.joints[index];
        if (node->parent != nullptr)
        {
            const auto parent = jointIndices.find(node->parent);
            destination.parentIndex = parent == jointIndices.end() ? -1 : parent->second;
            if (destination.parentIndex >= static_cast<int>(index))
            {
                cgltf_free(data);
                SetError(error, "Animated glTF joints must be stored parent-first.");
                return false;
            }
        }
        if (node->has_translation)
            destination.translation = { node->translation[0], node->translation[1], node->translation[2] };
        if (node->has_rotation)
            destination.rotation = { node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3] };
        if (node->has_scale)
            destination.scale = { node->scale[0], node->scale[1], node->scale[2] };
        if (cgltf_accessor_read_float(skin.inverse_bind_matrices, index, destination.inverseBindMatrix.values, 16) == 0)
        {
            cgltf_free(data);
            SetError(error, "Animated glTF contains unreadable inverse bind matrices.");
            return false;
        }
    }

    mesh.clips.clear();
    for (cgltf_size index = 0; index < data->animations_count; ++index)
    {
        AnimationClip3D clip;
        if (!LoadClip(data->animations[index], jointIndices, clip, error))
        {
            cgltf_free(data);
            return false;
        }
        if (!clip.channels.empty())
        {
            if (clip.name == "Animation")
            {
                clip.name += std::to_string(index);
            }
            mesh.clips.push_back(std::move(clip));
        }
    }

    const bool loadedTexture = LoadTexture(primitive.material, std::filesystem::path(filename), result, error);
    cgltf_free(data);
    if (!loadedTexture || !mesh.IsValidSkin())
    {
        if (loadedTexture)
        {
            SetError(error, "Animated glTF skin data is incomplete or invalid.");
        }
        return false;
    }
    return true;
}