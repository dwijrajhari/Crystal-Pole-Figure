#ifndef M3D_IMGUILAYER
#define M3D_IMGUILAYER

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

class M3D;  // Forward declare M3D class
struct M3Dstate;

void ImGuiLayer_Init(M3D window_);
void ImGuiLayer_Quit();

ImFont* ImGuiLayer_LoadFontFromMemory(const int fontResourceName_, const unsigned int pixelSize, const ImWchar* ranges_);

//void ImguiLayer_Model(M3Dstate&);
//void ImguiLayer_Stereogram(M3Dstate&);
//void ImguiLayer_Orientation(M3Dstate&);
//void ImguiLayer_Symmetry(M3Dstate&);
//void ImguiLayer_LatticeProperties(M3Dstate&);
//void ImguiLayer_AddPlanes(M3Dstate&);
//void ImguiLayer_Output(M3Dstate&);
//void ImguiLayer_Animation(M3Dstate&);
//void ImguiLayer_DisplayProperties(M3Dstate&);



#endif
