#pragma once

#include "common.h"
#include <qbsp/qbsp.h>

#include "material.h"
#include "model.h"

struct AppConfig
{
    bool DrawGrid = false;
    bool drawWireFrame = false;
    bool showFPS = false;
    int windowWidth = 640;
    int windowHeight = 480;
    string bspPath = "";
};

class App
{
  public:
    App(AppConfig cfg);
    void Run();
    void PrintInfo();
    void LoadMap();
    void LoadMapNg();
    const AppConfig &Cfg()
    {
        return config;
    }

  private:
    AppConfig config;
    Camera3D camera;
    vector<RayMaterial *> materials;
    vector<RayModel *> models;
    qbsp::QBsp *bsp;
    float inverseScale = 92;
    Material defaultMaterial = LoadMaterialDefault();
};