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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL.hh>
#include <SDL2pp/Window.hh>
#include <SDL2pp/Renderer.hh>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_sdlrenderer2.h>

namespace fs = std::filesystem;

struct Polygon {
    std::vector<ImVec2> points;
    ImVec2 point_ex;
    bool is_complete;
};

class Gui {
private:
    static constexpr const char *polygons_directory_path = "polygons";
    // 64 is a nice number...
#if FILENAME_MAX < 64
    static constexpr int filename_max = FILENAME_MAX;
#else
    static constexpr int filename_max = 64;
#endif

    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    Polygon polygon;
    bool is_running;
    struct {
        std::vector<fs::path> polygon_filenames;
        char polygon_filename[filename_max + 1];
        bool is_show_save_dialog;
        bool is_show_load_dialog;
        std::string dialog_message;
        bool is_show_info_dialog;
        bool is_show_error_dialog;
    } dialogs;

    void savePolygon()
    {
        fs::path path = fs::path(polygons_directory_path) / fs::path(dialogs.polygon_filename);
        std::ofstream stm(path);
        if (!stm) {
            dialogs.dialog_message = std::string() + "Could not open file \"" + path.string() + "\"";
            dialogs.is_show_error_dialog = true;
            return;
        }
        for (const auto& pt : polygon.points) {
            stm << pt.x << " " << pt.y << "\n";
        }
        if (!stm) {
            dialogs.dialog_message = std::string() + "Failed to write file \"" + path.string() + "\"";
            dialogs.is_show_error_dialog = true;
        } else {
            dialogs.dialog_message = std::string() + "Successfully written file \"" + path.string() + "\"";
            dialogs.is_show_info_dialog = true;
        }
    }

    void loadPolygon()
    {
        fs::path path = fs::path(polygons_directory_path) / fs::path(dialogs.polygon_filename);
        std::ifstream stm(path);
        if (!stm) {
            dialogs.dialog_message = std::string() + "Could not open file \"" + path.string() + "\"";
            dialogs.is_show_error_dialog = true;
            return;
        }
        polygon.points.clear();
        polygon.is_complete = false;
        std::string buf;
        ImVec2 pt;
        const char *nptr;
        char *endptr;
        while (std::getline(stm, buf)) {
            nptr = buf.c_str();
            pt.x = strtof(nptr, &endptr);
            if (nptr == endptr) continue;
            if (*endptr != ' ') continue;
            nptr = endptr + 1;
            pt.y = strtof(nptr, &endptr);
            if (nptr == endptr) continue;
            if (*endptr != '\0') continue;
            polygon.points.push_back(pt);
        }
        polygon.is_complete = true;
        dialogs.dialog_message = std::string("Polygon loaded successfully");
        dialogs.is_show_info_dialog = true;
    }

    void reloadPaths()
    {
        dialogs.polygon_filenames.clear();
        for (const auto& entry : fs::directory_iterator(polygons_directory_path)) {
            dialogs.polygon_filenames.push_back(entry.path().filename());
        }
    }

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

    bool showImGuiSaveLoadDialog(const char *window_caption, const char *btn_caption)
    {
        static size_t selected_item_idx = -1;
        bool result = false;

        ImGui::Begin(window_caption, NULL, 0);

        if (ImGui::BeginListBox("##listbox 1")) {
            for (size_t i = 0; i < dialogs.polygon_filenames.size(); ++i) {
                const bool is_selected = (i == selected_item_idx);
                if (ImGui::Selectable(dialogs.polygon_filenames[i].c_str(), is_selected)) {
                    selected_item_idx = i;
                    strncpy(dialogs.polygon_filename, dialogs.polygon_filenames[i].c_str(), filename_max);
                    dialogs.polygon_filename[filename_max] = '\0';
                }
            }
            ImGui::EndListBox();
        }
        ImGui::InputText("##filename", dialogs.polygon_filename, filename_max + 1);
        ImGui::SameLine();
        if (ImGui::Button(btn_caption)) {
            dialogs.is_show_load_dialog = dialogs.is_show_save_dialog = false;
            result = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            dialogs.is_show_load_dialog = dialogs.is_show_save_dialog = false;
        }

        ImGui::End();

        return result;
    }

    void showImGuiSaveDialog()
    {
        if (showImGuiSaveLoadDialog("Save polygon", "Save")) {
            savePolygon();
        }
    }

    void showImGuiLoadDialog()
    {
        if (showImGuiSaveLoadDialog("Load polygon", "Load")) {
            loadPolygon();
        }
    }

    void showImGuiMessageDialog(const char *caption)
    {
        ImGui::Begin(caption);
        ImGui::Text("%s", dialogs.dialog_message.c_str());
        if (ImGui::Button("OK")) {
            dialogs.is_show_error_dialog = dialogs.is_show_info_dialog = false;
        }
        ImGui::End();
    }

    void showImGuiErrorDialog()
    {
        showImGuiMessageDialog("Error");
    }

    void showImGuiInfoDialog()
    {
        showImGuiMessageDialog("Info");
    }

    void showImGuiMenuBar()
    {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Polygon")) {
                ImGui::BeginDisabled(!polygon.is_complete);
                if (ImGui::MenuItem("Clear")) {
                    polygon.points.clear();
                    polygon.is_complete = false;
                }
                if (ImGui::MenuItem("Save")) {
                    reloadPaths();
                    dialogs.is_show_save_dialog = true;
                }
                ImGui::EndDisabled();
                if (ImGui::MenuItem("Load")) {
                    reloadPaths();
                    dialogs.is_show_load_dialog = true;
                }
                ImGui::Separator();
                ImGui::BeginDisabled(!polygon.is_complete);
                if (ImGui::MenuItem("Trapezoidize")) {}
                if (ImGui::MenuItem("Triangulate")) {}
                ImGui::EndDisabled();
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
                // Show the edge to be added
                polygon.point_ex = io.MousePos;
                if (is_left_clicked) {
                    // Add new vertex
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

        if (dialogs.is_show_save_dialog) showImGuiSaveDialog();
        if (dialogs.is_show_load_dialog) showImGuiLoadDialog();
        if (dialogs.is_show_error_dialog) showImGuiErrorDialog();
        if (dialogs.is_show_info_dialog) showImGuiInfoDialog();
    }

    void render()
    {
        ImGui::Render();
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
        , dialogs{{}, {}, false, false, "", false, false}
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
