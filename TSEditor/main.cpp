#include "framework.h"
#include "renderer/render.h"

static SDL_Window* window;

static void mainExit(void)
{
    Render.Release3DImGui();
    Render.Release3DDevice();
    Render.Release3D();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void mainInit(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        TraceLogSDL(LogType::Error, "Failed to setup SDL");
        exit(EXIT_FAILURE);
    }

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    Uint32 window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    SDL_Window* window = SDL_CreateWindow("TSEditor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, window_flags);
    if (window == nullptr)
    {
        TraceLogSDL(LogType::Error, "Failed to create SDL window");
        exit(EXIT_FAILURE);
    }

    SDL_SysWMinfo wmInfo = {};
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForD3D(window);

    Render.Initialize(wmInfo.info.win.window, 1920, 1080, false);
    Render.SetZBuffer(true);
    Render.SetAlphaBlend(true);
    Render.SetScissorTest(false);
}

static void mainRun(void)
{
    bool done = false;
    bool show_demo_window = true;
    while (!done)
    {
        SDL_Event evts;
        while (SDL_PollEvent(&evts))
        {
            ImGui_ImplSDL2_ProcessEvent(&evts);
            switch (evts.type)
            {
            case SDL_QUIT:
                done = true;
                break;
            case SDL_WINDOWEVENT:
                switch (evts.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    if (evts.window.windowID == SDL_GetWindowID(window))
                        done = true;
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    // TODO: Handle resize.
                    //if (event.window.windowID == SDL_GetWindowID(window))
                    break;
                }
                break;
            }
        }

        Render.ClearScreen();
        Render.SetZBuffer(true);
        Render.SetAlphaBlend(true);
        Render.BeginScene();

        Render.EndScene();

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::EndFrame();

        Render.ClearScreen();
        Render.SetZBuffer(false);
        Render.SetAlphaBlend(false);
        Render.BeginScene();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        Render.EndScene();

        Render.Present();
    }
}

int main(int argc, char* argv[])
{
    atexit(mainExit);
    mainInit(argc, argv);
    mainRun();
    exit(EXIT_SUCCESS);
}
