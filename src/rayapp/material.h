#pragma once

#include "common.h"

class WadManager
{
  public:
    WadManager();
    static const WadManager *Instance();
    void AddWad(const string &wadFIle);
    wad::QuakeTexture *FromBuffer(const std::string &texname, uint8_t *buff, int width, int height) const;
    wad::QuakeTexture *FindTexture(const string &name) const;

  private:
    static WadManager *instance;
    wad::QuakeWadPtr defaultWad;
    vector<wad::QuakeWadPtr> wads;
};

class RayMaterial
{
  public:
    static RayMaterial *FromQuakeTexture(wad::QuakeTexture *texture);
    static RayMaterial *FromQuakeTexture(wad::QuakeTexture *texture, Texture lightmap);
    static void SetCamera(Vector3 *camPos);
    static void SetTime(float time);
    static void SetFogDensity(float density);

    RayMaterial();
    ~RayMaterial();
    const Material &M()
    {
        return m_mat;
    }
    const Texture2D &T()
    {
        return m_rayTexture;
    }
    const wad::QuakeTexture *M() const
    {
        return m_quakeTexture;
    }

  private:
    Material m_mat;
    Texture2D m_rayTexture;
    static Shader m_defaultshader;
    static Shader m_skyshader;

    static int m_defaultCamUniformLoc;
    static float m_defaultFogDensityUniformLoc;
    static int m_skyCamUniformLoc;
    static int m_skyTimeUniformLoc;

    wad::QuakeTexture *m_quakeTexture;
};