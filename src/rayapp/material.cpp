#include "material.h"
#include <string.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#elif
#include <GL/gl.h>
#endif

WadManager *WadManager::instance = new WadManager();

const char *default_vs = R"""(
#version 330

// Input vertex attributes
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexTexCoord;
layout (location = 5) in vec2 vertexTexCoord2;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec2 fragTexCoord2;

void main()
{
    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragTexCoord2 = vertexTexCoord2;

    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
)""";

const char *default_fs = R"""(
#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec2 fragTexCoord2;
in vec3 fragPosition;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D texture1;

// Output fragment color
out vec4 finalColor;

void main()
{
    // Texel color fetching from texture sampler
    vec4 result = texture(texture0, fragTexCoord);
    vec3 total_light;
    vec4 lm0 = textureLod(texture1, fragTexCoord2, 0.);
    total_light = lm0.xyz;
    total_light *= 2.0;
	result.rgb *= total_light;
	result = clamp(result, 0.0, 1.0);
	
    finalColor = result;
}
)""";

const char *sky_vs = R"""(
#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;

uniform vec3 camera;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragDir;

void main()
{
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragDir = vertexPosition - camera;
    gl_Position = mvp*vec4(vertexPosition, 1.0);
    
}
)""";

const char *sky_fs = R"""(
#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec3 fragDir;

out vec3 fragPosition;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float time;

// Output fragment color
out vec4 finalColor;

void main()
{
   vec3 dir = fragDir;
   dir.y *= 3;
   dir = normalize(dir) * 6*63 / 128.f;
   float scroll = time / 8;

   vec2 texCoordFront = vec2(scroll + dir.x, scroll - dir.z);
   scroll = scroll / 2;
   vec2 texCoordBack = vec2(scroll + dir.x, scroll - dir.z);

   vec4 backColor = texture(texture0, (texCoordBack * vec2(1,0.5))*1.2);
   vec4 frontColor = texture(texture1, (texCoordFront * vec2(1,0.5))*1.2);
   vec4 color = frontColor;
   if (frontColor.x + frontColor.y + frontColor.z < .01f) {
      color = backColor;
   }

   finalColor = color * 1.4;
}
)""";

WadManager const *WadManager::Instance()
{
    if (instance == nullptr)
    {
        instance = new WadManager();
    }
    return instance;
}

WadManager::WadManager()
{
    defaultWad = wad::QuakeWad::NewQuakeWad();
}

void WadManager::AddWad(const string &wadFile)
{
    auto w = wad::QuakeWad::FromFile(wadFile);
    wads.push_back(w);
}

wad::QuakeTexture *WadManager::FromBuffer(const std::string &texname, uint8_t *buff, int width, int height) const
{
    const auto w = wads.size() > 0 ? wads[0] : defaultWad;
    return w->FromBuffer(buff, wad::QuakeWad::IsSkyTexture(texname), width, height);
}

wad::QuakeTexture *WadManager::FindTexture(const string &name) const
{
    wad::QuakeTexture *t = nullptr;
    for (auto &w : wads)
    {
        if (t = w->GetTexture(name); t != nullptr)
        {
            break;
        }
    }
    return t;
}

Shader RayMaterial::m_defaultshader;
Shader RayMaterial::m_skyshader;
int RayMaterial::m_camUniformLoc = 0;
int RayMaterial::m_timeUniformLoc = 0;

RayMaterial::RayMaterial() : m_mat(LoadMaterialDefault())
{
    if (m_defaultshader.id == 0)
    {
        RayMaterial::m_defaultshader = LoadShaderFromMemory(default_vs, default_fs);
        RayMaterial::m_skyshader = LoadShaderFromMemory(sky_vs, sky_fs);
        m_camUniformLoc = GetShaderLocation(m_skyshader, "camera");
        m_timeUniformLoc = GetShaderLocation(m_skyshader, "time");
    }
    return;
}

void RayMaterial::SetCamera(Vector3 *camPos)
{
    SetShaderValue(m_skyshader, m_camUniformLoc, camPos, SHADER_UNIFORM_VEC3);
}

void RayMaterial::SetTime(float time)
{
    SetShaderValue(m_skyshader, m_timeUniformLoc, &time, SHADER_UNIFORM_FLOAT);
}

Texture2D generateTexture(size_t size, int width, int height, void *buff)
{
    Image image;
    image.data = RL_MALLOC(size);
    memcpy(image.data, buff, size);
    image.width = width;
    image.height = height;
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    auto tex = LoadTextureFromImage(image);
    MemFree(image.data);
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_ANISOTROPIC_16X);
    return tex;
}

RayMaterial *RayMaterial::FromQuakeTexture(wad::QuakeTexture *texture)
{
    RayMaterial *rm = new RayMaterial();
    rm->m_quakeTexture = texture;

    const size_t size = rm->m_quakeTexture->raw.size() * sizeof(qformats::wad::color);
    rm->m_rayTexture = generateTexture(size, rm->m_quakeTexture->width, rm->m_quakeTexture->height, &rm->m_quakeTexture->raw[0]);

    if (texture->type == wad::TTYPE_SKY_TEXTURE)
    {
        rm->m_mat.shader = m_skyshader;
        auto skyFront = generateTexture(size, rm->m_quakeTexture->width, rm->m_quakeTexture->height, &rm->m_quakeTexture->raw[0]);
        rm->m_mat.maps[MATERIAL_MAP_METALNESS].texture = skyFront;
    }
    else
    {
        rm->m_mat.shader = m_defaultshader;
    }
    rm->m_mat.maps[MATERIAL_MAP_ALBEDO].texture = rm->m_rayTexture;
    rm->m_mat.maps[MATERIAL_MAP_ALBEDO].color = WHITE;

    return rm;
}

RayMaterial *RayMaterial::FromQuakeTexture(wad::QuakeTexture *texture, Texture lightmap)
{
    auto rm = FromQuakeTexture(texture);
    if (rm->m_quakeTexture->type != wad::TTYPE_SKY_TEXTURE)
    {
        rm->m_mat.maps[MATERIAL_MAP_METALNESS].texture = lightmap;
    }

    return rm;
}