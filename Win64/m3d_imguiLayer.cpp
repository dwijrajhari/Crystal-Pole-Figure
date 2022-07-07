#include <M3D/m3d.h>
#include <M3D/m3d_res.h>
#include <M3D/m3d_imguiLayer.h>

#include "resource.h"

void ImGuiLayer_Init(M3D window_)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 5.0f;
    ImGui::GetStyle().GrabRounding = 5.0f;
    ImGui::GetStyle().FrameRounding = 5.0f;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_.mWindow, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void ImGuiLayer_Quit()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

ImFont* ImGuiLayer_LoadFontFromMemory(const int fontResourceName_, const unsigned int pixelSize, const ImWchar* ranges_)
{
    unsigned int fontfileSize = fGetResourceSize(fontResourceName_, FONT);
    char* fontfileData = new char[fontfileSize];
    fGetResourceData(fontResourceName_, FONT, fontfileData);
    return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontfileData, fontfileSize, pixelSize, 0, ranges_);
}

//void ImguiLayer_Model(M3Dstate&)
//{
//}
//
//void ImguiLayer_Stereogram(M3Dstate&)
//{
//}
//
//void ImguiLayer_Orientation(M3Dstate&)
//{
//}
//
//void ImguiLayer_Symmetry(M3Dstate&)
//{
//}
//
//void ImguiLayer_LatticeProperties(M3Dstate& state)
//{
//}
//
//void ImguiLayer_AddPlanes(M3Dstate&)
//{
//}
//
//void ImguiLayer_Output(M3Dstate&)
//{
//}
//
//void ImguiLayer_Animation(M3Dstate&)
//{
//}
//
//void ImguiLayer_DisplayProperties(M3Dstate&)
//{
//}

