#pragma once

#include "common.h"
#include "material.h"

#include <qbsp/qbsp.h>

class RayModel
{
  public:
    static RayModel *FromQuakeSolidEntity(const qbsp::SolidEntityPtr ent, float inverseScale,
                                          const std::vector<RayMaterial *> &mats);
    const Model &M() const
    {
        return m_model;
    }
    void SetMaterials(const std::vector<RayMaterial *> &mats);

  private:
    Model m_model;
    vector<Mesh> m_meshes;
    vector<RayMaterial *> m_materials;
    std::map<int, bool> textureIDs;
};