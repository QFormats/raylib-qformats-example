#include "model.h"
#include <raymath.h>
#include <map>

// Get identity matrix

/*
0 -1 0 0
0 0 1 0
-1 0 0 0
0 0 0 1
*/

static Matrix MatrixQIdentity(void)
{
    Matrix result = {0.0f, -1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.0f,
                     -1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 0.0f, 1.0f};

    return result;
}

RayModel *RayModel::FromQuakeSolidEntity(const qbsp::SolidEntityPtr ent, float inverseScale, const std::vector<RayMaterial *> &mats)
{
    RayModel *rm = new RayModel();
    std::vector<int> meshesMatIDs;

    int fc = 0;
    for (const auto f : ent->Faces())
    {
        auto mesh = Mesh{0};
        mesh.triangleCount = f->indices.size() / 3;
        mesh.vertexCount = f->verts.size();
        mesh.vertices = (float *)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
        mesh.texcoords = (float *)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
        mesh.texcoords2 = (float *)MemAlloc(mesh.vertexCount * 2 * sizeof(float));

        mesh.indices = (unsigned short *)MemAlloc(f->indices.size() * sizeof(unsigned short));
        int iv = 0, iu = 0, iu2 = 0;
        for (const auto &v : f->verts)
        {
            mesh.vertices[iv++] = v.point.x / inverseScale;
            mesh.vertices[iv++] = v.point.y / inverseScale;
            mesh.vertices[iv++] = v.point.z / inverseScale;
            mesh.texcoords[iu++] = v.uv.x;
            mesh.texcoords[iu++] = v.uv.y;
            mesh.texcoords2[iu2++] = v.lm_uv.x;
            mesh.texcoords2[iu2++] = v.lm_uv.y;
        }

        for (int i = 0; i < f->indices.size(); i++)
        {
            mesh.indices[i] = f->indices[i];
        }

        UploadMesh(&mesh, false);
        rm->m_meshes.push_back(mesh);
        rm->textureIDs[f->info->texture_id] = true;
        meshesMatIDs.push_back(f->info->texture_id);
    }

    // rm->m_model.transform = MatrixMultiply(MatrixIdentity(), MatrixRotateX(DEG2RAD * -90));
    rm->m_model.transform = MatrixIdentity();
    rm->m_model.meshCount = rm->m_meshes.size();

    rm->m_model.meshes = (Mesh *)MemAlloc(rm->m_model.meshCount * sizeof(Mesh));
    rm->m_model.materials = (Material *)MemAlloc(mats.size() * sizeof(Material));
    rm->m_model.meshMaterial = (int *)MemAlloc(rm->m_model.meshCount * sizeof(int));

    for (int im = 0; im < mats.size(); im++)
    {
        rm->m_model.materials[im] = mats[im]->M();
    }

    for (int m = 0; m < rm->m_model.meshCount; m++)
    {
        rm->m_model.meshes[m] = rm->m_meshes[m];
        rm->m_model.meshMaterial[m] = meshesMatIDs[m] - 1;
    }

    return rm;
}

void RayModel::SetMaterials(const std::vector<RayMaterial *> &mats)
{
    for (auto tid : textureIDs)
    {
        m_model.materials[0] = mats[tid.first - 1]->M();
        continue;
    }
}