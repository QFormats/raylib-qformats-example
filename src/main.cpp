#include <chrono>
#include <iostream>
#include <ostream>
#include <string.h>

#include "rayapp/app.h"

static void printBanner()
{
    string banner = R"""(
  ______  ___________   _   _ _____ _____ _    _ ___________ 
| ___ \/  ___| ___ \ | | | |_   _|  ___| |  | |  ___| ___ \
| |_/ /\ `--.| |_/ / | | | | | | | |__ | |  | | |__ | |_/ /
| ___ \ `--. \  __/  | | | | | | |  __|| |/\| |  __||    / 
| |_/ //\__/ / |     \ \_/ /_| |_| |___\  /\  / |___| |\ \ 
\____/ \____/\_|      \___/ \___/\____/ \/  \/\____/\_| \_|

Quake BSP Viewer sample app using qformats and raylib

https://github.com/QFormats

usage: bspviewer -bsp E1M1.bsp

)""";

    std::cout << banner << std::endl;
}

static int stoiOrDef(const string &str, int def)
{
    try
    {
        def = std::stoi(string(str));
    }
    catch (std::invalid_argument e)
    {
        cout << "WARN: cannot convert " << str << " to int" << endl;
    }

    return def;
}

AppConfig argsToAppConfig(int argc, char *argv[])
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    AppConfig cfg;
    for (auto argIter = args.begin(); argIter != args.end(); argIter++)
    {
        if (*argIter == "-bsp" && ++argIter != args.end())
        {
            cfg.bspPath = *argIter;
            continue;
        }
        if (*argIter == "-width" && ++argIter != args.end())
        {
            cfg.windowWidth = stoiOrDef(string(*argIter), 800);
            continue;
        }
        if (*argIter == "-height" && ++argIter != args.end())
        {
            cfg.windowHeight = std::stoi(string(*argIter));
            continue;
        }
    }
    return cfg;
}

int main(int argc, char *argv[])
{
    auto cfg = argsToAppConfig(argc, argv);
    if (cfg.bspPath == "")
    {
        printBanner();
        return 0;
    }

    auto app = App(cfg);
    auto start_time = std::chrono::high_resolution_clock::now();
    app.LoadMap();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = end_time - start_time;

    std::cout << "bsp loaded in: " << time / std::chrono::milliseconds(1) << " ms\n";
    app.Run();
    return 0;
}
