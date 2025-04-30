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

#include <SDL2/SDL.h>
#include <SDL2pp/SDL.hh>
#include <SDL2pp/Window.hh>
#include <SDL2pp/Renderer.hh>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_sdlrenderer2.h>

static void polygon_main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::Window window("Polygon triangulation demo", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
    SDL2pp::Renderer renderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	ImGui_ImplSDL2_InitForSDLRenderer(window.Get(), renderer.Get());
	ImGui_ImplSDLRenderer2_Init(renderer.Get());

    window.Show();

    SDL_Event ev;
    bool is_running = true;
    while (is_running) {
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if (ev.type == SDL_QUIT) {
                is_running = false;
            }
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Begin invisible window
        ImGuiWindowFlags flags
            = ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoBackground
            | ImGuiWindowFlags_MenuBar;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Polygon", NULL, flags);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Polygon")) {
                if (ImGui::MenuItem("Clear")) {}
                if (ImGui::MenuItem("Save")) {}
                if (ImGui::MenuItem("Load")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Trapezoidize")) {}
                if (ImGui::MenuItem("Triangulate")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // End window
        ImGui::End();

        ImGui::Render();

        renderer.SetDrawColor(0xff, 0xff, 0xff);
        renderer.Clear();

        renderer.SetDrawColor();
        SDL_FPoint points[] = {
            {140.0, 120.0},
            {360.0, 90.0},
            {390.0, 280.0},
            {220.0, 220.0},
            {160.0, 300.0},
            {140.0, 120.0},
        };
        SDL_RenderDrawLinesF(renderer.Get(), points, 6);

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer.Get());
        renderer.Present();
    }
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
