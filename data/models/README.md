# Animated 3D Assets

Static props use OBJ/MTL through `Engine::LoadMesh()`. Export animated characters as binary glTF 2.0 (`.glb`) and load them through `Engine::LoadAnimatedMesh()`.

Place the demo asset at `data/models/animated_character.glb`. The game starts its `Idle` clip when it exists; missing assets are reported but do not stop the rest of the demo.

## Current Loader Limits

- One node containing one skinned mesh primitive.
- Indexed triangles, at most 65,535 vertices.
- `POSITION`, `NORMAL`, `TEXCOORD_0`, `JOINTS_0`, and `WEIGHTS_0` attributes are required.
- Up to four normalized joint weights per vertex.
- One PBR base-color texture, embedded in the GLB or referenced by URI.
- TRS joint transforms only; matrix-authored joints, morph targets, multiple materials, and multiple primitives are not supported yet.
- Animation interpolation must be `LINEAR` or `STEP`; do not export `CUBICSPLINE` channels.
- Name clips explicitly, for example `Idle`, `Walk`, and `Attack`.

## 3ds Max

1. Skin the mesh with a normal bone hierarchy. Bake IK, constraints, and procedural controllers to transform keys.
2. Reset transforms on the mesh before skinning. Keep the skeleton and mesh in Y-up orientation.
3. Limit skinning to four influences per vertex and normalize weights. Collapse or remove unused modifiers.
4. Triangulate the mesh, export normals and UV channel 0, and use one PBR material with its base-color texture.
5. Use a maintained glTF exporter such as the Babylon glTF exporter. Export binary GLB with mesh, skin, animations, and embedded textures. Select linear or step animation interpolation.
6. Create distinct named clips, such as `Idle`, `Walk`, and `Attack`, then export each clip in the same GLB.

## Maya

1. Bind the mesh to a joint hierarchy. Bake animation layers, constraints, IK, and controllers to the joints.
2. Freeze mesh transforms before binding; avoid nonuniform joint scaling.
3. Set a maximum of four influences per vertex and normalize weights.
4. Triangulate, export normals and UV set 0, and use one PBR base-color texture.
5. Use a maintained glTF exporter such as the Babylon glTF exporter. Export a binary GLB with the skin, clips, and embedded textures.
6. Give clips stable names such as `Idle`, `Walk`, and `Attack`; export `LINEAR` or `STEP` keys, not cubic spline interpolation.

Before importing into the engine, open the GLB in Babylon Sandbox or Khronos glTF Sample Viewer. Confirm every clip plays and the texture appears before copying it beside this file.