/**
 * @file main.cpp
 * @author Tomáš Ludrovan
 * @brief Main module of the Polygon triangulation demo project.
 * @version 0.1
 * @date 2025-04-29
 * 
 * @copyright MIT license
 * 
 */

#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL.hh>
#include <SDL2pp/Window.hh>
#include <SDL2pp/Renderer.hh>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_sdlrenderer2.h>

struct Polygon {
    std::vector<ImVec2> points;
    ImVec2 point_ex;
    bool is_complete;
};

class Gui {
private:
    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    Polygon polygon;
    bool is_running;

    void processEvents()
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            switch (ev.type) {
                // The X is not shown, but user can still use, e.g., Alt+F4 to trigger this event.
                case SDL_QUIT: {
                    is_running = false;
                } break;

                default: break;
            }
        }
    }

    void showImGuiMenuBar()
    {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Polygon")) {
                if (ImGui::MenuItem("Clear")) {
                    polygon.points.clear();
                    polygon.is_complete = false;
                }
                if (ImGui::MenuItem("Save")) {}
                if (ImGui::MenuItem("Load")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Trapezoidize")) {}
                if (ImGui::MenuItem("Triangulate")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void showImGuiCanvas()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
        if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
        if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

        // Draw border and background color
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

        // This will catch our interactions
        ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        const bool is_left_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
        const bool is_right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
        
        // Polygon construction
        if (!polygon.is_complete) {
            if (is_right_clicked) {
                // Complete the polygon
                polygon.is_complete = true;
            } else {
                // 
                polygon.point_ex = io.MousePos;
                if (is_left_clicked) {
                    polygon.points.push_back(polygon.point_ex);
                }
            }
        }

        // Grid
        draw_list->PushClipRect(canvas_p0, canvas_p1, true);
        const float GRID_STEP = 64.0f;
        for (float x = 0.0f; x < canvas_sz.x; x += GRID_STEP)
            draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
        for (float y = 0.0f; y < canvas_sz.y; y += GRID_STEP)
            draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
        // Polygon
        if (polygon.points.size() > 0) {
            if (polygon.is_complete) {
                draw_list->AddPolyline(polygon.points.data(), polygon.points.size(), IM_COL32(255, 255, 0, 255), ImDrawFlags_Closed, 2.0f);
            } else {
                draw_list->AddPolyline(polygon.points.data(), polygon.points.size(), IM_COL32(255, 255, 0, 255), 0, 2.0f);
                draw_list->AddLine(polygon.points.back(), polygon.point_ex, IM_COL32(255, 255, 0, 255), 1.0f);
            }
        }
        draw_list->PopClipRect();
    }

    void showImGui()
    {
        ImGuiWindowFlags flags
            = ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_MenuBar
            | ImGuiWindowFlags_NoCollapse;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Polygon triangulation demo", &is_running, flags);

        showImGuiMenuBar();
        showImGuiCanvas();

        ImGui::End();
        
        ImGui::Render();
    }

    void render()
    {
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer.Get());
        renderer.Present();
    }
public:
    Gui()
        : sdl(SDL_INIT_VIDEO)
        , window("Polygon triangulation demo", SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_BORDERLESS)
        , renderer(window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)
        , polygon{{}, ImVec2(), false}
        , is_running{false}
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui::StyleColorsDark();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        ImGui_ImplSDL2_InitForSDLRenderer(window.Get(), renderer.Get());
        ImGui_ImplSDLRenderer2_Init(renderer.Get());
    }

    void mainLoop()
    {
        window.Show();
    
        is_running = true;
        while (is_running) {
            processEvents();
    
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            showImGui();
    
            render();
        }
    }

    ~Gui() noexcept
    {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }
};

static void polygon_main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    Gui gui;
    gui.mainLoop();
}

int main(int argc, char *argv[])
{
    try {
        polygon_main(argc, argv);
        return EXIT_SUCCESS;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
