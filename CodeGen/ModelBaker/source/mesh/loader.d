module mesh.loader;
import std.string;
import std.conv;
import std.container.array;

import gfm.math;
import bindbc.assimp;

/**
 * Represents a single vertex in a mesh
 * NOTICE: TexCoords are packed into position.w and normal.x!
 */
struct Mesh3DVertex {
    /// The position in object space
    vec4f p; // 0
    /// The normals
    vec4f n; // 4
    /// Vertex colors
    vec4f color;
}

/// A collection of vertices composing a mesh, along with related metadata
struct MeshData {
    /// The collection of vertices
    Mesh3DVertex[] vertices;
    /// The number of vertices
    uint numVertices;

    /// The collection of indices
    uint[] indices;
    /// The number of indices
    uint numIndices;

    /// The mesh's name
    string name;
    /// The name of the mesh's material
    string material;
    /// True if the mesh has a material
    bool hasMaterial;

    /// The mins extent (AABB) of the mesh in object space
    vec3f mins;
    /// The maxs extent (AABB) of the mesh in object space
    vec3f maxs;
}

/// A collection of loaded meshes from a model file
struct ModelData {
    /// The list of meshes in this model
    MeshData[] meshes;
    /// The number of meshes found
    uint numMeshes;
}

private struct ImportBlackboard {
    Array!MeshData foundMeshes;
    const aiScene* scene;
}

/// Loads a model file from disk using assimp
class ModelLoader {
    public:
        /// Loads a model, populating data with it's contents
        static bool load(const string path, out ModelData data) {
            const uint flags = aiPostProcessSteps.Triangulate | aiPostProcessSteps.GenSmoothNormals |
                aiPostProcessSteps.GenUVCoords | aiPostProcessSteps.GenBoundingBoxes |
                aiPostProcessSteps.FlipWindingOrder | aiPostProcessSteps.RemoveRedundantMaterials |
                aiPostProcessSteps.FlipUVs |
                aiPostProcessSteps.FindDegenerates | aiPostProcessSteps.JoinIdenticalVertices |
                aiPostProcessSteps.OptimizeMeshes | aiPostProcessSteps.ImproveCacheLocality;

            const aiScene* scene = aiImportFile(path.toStringz, flags);
            if (scene == null) return false;

            Array!MeshData foundMeshes;
            ImportBlackboard blackboard = ImportBlackboard(foundMeshes, scene);

            // Cast away the const-ness... eguh
            processNode(cast(aiNode*)scene.mRootNode, blackboard);
            data.meshes = new MeshData[blackboard.foundMeshes.length()];

            for (uint i = 0; i < blackboard.foundMeshes.length(); i++) {
                data.meshes[i] = blackboard.foundMeshes[i];
            }
            data.numMeshes = cast(uint)blackboard.foundMeshes.length();

            return true;
        }

    private:
        /// Simple recursion
        static void processNode(aiNode* node, ref ImportBlackboard blackboard) {
            for (uint i = 0; i < node.mNumMeshes; i++) {
                MeshData data;
                loadMesh(node, blackboard, data, i);
            }

            for (uint i = 0; i < node.mNumChildren; i++) {
                processNode(node.mChildren[i], blackboard);
            }
        }

        static void loadMesh(aiNode* node, ref ImportBlackboard blackboard, ref MeshData mesh, uint idx) {
            auto aMesh = blackboard.scene.mMeshes[node.mMeshes[idx]];
            mesh.name = to!string(aMesh.mName.data);
            mesh.name.length = aMesh.mName.length;
            
            mesh.hasMaterial = false;
            if (blackboard.scene.mNumMaterials > 0) {
                auto mat = blackboard.scene.mMaterials[aMesh.mMaterialIndex];
                if (mat) {
                    aiString outS;
                    aiGetMaterialString(
                        mat,
                        std.string.toStringz(AI_MATKEY_NAME[0]),
                        AI_MATKEY_NAME[1],
                        AI_MATKEY_NAME[2],
                        &outS
                    );
                    mesh.hasMaterial = true;
                    mesh.material = to!string(outS.data);
                    mesh.material.length = outS.length;
                }
            }

            // Load vertices
            mesh.numVertices = aMesh.mNumVertices;
            mesh.vertices = new Mesh3DVertex[aMesh.mNumVertices];
            for (uint i = 0; i < aMesh.mNumVertices; i++) {
                Mesh3DVertex vertex;
                // Position
                vertex.p.x = aMesh.mVertices[i].x;
                vertex.p.y = aMesh.mVertices[i].y;
                vertex.p.z = aMesh.mVertices[i].z;

                // Normal
                // Note: we sneak in the texcoords in position.w and normal.x
                // thus, normals start at normal.y
                vertex.n.y = aMesh.mNormals[i].x;
                vertex.n.z = aMesh.mNormals[i].y;
                vertex.n.w = aMesh.mNormals[i].z;

                if (aMesh.mTextureCoords[0]) {
                    vertex.p.w = aMesh.mTextureCoords[0][i].x;
                    vertex.n.x = aMesh.mTextureCoords[0][i].y;
                } else {
                    vertex.p.w = 0.0f;
                    vertex.n.x = 0.0f;
                }

                if (aMesh.mColors[0]) {
                    vertex.color.x = aMesh.mColors[0][i].r;
                    vertex.color.y = aMesh.mColors[0][i].g;
                    vertex.color.z = aMesh.mColors[0][i].b;
                    vertex.color.w = aMesh.mColors[0][i].a;
                } else {
                    vertex.color = vec4f(0.0f, 0.0f, 0.0f, 0.0f);
                }

                mesh.vertices[i] = vertex;
            }

            // Load indices
            mesh.numIndices = 0;
            for (uint i = 0; i < aMesh.mNumFaces; i++) {
                const auto face = aMesh.mFaces[i];
                for (uint j = 0; j < face.mNumIndices; j++) {
                    mesh.numIndices++;
                }
            }

            mesh.indices = new uint[mesh.numIndices];
            uint fidx = 0;
            for (uint i = 0; i < aMesh.mNumFaces; i++) {
                const auto face = aMesh.mFaces[i];
                for (uint j = 0; j < face.mNumIndices; j++) {
                    mesh.indices[fidx] = face.mIndices[j];
                    fidx++;
                }
            }

            // Load AABB
            mesh.mins = vec3f(
                aMesh.mAABB.mMin.x,
                aMesh.mAABB.mMin.y,
                aMesh.mAABB.mMin.z
            );
            mesh.maxs = vec3f(
                aMesh.mAABB.mMax.x,
                aMesh.mAABB.mMax.y,
                aMesh.mAABB.mMax.z
            );

            blackboard.foundMeshes.insertBack(mesh);
        }
}