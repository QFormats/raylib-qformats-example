#include "app.h"

#include <raymath.h>
#include <string.h>

#include <memory>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

bool contains(const string &str, const string &what)
{
    return str.find(what) != str.npos;
}

App::App(AppConfig cfg) : config(cfg)
{

    this->bsp = new qformats::qbsp::QBsp(qformats::qbsp::QBspConfig{.loadTextures = true});
    auto res = bsp->LoadFile(cfg.bspPath.c_str());

    if (res != qformats::qbsp::QBSP_OK)
    {
        cout << "BSP ERROR: " << res << endl;
        return;
    }

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(config.windowWidth, config.windowHeight, "QBSP Viewer");
    SetTargetFPS(60);
    camera = {{2.0f, 5.0f, 2.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 45.0f, 0};
}

void App::LoadMap()
{
    const qbsp::Lightmap *lm = bsp->LightMap();
    Image lmImg;
    lmImg.width = lm->Width();
    lmImg.height = lm->Height();
    lmImg.mipmaps = 1;
    lmImg.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    lmImg.data = RL_MALLOC((lm->Width() * lm->Height()) * sizeof(qbsp::Color));
    memcpy(lmImg.data, lm->RGBA().data(), (lm->Width() * lm->Height()) * sizeof(qbsp::Color));

    Texture2D lmTex = LoadTextureFromImage(lmImg);
    MemFree(lmImg.data);
    GenTextureMipmaps(&lmTex);
    SetTextureFilter(lmTex, TEXTURE_FILTER_TRILINEAR);

    for (const auto &tex : bsp->Textures())
    {
        auto qt = WadManager::Instance()->FromBuffer(tex.name, tex.data, tex.width, tex.height);
        materials.push_back(RayMaterial::FromQuakeTexture(qt, lmTex));
    }

    for (const auto &se : bsp->SolidEntities())
    {
        auto &clname = se->Classname();
        if (contains(clname, "trigger") || contains(clname, "teleport"))
        {
            continue;
        }
        auto m = RayModel::FromQuakeSolidEntity(se, inverseScale, materials);
        models.push_back(m);
    }

    auto pstart = bsp->Entities("info_player_start", [&](const qbsp::EntityPtr pstart) -> bool
                                {
        camera.position =
            Vector3{pstart->Origin().x / inverseScale, pstart->Origin().y / inverseScale, pstart->Origin().z / inverseScale};
        const auto angle = pstart->Angle() * DEG2RAD;
        // Recalculate camera target considering translation and rotation
        auto target = Vector3Transform({0, 0, 1}, MatrixRotateZYX({0, angle, 0}));

        camera.target.x = camera.position.x + target.x;
        camera.target.y = camera.position.y + target.y;
        camera.target.z = camera.position.z + target.z;
        return false; });

    RayMaterial::SetFogDensity(0.07);

    return;
}

void App::Run()
{
    DisableCursor();
    // glFrontFace(GL_CW);
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        if (IsKeyPressed(KEY_M))
        {
            EnableCursor();
        }

        if (IsCursorHidden())
        {
            UpdateCamera(&camera, CAMERA_FREE);
        }

        RayMaterial::SetCamera(&camera.position);
        RayMaterial::SetTime(GetTime());

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        for (const auto &m : models)
        {
            DrawModel(m->M(), {0, 0, 0}, 1, WHITE);

            if (!config.drawWireFrame)
                DrawModelWires(m->M(), {0, 0, 0}, 1, RED);
            if (config.DrawGrid)
                DrawGrid(64, 1.0f); // Draw a grid
        }
        EndMode3D();
        if (config.showFPS)
        {
            DrawFPS(10, 10);
        }

        EndDrawing();
    }
}

void App::PrintInfo()
{
    cout << "BSP Version:\t" << bsp->Version() << endl;
    cout << "NUM Entities:\t" << bsp->SolidEntities().size() << endl;
    cout << "NUM textures:\t" << bsp->Textures().size() << endl;
    for (const auto &tex : bsp->Textures())
    {
        cout << "   " << tex.name << " " << tex.width << "x" << tex.height << endl;
    }
}
