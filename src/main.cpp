#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "common.h"

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

            printf("(%f %f) (%f %f)\n", vMin.x, vMin.y, vMax.x, vMax.y);
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