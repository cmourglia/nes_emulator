#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "common.h"
#include "cpu.h"
#include "opcode.h"

#include "disassembler.h"

u8 test_code[] = {
    0x20, 0x06, 0x06, 0x20, 0x38, 0x06, 0x20, 0x0d, 0x06, 0x20, 0x2a, 0x06,
    0x60, 0xa9, 0x02, 0x85, 0x02, 0xa9, 0x04, 0x85, 0x03, 0xa9, 0x11, 0x85,
    0x10, 0xa9, 0x10, 0x85, 0x12, 0xa9, 0x0f, 0x85, 0x14, 0xa9, 0x04, 0x85,
    0x11, 0x85, 0x13, 0x85, 0x15, 0x60, 0xa5, 0xfe, 0x85, 0x00, 0xa5, 0xfe,
    0x29, 0x03, 0x18, 0x69, 0x02, 0x85, 0x01, 0x60, 0x20, 0x4d, 0x06, 0x20,
    0x8d, 0x06, 0x20, 0xc3, 0x06, 0x20, 0x19, 0x07, 0x20, 0x20, 0x07, 0x20,
    0x2d, 0x07, 0x4c, 0x38, 0x06, 0xa5, 0xff, 0xc9, 0x77, 0xf0, 0x0d, 0xc9,
    0x64, 0xf0, 0x14, 0xc9, 0x73, 0xf0, 0x1b, 0xc9, 0x61, 0xf0, 0x22, 0x60,
    0xa9, 0x04, 0x24, 0x02, 0xd0, 0x26, 0xa9, 0x01, 0x85, 0x02, 0x60, 0xa9,
    0x08, 0x24, 0x02, 0xd0, 0x1b, 0xa9, 0x02, 0x85, 0x02, 0x60, 0xa9, 0x01,
    0x24, 0x02, 0xd0, 0x10, 0xa9, 0x04, 0x85, 0x02, 0x60, 0xa9, 0x02, 0x24,
    0x02, 0xd0, 0x05, 0xa9, 0x08, 0x85, 0x02, 0x60, 0x60, 0x20, 0x94, 0x06,
    0x20, 0xa8, 0x06, 0x60, 0xa5, 0x00, 0xc5, 0x10, 0xd0, 0x0d, 0xa5, 0x01,
    0xc5, 0x11, 0xd0, 0x07, 0xe6, 0x03, 0xe6, 0x03, 0x20, 0x2a, 0x06, 0x60,
    0xa2, 0x02, 0xb5, 0x10, 0xc5, 0x10, 0xd0, 0x06, 0xb5, 0x11, 0xc5, 0x11,
    0xf0, 0x09, 0xe8, 0xe8, 0xe4, 0x03, 0xf0, 0x06, 0x4c, 0xaa, 0x06, 0x4c,
    0x35, 0x07, 0x60, 0xa6, 0x03, 0xca, 0x8a, 0xb5, 0x10, 0x95, 0x12, 0xca,
    0x10, 0xf9, 0xa5, 0x02, 0x4a, 0xb0, 0x09, 0x4a, 0xb0, 0x19, 0x4a, 0xb0,
    0x1f, 0x4a, 0xb0, 0x2f, 0xa5, 0x10, 0x38, 0xe9, 0x20, 0x85, 0x10, 0x90,
    0x01, 0x60, 0xc6, 0x11, 0xa9, 0x01, 0xc5, 0x11, 0xf0, 0x28, 0x60, 0xe6,
    0x10, 0xa9, 0x1f, 0x24, 0x10, 0xf0, 0x1f, 0x60, 0xa5, 0x10, 0x18, 0x69,
    0x20, 0x85, 0x10, 0xb0, 0x01, 0x60, 0xe6, 0x11, 0xa9, 0x06, 0xc5, 0x11,
    0xf0, 0x0c, 0x60, 0xc6, 0x10, 0xa5, 0x10, 0x29, 0x1f, 0xc9, 0x1f, 0xf0,
    0x01, 0x60, 0x4c, 0x35, 0x07, 0xa0, 0x00, 0xa5, 0xfe, 0x91, 0x00, 0x60,
    0xa6, 0x03, 0xa9, 0x00, 0x81, 0x10, 0xa2, 0x00, 0xa9, 0x01, 0x81, 0x10,
    0x60, 0xa2, 0x00, 0xea, 0xea, 0xca, 0xd0, 0xfb, 0x60};

const char* get_disassembled_line(void* data, int line) {
    const auto* v = (std::vector<std::string>*)data;
    return v->at(line).c_str();
};

int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0) {
        printf("SDL_Init() error: %s\n", SDL_GetError());
        return 1;
    }

    defer(SDL_Quit());

    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    u32 window_flags = SDL_WINDOW_OPENGL;

    SDL_Window* window =
        SDL_CreateWindow("NES Emulator", 1280, 720, window_flags);

    if (window == nullptr) {
        printf("SDL_CreateWindow() error: %s\n", SDL_GetError());
        return 2;
    }

    defer(SDL_DestroyWindow(window));

    u32 renderer_flags = 0;  // PRESENT_VSYNC ?
    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, nullptr, renderer_flags);

    if (renderer == nullptr) {
        printf("SDL_CreateRenderer() error: %s\n", SDL_GetError());
        return 3;
    }

    defer(SDL_DestroyRenderer(renderer));

    ImGui::CreateContext();
    defer(ImGui::DestroyContext());

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    defer(ImGui_ImplSDL3_Shutdown());
    defer(ImGui_ImplSDLRenderer3_Shutdown());

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.0f);

    bool done = false;

    auto disassembly = disassemble_code(test_code, sizeof(test_code));
    int line = 0;

    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);

            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    done = true;
                } break;

                case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                    if (event.window.windowID == SDL_GetWindowID(window)) {
                        done = true;
                    }
                } break;

                case SDL_EVENT_KEY_DOWN: {
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                        done = true;
                    }
                } break;
            }
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        ImGui::DockSpace(ImGui::GetID("DockSpace"));

        ImGui::Begin("Hello, World");
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Text("This is some useful text");
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Another window", &show_another_window);

            ImGui::ColorEdit3("Clear color", (float*)&clear_color);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / io.Framerate, io.Framerate);

            ImGui::ListBox("Disassembly", &line, get_disassembled_line,
                           (void*)&disassembly, (int)disassembly.size(), 30);
        }
        ImGui::End();

        ImGui::Begin("Game");
        {
            ImGui::SetNextWindowContentSize(ImVec2(800, 600));
            ImVec2 vMin = ImGui::GetWindowContentRegionMin();
            ImVec2 vMax = ImGui::GetWindowContentRegionMax();

            vMin.x += ImGui::GetWindowPos().x;
            vMin.y += ImGui::GetWindowPos().y;
            vMax.x += ImGui::GetWindowPos().x;
            vMax.y += ImGui::GetWindowPos().y;

            // printf("(%f %f) (%f %f)\n", vMin.x, vMin.y, vMax.x, vMax.y);
        }
        ImGui::End();

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, (u8)(clear_color.x * 255),
                               (u8)(clear_color.y * 255),
                               (u8)(clear_color.z * 255), 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData());

        SDL_FRect r = {.x = 500, .y = 200, .w = 200, .h = 200};
        // SDL_RenderFillRect(renderer, nullptr);

        SDL_RenderPresent(renderer);
    }

    return 0;
}
