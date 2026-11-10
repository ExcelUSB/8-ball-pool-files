#include "Main.h"
#include "ui/ui.hh"
#include "globals.hh"
#include "./iconcpp.h"
#include "./Font.h"
#include "const.h"

#include "injector.h"
#include "binary.hpp"
#include "SecureStrings.h"
#include "resource.h"

void CALLBACK securityTimerCallback(HWND, UINT, UINT_PTR, DWORD)
{
    HW_PROFILE_INFO hwProfileInfo;

    if (IsDebuggerPresent()) {
        if (GetCurrentHwProfile(&hwProfileInfo) != NULL){
            ui::UserBan(hwProfileInfo.szHwProfileGuid);
        }

        exit(EXIT_FAILURE);
    }
}

void securityThread()
{
    HW_PROFILE_INFO hwProfileInfo;

    while (true) {
        if (IsDebuggerPresent()) {
            if (GetCurrentHwProfile(&hwProfileInfo) != NULL) {
                ui::UserBan(hwProfileInfo.szHwProfileGuid);
            }

            exit(EXIT_FAILURE);
        }

        Sleep(1000 * 15);
    }
}

void loadHack(const char* decryptionKey, int keySize)
{
    int i;

    for (i = 0; i < sizeof(binaryRawData); i++) {
        binaryRawData[i] ^= decryptionKey[i % keySize];
    }

    ManualMapDll(GetCurrentProcess(), binaryRawData, sizeof(binaryRawData));
}

void loadBetaHack(const char* decryptionKey, int keySize)
{
    int i;

    for (i = 0; i < sizeof(betaTestBinaryRawData); i++) {
        betaTestBinaryRawData[i] ^= decryptionKey[i % keySize];
    }

    ManualMapDll(GetCurrentProcess(), betaTestBinaryRawData, sizeof(betaTestBinaryRawData));
}

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    int         securityTimerID = SetTimer(nullptr, 0UL, 1000 * 15, securityTimerCallback);
    std::thread secThread       = std::thread(securityThread);

    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, instance, LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON1)), NULL, NULL, NULL, ui::window_title, NULL};
    RegisterClassEx(&wc);
    main_hwnd = CreateWindow(wc.lpszClassName, ui::window_title, WS_POPUP, 0, 0, 5, 5, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(main_hwnd)) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ShowWindow(main_hwnd, SW_HIDE);
    UpdateWindow(main_hwnd);

    // Setup Dear ImGui context
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; //crutial for not leaving the imgui.ini file
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
    ImFontConfig icons_config;

    ImFontConfig CustomFont;
    CustomFont.FontDataOwnedByAtlas = false;


    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = 2.5;
    icons_config.OversampleV = 2.5;

    io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 21.f, &CustomFont);
    io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 19.0f, &icons_config, icons_ranges);
    io.Fonts->AddFontDefault();
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(main_hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);


    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            static int init = false;
            if (!init) {
                ui::init(g_pd3dDevice);
                init = true;
            }
            else {
                ui::render();
            }
        }

        ImGui::EndFrame();
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();

            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            ResetDevice();
        }

        if (!globals.active) {
            msg.message = WM_QUIT;
        }
    }

    KillTimer(nullptr, securityTimerID);

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(main_hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}