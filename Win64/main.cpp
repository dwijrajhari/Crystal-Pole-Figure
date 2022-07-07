#include <M3D/m3d.h>
#include <M3D/m3d_vao.h>
#include <ft2build.h>
#include FT_FREETYPE_H  
#include <M3D/m3d_camera.h>
#include <M3D/m3d_shader.h>
#include <M3D/m3d_crystals.h>
#include <M3D/m3d_symmetry.h>
#include <M3D/m3d_imguiLayer.h>
#include <M3D/m3d_renderObjects.h>
#include <M3D/m3d_stereographic.h>

#include <M3D/m3d_symmetrySymbols.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "resource.h"

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

std::map<char, Character> Characters;

std::string LS_enum2str(LatticeSystem& ls)
{
    if (ls == LatticeSystem::CUBIC) return "Cubic";
    else if (ls == LatticeSystem::TETRAGONAL) return "Tetragonal";
    else if (ls == LatticeSystem::HEXAGONAL) return "Hexagonal";
    else if (ls == LatticeSystem::RHOMBOHEDRAL) return "Rhombohedral";
    else if (ls == LatticeSystem::ORTHORHOMBIC) return "Orthorhombic";
    else if (ls == LatticeSystem::MONOCLINIC) return "Monoclinic";
    else if (ls == LatticeSystem::TRICLINIC) return "Triclinic";
    else return "None";
}

void print_symmetry(Symmetry& s)
{
    std::printf("Symmetry(%d)\n", (int)s.operations.size());
    for (const std::pair<glm::quat, bool>& q : s.operations)
    {
        std::printf("[%.3f, %.3f, %.3f, %.3f]\n", q.first.w, q.first.x, q.first.y, q.first.z);
    }
    std::printf("\n");
}

#define min(A, B) (A<=B)?(A):(B)
#define max(A, B) (A>=B)?(A):(B)

M3D M3Dwindow(1920, 1080, "Crystal Pole Figure - (Test)");
M3DCamera M3Dcam0;

float vertex_labels[32];
char32_t vertx_labels_str[8] = { 'D', ' ', ' ', ' ', 'C', 'A', 'B', ' ' };

GLuint fb_main, fb_intermediate, fb_stereo;

GLfloat MainViewPortPos[2]  = { 0.0f, 0.0f };
GLfloat MainViewPortSize[2] = { 1024, 1024 };
GLfloat StereoViewPortSize  = 800;

glm::vec4 color_edge_gizmo[3] =
{
    {1.0, 0.0, 0.0, 0.95},
    {0.0, 1.0, 0.0, 0.95},
    {0.0, 0.0, 1.0, 0.95}
};

float GizmoSize = 150;
bool MainViewPortSizeHasChanged   = 0;
bool StereoViewPortSizeHasChanged = 0;

GLint selectedObjectID = 0;
unsigned int VAO, VBO;

// Mouse event handler properties
bool mouseInit = 0;
bool leftButtonPressed = 0;
bool cursorOverMainViewport = 0;
GLfloat lastMouse[2] = { 300, 300 };
GLfloat mouseSensitivity = 0.4f;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS) leftButtonPressed = true;
        else if (action == GLFW_RELEASE) leftButtonPressed = false;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (cursorOverMainViewport)
    {
        glm::vec3 newPos = M3Dcam0.mPos + (GLfloat)(yoffset * 0.2) * M3Dcam0.mDir;
        // Dont let the camera cross the origin !
        if (glm::dot(newPos, M3Dcam0.mPos) > 0) 
            M3Dcam0.mPos = newPos;
    }
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    // Resize M3Dwindow
    M3Dwindow.mScreenHeight = height; M3Dwindow.mScreenWidth = width;
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    if (!mouseInit)
    {
        lastMouse[0] = (GLfloat)x;
        lastMouse[1] = (GLfloat)y;

        mouseInit = 1;
    }

    // Handle camera rotation
    if (leftButtonPressed && cursorOverMainViewport)
    {
        M3Dcam0.fHandleRotationTurntable( ((GLfloat)x - lastMouse[0]) * mouseSensitivity, 
                                       ((GLfloat)y - lastMouse[1]) * mouseSensitivity );
    }

    // Save last mouse position
    lastMouse[0] = (GLfloat)x;
    lastMouse[1] = (GLfloat)y;

    // Stencil based mouse picking
    if (cursorOverMainViewport)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fb_intermediate);
        glReadPixels(x - MainViewPortPos[0], MainViewPortSize[1] - (y - MainViewPortPos[1]) - 1, 1, 1, 
            GL_STENCIL_INDEX, GL_UNSIGNED_INT, &selectedObjectID);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void RenderText(ShaderProgram& s, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    s.fUseProgram();
    glUniform3f(glGetUniformLocation(s.programID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderCharCentered(GLuint sp_text, FT_ULong c, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    glUseProgram(sp_text);
    glUniform3f(glGetUniformLocation(sp_text, "textColor"), color.x, color.y, color.z);


    Character ch = Characters[c];


    float w = ch.Size.x * scale;
    float h = ch.Size.y * scale;

    float xpos = x + ch.Bearing.x * scale - w / 2;
    float ypos = y - (ch.Size.y - ch.Bearing.y) * scale - h / 2;
    // update VBO for each character
    float vertices[6][4] = {
        { xpos,     ypos + h,   0.0f, 0.0f },
        { xpos,     ypos,       0.0f, 1.0f },
        { xpos + w, ypos,       1.0f, 1.0f },

        { xpos,     ypos + h,   0.0f, 0.0f },
        { xpos + w, ypos,       1.0f, 1.0f },
        { xpos + w, ypos + h,   1.0f, 0.0f }
    };
    // update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // render glyph texture over quad
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    glUniform1i(glGetUniformLocation(sp_text, "text"), 0);
    // render quad
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


int main()
{
    // Set callbacks
    glfwSetCursorPosCallback(M3Dwindow.mWindow, cursor_position_callback);
    glfwSetScrollCallback(M3Dwindow.mWindow, scroll_callback);
    glfwSetMouseButtonCallback(M3Dwindow.mWindow, mouse_button_callback);
    glfwSetWindowSizeCallback(M3Dwindow.mWindow, window_size_callback);

    // Enable V-sync
    glfwSwapInterval(1);

    // Setup Dear ImGui 
    ImGuiLayer_Init(M3Dwindow);
    
    // Load Font with glyph ranges
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF,  // Basic Latin
        0x2081, 0x2081,  // Subscript 1   ₁ 
        0x2082, 0x2082,  // Subscript 2   ₂ 
        0x03C6, 0x03C6,  // Small phi     φ 
        0x03D5, 0x03D5,  // Capital phi   Φ 
        0x03B1, 0x03B1,  // Small alpha   α
        0x03B2, 0x03B2,  // Small alpha   β
        0x03B3, 0x03B3,  // Small alpha   γ
    };
    ImFont* avrile = ImGuiLayer_LoadFontFromMemory(AVRILE_SANS_REGULAR, 22, ranges);

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "avrile-sans-regular.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Load Shaders
    ShaderProgram sp_basic  = fShaderProgramfromMemory(M3D_VSHADER       , M3D_FSHADER       );
    ShaderProgram sp_point  = fShaderProgramfromMemory(M3D_VSHADER_POINT , M3D_FSHADER_POINT );
    ShaderProgram sp_stereo = fShaderProgramfromMemory(M3D_VSHADER_STEREO, M3D_FSHADER_STEREO);
    ShaderProgram sp_text   = fShaderProgramfromMemory(M3D_VSHADER_TEXT,   M3D_FSHADER_TEXT  );
  
    // Demo crystal data
    Crystal cubic(Lattice(1.0f, 1.0f, 1.0f));

    float cubeData[6 * 6 * 3];
    float ttData[12 * 3];
    cubic.fGenerateCrystalRenderData(cubeData, ttData);

    GLuint layoutTT[] = { 3 };

    GLuint bCube = 0;
    VertexArrayObject cube;
    cube.fAddArrayBuffer(&bCube, sizeof(cubeData), cubeData, 3 * sizeof(float), 1, layoutTT);

    GLuint bTT = 0;
    VertexArrayObject TT;
    TT.fAddArrayBuffer(&bTT, sizeof(ttData), ttData, 3 * sizeof(float), 1, layoutTT);

    float quad[] =
    {
        -1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f
    };

    VertexArrayObject Circ;
    GLuint bCirc = 0;
    GLuint layoutC[] = { 3 };
    Circ.fAddArrayBuffer(&bCirc, sizeof(quad), quad, 3 * sizeof(float), 1, layoutC);

    // BB data
    AABBObject BB;

    // Gizmo data
    GLfloat verticesGizmo[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    GLuint bGizmo = 0;
    VertexArrayObject gizmo;
    GLuint layout[] = { 3 };
    gizmo.fAddArrayBuffer(&bGizmo, sizeof(verticesGizmo), verticesGizmo, 3 * sizeof(float), 1, layout);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    //---------------------------MainViewPort FBO----------------------------------------------------------
    GLuint MaxMainSize[2] = { 1920, 1080 };
    glGenFramebuffers(1, &fb_main);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_main);

    GLuint mainTexture;
    glGenTextures(1, &mainTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mainTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, MaxMainSize[0], MaxMainSize[1], GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mainTexture, 0);

    GLuint mainSDTexture;
    glGenRenderbuffers(1, &mainSDTexture);
    glBindRenderbuffer(GL_RENDERBUFFER, mainSDTexture);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, MaxMainSize[0], MaxMainSize[1]);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mainSDTexture);
    //--------------------------------------------------------------------------------------------------------


    //----------------------------StereoViewPort FBO----------------------------------------------------------
    GLuint MaxStereoSize = 1024;
    glGenFramebuffers(1, &fb_stereo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_stereo);

    GLuint stereoTex;
    glGenTextures(1, &stereoTex);
    glBindTexture(GL_TEXTURE_2D, stereoTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, MaxStereoSize, MaxStereoSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, stereoTex, 0);
    //-------------------------------------------------------------------------------------------------


    //----------------------------Intermediate FBO-----------------------------------------------------
    glGenFramebuffers(1, &fb_intermediate);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_intermediate);

    GLuint screenTexture;
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, MaxMainSize[0], MaxMainSize[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

    GLuint screenStencil;
    glGenRenderbuffers(1, &screenStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, screenStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, MaxMainSize[0], MaxMainSize[1]);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, screenStencil);
    //-----------------------------------------------------------------------------------------------------

    // State
    bool projectOrtho = 0; 
    bool show_demo_window = 0;
    bool showFaces = 1;
    bool showGizmo = 0;
    bool showTT = 0;
    bool showBB = 1;
    glm::vec3 bgColor = { 0.0, 0.0, 0.0 };
    glm::vec4 wireColor = { 1.0f, 1.0f, 0.0f, 0.9f};
    glm::vec4 bbColor = { 1.0, 1.0, 1.0 , 0.3f };

    float mainAspect = 1.0f;

    // grid controls
    glm::vec3 stereobgColor  = { 0.2f, 0.2f, 0.2f };
    glm::vec4 gridFillColor  = { 0.8f, 0.8f, 0.8f, 1.0f };
    glm::vec4 gridFrameColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    float stereoZoom = 0.9f;
    int azimuthalGridDensity = 1, polarGridDensity = 2;

    // plane debug
    std::vector<PlaneObject> planes;
    PlaneObject p1(&cubic, 1, 1, 1);
    planes.push_back(p1);

    

    //------------------------------------------------------------------------------------------------------------------------
    
    
    //Symmetry _s4;
    //_s4.fAddOp(glm::quat(1.0, 0.0, 0.0, 0.0));
    //_s4.fAddOp(glm::quat(0.707106, 0.0, 0.0, 0.707106));
    

    // Miller direction for pole figure
    MillerDirection m1(1, 0, 0);
    std::vector<MillerDirection> eqv = s432 * m1;

    // Set opengl state
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);

    // main loop
    while (M3Dwindow.fIsRunning())
    {        
        glm::vec3 eulerRad = glm::radians(cubic.orientation);
        glm::mat4 orientation =
            glm::rotate(glm::rotate(glm::rotate(glm::mat4(1.0f),
                eulerRad[0], { 0.0f, 1.0f, 0.0f }),
                eulerRad[1], { 0.0f, 0.0f, 1.0f }),
                eulerRad[2], { 0.0f, 1.0f, 0.0f }) *
            glm::rotate(glm::rotate(glm::mat4(1.0f),
                glm::radians(-90.0f), { 1.0f, 0.0f, 0.0f }),
                glm::radians(-90.0f), { 0.0f, 0.0f, 1.0f });

        glm::mat4 orientation_polefig =

            glm::rotate(glm::rotate(glm::rotate(glm::mat4(1.0f),
                eulerRad[0], { 0.0f, 0.0f, 1.0f }),
                eulerRad[1], { 1.0f, 0.0f, 0.0f }),
                eulerRad[2], { 0.0f, 0.0f, 1.0f });

        // Transforms
        glm::mat4 model_crystal = glm::translate(orientation, -cubic.centroid);
        glm::mat4 model_bb = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
        glm::mat4 model_gizmo = glm::scale(orientation, glm::vec3(1.5f));

        glm::mat4 view = M3Dcam0.fGetView();

        float aspect = mainAspect;
        glm::mat4 projection;
        if (projectOrtho)
        {
            float l = 0.4 * glm::length(M3Dcam0.mPos);
            projection = glm::ortho(-aspect * l, aspect * l, -l, l, -10.0f, 100.0f);
        }
        else
        {
            projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        }

        //---------------------------------------Render Stereogram------------------------------------------------------------------
        glDisable(GL_DEPTH_TEST);

        glBindFramebuffer(GL_FRAMEBUFFER, fb_stereo);
        
        glm::mat4 Transformation = glm::scale(glm::mat4(1.0f),glm::vec3(1.0f));
        sp_stereo.fUseProgram();
        glUniformMatrix4fv(glGetUniformLocation(sp_stereo.programID, "transform"), 1, GL_FALSE, glm::value_ptr(Transformation));
        glUniform1i(glGetUniformLocation(sp_stereo.programID, "azimuthalGridDensity"), azimuthalGridDensity);
        glUniform1i(glGetUniformLocation(sp_stereo.programID, "polarGridDensity"), polarGridDensity);

        glViewport(0, 0, StereoViewPortSize, StereoViewPortSize);

        Circ.fBind();
        glUniform4f(glGetUniformLocation(sp_stereo.programID, "wireColor"), gridFrameColor.r, gridFrameColor.g, gridFrameColor.b, gridFrameColor.a);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        sp_point.fUseProgram();
        Transformation = glm::scale(glm::mat4(1.0f), stereoZoom * glm::vec3(1.0f));

        glUniformMatrix4fv(glGetUniformLocation(sp_point.programID, "transform"), 1, GL_FALSE, glm::value_ptr(Transformation));
        glUniform4f(glGetUniformLocation(sp_point.programID, "color"), 1.0f, 0.0f, 0.0f, 1.0f);
        glPointSize(30.0f);

        for (const MillerDirection& m : eqv)
        {
            glm::vec3 ptOnSphere = orientation_polefig * glm::vec4(glm::normalize(m.uvw*glm::vec3(cubic.lattice.a, cubic.lattice.b, cubic.lattice.c)), 1.0f);
            //if (ptOnSphere.z >= 0)
            {
                glm::vec2 pole = fProjectPoint(ptOnSphere);
                glUniform2f(glGetUniformLocation(sp_point.programID, "position"), -pole.y, pole.x);
                glUniform1i(glGetUniformLocation(sp_point.programID, "south"), ptOnSphere.z >= 0 ? 1 : 0);
                glDrawArrays(GL_POINTS, 0, 1);
            }
        }

        glEnable(GL_DEPTH_TEST);
        //------------------------------------------------------------------------------------------------------------------
        

        //--------------------------------Render to MainViewPort FBO--------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, fb_main);
        
        sp_basic.fUseProgram();
        glUniformMatrix4fv(glGetUniformLocation(sp_basic.programID, "model"), 1, GL_FALSE, glm::value_ptr(model_crystal));
        glUniformMatrix4fv(glGetUniformLocation(sp_basic.programID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(sp_basic.programID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // clear screen
        glViewport(0, 0, MainViewPortSize[0], MainViewPortSize[1]);
        M3Dwindow.fClearScreen(bgColor.x, bgColor.y, bgColor.z);

        // Draw hkl plane frames
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        sp_basic.fUseProgram();

        for (PlaneObject const& plane : planes)
        {
            if (plane.objectDisplay)
            {
                if (selectedObjectID == plane.objectID + 11)
                {
                    glLineWidth(3.0f);
                    glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), 0.0, 1.0, 0.0, 1.0);
                    plane.fRenderFrame();
                    glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), 1.0, 1.0, 1.0, 0.8);
                    plane.fRenderFill();

                }
                else
                {
                    glLineWidth(1.0f);
                    glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), 0.9, 0.9, 0.9, 0.6);
                    plane.fRenderFrame();
                    glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), 0.7, 0.7, 0.7, 0.8);
                    plane.fRenderFill();
                }
            }
        }

        glLineWidth(2.0f);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        // Draw Crystal frame
        cube.fBind();
        sp_basic.fUseProgram();
        glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), wireColor.r, wireColor.g, wireColor.b, wireColor.a);
        glDrawArrays(GL_LINE_STRIP, 0, 18);
        glDrawArrays(GL_LINE_STRIP, 18, 18);

        // Draw TT frame
        if (showTT)
        {
            TT.fBind();
            sp_basic.fUseProgram();
            glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), wireColor.r, wireColor.g, wireColor.b, wireColor.a);
            for (int i = 0; i < 3; i++) {
                glDrawArrays(GL_LINE_LOOP, i*3, 3);
            }
        }

        // Draw hkl plane faces
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        sp_basic.fUseProgram();
        glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), 0.5, 0.5, 0.5, 0.5);
        for (PlaneObject const& plane:planes)    
        {    
            if (plane.objectDisplay)
            { 
                glStencilFunc(GL_ALWAYS, plane.objectID + 11, -1); 
                plane.fRenderFill();
            }
        }
        glEnable(GL_CULL_FACE);

        // Draw TT faces
        if (showTT)
        {
            TT.fBind();
            sp_basic.fUseProgram();
            for (int i = 0; i < 4; i++) {
                glStencilFunc(GL_ALWAYS, i + 7, -1);
                glDrawArrays(GL_TRIANGLES, i*3, 3);
            }

            glUseProgram(sp_text.programID);
            glm::mat4 m_projection_text = glm::ortho(0.0f, MainViewPortSize[0], 0.0f, MainViewPortSize[1]);
            glUniformMatrix4fv(glGetUniformLocation(sp_text, "projection"), 1, GL_FALSE, glm::value_ptr(m_projection_text));
            glm::mat4 m_get_label_coord = projection * view * model_crystal;
            glm::vec3 label_coord = m_get_label_coord * glm::vec4(vertex_labels[i] / 2, vertex_labels[i + 1] / 2, vertex_labels[i + 2] / 2, vertex_labels[i + 3]);
            RenderCharCentred(
                sp_text,
                vertx_labels_str[i / 4],
                (MainViewPortSize[0] / 2)* (1.0f + label_coord.x),
                (MainViewPortSize[1] / 2)* (1.0f + label_coord.y),
                0.7f, glm::vec3(1.0f));
        }

        // Draw crystal faces
        if (showFaces)
        {
            cube.fBind();
            sp_basic.fUseProgram();
            glUniformMatrix4fv(glGetUniformLocation(sp_basic.programID, "model"), 1, GL_FALSE, glm::value_ptr(model_crystal));
            for (int i = 0; i < 6; i++) {
                glStencilFunc(GL_ALWAYS, i + 1, -1);
                glDrawArrays(GL_TRIANGLES, i*6, 6);
            }
        }

        // Highlight selected object
        glLineWidth(3.0f);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST | GL_STENCIL_TEST);
        if (selectedObjectID > 0 && selectedObjectID <= 6)
        {
            cube.fBind();
            sp_basic.fUseProgram();
            glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), 0.0, 1.0, 0.0, 1.0);
            
            glDrawArrays(GL_LINE_LOOP, (selectedObjectID-1)*6, 6);
        }
        else if (selectedObjectID > 6 && selectedObjectID <= 10)
        {
            TT.fBind();
            sp_basic.fUseProgram();
            glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), 0.0, 1.0, 0.0, 1.0);
            glDrawArrays(GL_LINE_LOOP, (selectedObjectID - 7)*3, 3);
        }

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        // Draw Gizmo
        if (showGizmo)
        {
            gizmo.fBind();
            sp_basic.fUseProgram();
            glUniformMatrix4fv(glGetUniformLocation(sp_basic.programID, "model"), 1, GL_FALSE, glm::value_ptr(model_gizmo));
            glLineWidth(5.0f);

            glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), 1.0, 0.0, 0.0, 1.0);
            glDrawArrays(GL_LINES, 0, 2);

            glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), 0.0, 1.0, 0.0, 1.0);
            glDrawArrays(GL_LINES, 2, 2);

            glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), 0.0, 0.0, 1.0, 1.0);
            glDrawArrays(GL_LINES, 4, 2);
        }

        glLineWidth(1.0f);

        // Draw BB
        if (showBB)
        {
            sp_basic.fUseProgram();
            glUniformMatrix4fv(glGetUniformLocation(sp_basic.programID, "model"), 1, GL_FALSE, glm::value_ptr(model_bb));
            glUniform4f(glGetUniformLocation(sp_basic.programID, "wireColor"), bbColor.r, bbColor.g, bbColor.b, bbColor.a);
            BB.fRender();
        }
        sp_text.fUseProgram();
        glm::mat4 mmm = glm::ortho(0.0f, MainViewPortSize[0], 0.0f, MainViewPortSize[1]);
        glUniformMatrix4fv(glGetUniformLocation(sp_text.programID, "projection"), 1, GL_FALSE, glm::value_ptr(mmm));
        //-----------------------------------------------------------------------------------------------------------------

        model_gizmo = glm::mat4(1.0f);
        if (showGizmo)
        {
            sp_basic.fUseProgram();
            float l = 1.1;
            glm::mat4 m_projection_gizmo = glm::ortho(-l, l, -l, l, -10.0f, 100.0f);
            glm::mat4 m_view = view * glm::rotate(glm::rotate(glm::mat4(1.0f),
                glm::radians(-90.0f), { 1.0f, 0.0f, 0.0f }),
                glm::radians(-90.0f), { 0.0f, 0.0f, 1.0f });
            glUniformMatrix4fv(glGetUniformLocation(sp_basic.programID, "view"), 1, GL_FALSE, glm::value_ptr(m_view));
            glUniformMatrix4fv(glGetUniformLocation(sp_basic.programID, "projection"), 1, GL_FALSE, glm::value_ptr(m_projection_gizmo));
            glUniformMatrix4fv(glGetUniformLocation(sp_basic.programID, "model"), 1, GL_FALSE, glm::value_ptr(model_gizmo));
            glViewport(MainViewPortSize[0] - GizmoSize, MainViewPortSize[1] - GizmoSize, GizmoSize, GizmoSize);
            gizmo.fBind();
            glLineWidth(4.0f);
            glEnable(GL_DEPTH_TEST);
            for (int i = 0; i < 3; i++)
            {
                glUniform4fv(glGetUniformLocation(sp_basic.programID, "wireColor"), 1, glm::value_ptr(color_edge_gizmo[i]));
                glDrawArrays(GL_LINES, i * 2, 2);
            }
            glLineWidth(2.0f);

            glDisable(GL_DEPTH_TEST);
            sp_text.fUseProgram();
            glm::mat4 m_transform_to_text_coord = m_projection_gizmo * model_gizmo * m_view;
            glm::vec3 x_axis = (GizmoSize / 2) + (GizmoSize / 2) * (m_transform_to_text_coord * glm::vec4(0.9, 0.0, 0.0, 1.0));
            glm::vec3 y_axis = (GizmoSize / 2) + (GizmoSize / 2) * (m_transform_to_text_coord * glm::vec4(0.0, 0.9, 0.0, 1.0));
            glm::vec3 z_axis = (GizmoSize / 2) + (GizmoSize / 2) * (m_transform_to_text_coord * glm::vec4(0.0, 0.0, 0.9, 1.0));
            glm::mat4 m_projection_gizmo_text = glm::ortho(0.0f, GizmoSize, 0.0f, GizmoSize);
            glUniformMatrix4fv(glGetUniformLocation(sp_text.programID, "projection"), 1, GL_FALSE, glm::value_ptr(m_projection_gizmo_text));
            RenderCharCentered(sp_text.programID, 'x', x_axis.x, x_axis.y, 0.5f, glm::vec3(1.0f));
            RenderCharCentered(sp_text.programID, 'y', y_axis.x, y_axis.y, 0.5f, glm::vec3(1.0f));
            RenderCharCentered(sp_text.programID, 'z', z_axis.x, z_axis.y, 0.5f, glm::vec3(1.0f));
            glViewport(0, 0, MainViewPortSize[0], MainViewPortSize[1]);
        }


        //----------------------------------------Blit fb_main to fb_intermediate------------------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fb_main);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_intermediate);
        glBlitFramebuffer(
            0, 0, MainViewPortSize[0], MainViewPortSize[1], 
            0, 0, MainViewPortSize[0], MainViewPortSize[1], 
            GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        //-----------------------------------------------------------------------------------------------------------------


        //----------------------------------------Render ImGui Windows-----------------------------------------------------
    
        // Bind FBO and clear screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        M3Dwindow.fClearScreen(bgColor.x, bgColor.y, bgColor.z);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, ImGuiDockNodeFlags_PassthruCentralNode);

        // Show the big demo window 
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Orientation Window
        {
            ImGuiWindowFlags EulerAnglesWindowFlags = ImGuiWindowFlags_NoResize;

            ImGui::Begin("Orientation", 0, EulerAnglesWindowFlags);

            ImGui::SliderFloat(u8"\u03D5\u2081", &cubic.orientation.x, 0.0f, 360.0f, "%0.2f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SameLine();  if (ImGui::Button(u8"Reset \u03D5\u2081")) cubic.orientation.x = 0.0f;
            ImGui::SliderFloat(u8"\u03C6 ", &cubic.orientation.y, 0.0f, 180.0f, "%0.2f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SameLine();  if (ImGui::Button(u8"Reset \u03C6 ")) cubic.orientation.y = 0.0f;
            ImGui::SliderFloat(u8"\u03D5\u2082", &cubic.orientation.z, 0.0f, 360.0f, "%0.2f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SameLine();  if (ImGui::Button(u8"Reset \u03D5\u2082")) cubic.orientation.z = 0.0f;

            if (ImGui::Button("Reset all")) cubic.orientation = glm::vec3(0.0f);

            static const char* ideal_tex[] = {
                            "{001}<100> Cube",
                            "{112}<111> Copper",
                            "{123}<634> S",
                            "{110}<112> Bs",
                            "{110}<001> G",
                            "{113}<332> D",
                            "{013}<100> CubeRD",
                            "{001}<310> CubeND",
                            "{001}<310> BR",
                            "{258}<121> U(transition)",
                            "{124}<211> R",
                            "{011}<122> P",
                            "{013}<231> Q",
                            "{001}<110> Rotated cube",
                            "{525}<151> A1",
                            "{323}<131> A2",
                            "{110}<011> A3"
            };
            static const glm::vec3 ideal_tex_orientation[] =
            {
                { 0.0f,  0.0f,  0.0f},
                {90.0f, 30.0f, 45.0f},
                {59.0f, 37.0f, 63.0f},
                {35.0f, 45.0f,  0.0f},
                { 0.0f, 45.0f,  0.0f},
                {90.0f, 25.0f, 45.0f},
                { 0.0f, 22.0f,  0.0f},
                {22.0f,  0.0f,  0.0f},
                {80.0f, 31.0f, 34.0f},
                {45.0f, 35.0f, 20.0f},
                {53.0f, 36.0f, 60.0f},
                {65.0f, 45.0f,  0.0f},
                {45.0f, 15.0f, 10.0f},
                { 0.0f,  0.0f, 45.0f},
                {15.0f, 47.0f, 38.0f},
                {23.0f, 50.0f, 56.0f},
                {60.0f, 55.0f, 45.0f}
            };
            static int tex_current = 0;
            ImGui::Text("Ideal texture components");
            if (ImGui::Combo("##IdealTextureComponents", &tex_current, ideal_tex, IM_ARRAYSIZE(ideal_tex)))
            {
                cubic.orientation = ideal_tex_orientation[tex_current];
            }

            ImGui::SameLine();  
            ImGui::Checkbox("Show local Axes", &showGizmo);

            ImGui::Separator();
            ImGui::Checkbox("Global bounding box", &showBB);

            ImGui::Separator();
            ImGui::Text("Look Along:");
            if (ImGui::Button("X"))
            {
                M3Dcam0.mYaw = -90.0f; M3Dcam0.mPitch = 0.0f;
                M3Dcam0.fHandleRotationTurntable(0.0f, 0.0f);
            } ImGui::SameLine();
            if (ImGui::Button("Y"))
            {
                M3Dcam0.mYaw = 180.0f; M3Dcam0.mPitch = 0.0f;
                M3Dcam0.fHandleRotationTurntable(0.0f, 0.0f);
            } ImGui::SameLine();
            if (ImGui::Button("Z"))
            {
                M3Dcam0.mYaw = -90.0f; M3Dcam0.mPitch = -90.0f;
                M3Dcam0.fHandleRotationTurntable(0.0f, 0.0f);
            }

            ImGui::SameLine();
            if (ImGui::Button("Isometric##cam"))
            {
                M3Dcam0.mDir = glm::normalize(glm::vec3({ -0.57735026919, -0.57735026919, -0.57735026919 }));
                M3Dcam0.mPos = -M3Dcam0.mDir* glm::length(M3Dcam0.mPos);
                M3Dcam0.mPitch = -35.0f; M3Dcam0.mYaw = -135.0f;
            }

            ImGui::End();
        }

        // Symmetry
        {
            ImGui::Begin("Symmetry");
            ImGui::Text("Point Group = 432");
            ImGui::End();
        }

        // Display Properties
        {
            ImGui::Begin("Display Properties");

            ImGui::Checkbox("Show Faces", &showFaces);

            ImGui::ColorEdit4("Edge Color", glm::value_ptr(wireColor));
        
            ImGui::Separator();
            ImGui::ColorEdit4("BB Color", glm::value_ptr(bbColor));

            ImGui::Separator();
            ImGui::Text("Theme:");
            ImGui::SameLine();
            if (ImGui::Button("Classic"))
            {
                ImGui::StyleColorsClassic();
                bbColor = {1.0f, 1.0f, 0.5f, 0.3f};
                bgColor = {0.0f, 0.0f, 0.0f};
                wireColor = { 0.0f, 0.0f, 0.0f, 0.6f };
            }
            ImGui::SameLine();
            if (ImGui::Button("Light")) 
            { 
                ImGui::StyleColorsLight(); 
                bbColor = { 0.0f, 0.0f, 0.0f, 0.4f };
                bgColor = { 0.8f, 0.8f, 0.8f};
                wireColor = { 0.0f, 0.0f, 0.0f, 0.7f };
            }
            ImGui::SameLine();
            if (ImGui::Button("Dark")) 
            {
                ImGui::StyleColorsDark(); 
                bbColor = { 1.0f, 1.0f, 1.0f, 0.3f };
                bgColor = { 0.0f, 0.0f, 0.0f };
                wireColor = { 0.9f, 1.0f, 0.0f, 0.7f };
            }

            ImGui::ColorEdit3("Background Color", glm::value_ptr(bgColor));
        
            ImGui::Separator();
            ImGui::Checkbox("Enable orthogonal projection", &projectOrtho);

            ImGui::Separator();
            ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);

            //ImGui::Text("Yaw = %0.2f Pitch = %0.2f", M3Dcam0.mYaw, M3Dcam0.mPitch);
            //ImGui::Text("%0.2f %0.2f %0.2f", M3Dcam0.mDir.x, M3Dcam0.mDir.y, M3Dcam0.mDir.z);

            //ImGui::Text("%0.2f %0.2f", lastMouse[0], lastMouse[1]);
            //ImGui::Text("%d", selectedObjectID);


            ImGui::End();
        }

        // Lattice Properties
        {
            bool latticeHasChanged = false;

            ImGui::Begin("Lattice");

            ImGui::Text("Lattice parameters (A, deg):");
            if (ImGui::InputFloat("a",        &cubic.lattice.a    ))    latticeHasChanged = true;
            if (ImGui::InputFloat("b",        &cubic.lattice.b    ))    latticeHasChanged = true;
            if (ImGui::InputFloat("c",        &cubic.lattice.c    ))    latticeHasChanged = true;
            if (ImGui::InputFloat(u8"\u03b1", &cubic.lattice.alpha))    latticeHasChanged = true;
            if (ImGui::InputFloat(u8"\u03b2", &cubic.lattice.beta ))    latticeHasChanged = true;
            if (ImGui::InputFloat(u8"\u03b3", &cubic.lattice.gamma))    latticeHasChanged = true;

            if (latticeHasChanged)
            { 
                cubic.fGenerateCrystalRenderData(cubeData, ttData);
                glNamedBufferSubData(bCube, 0, sizeof(cubeData), cubeData);
                glNamedBufferSubData(bTT, 0, sizeof(ttData), ttData);
            
                for (PlaneObject& pl:planes)  {    pl.fUpdateRenderData();   }
            }

            ImGui::Separator();
            ImGui::Text("Lattice basis vectors:");
            ImGui::Text("A:\t% 04.2f,\t% 04.2f,\t% 04.2f", cubic.lattice.basis[0].x, cubic.lattice.basis[0].y, cubic.lattice.basis[0].z);
            ImGui::Text("B:\t% 04.2f,\t% 04.2f,\t% 04.2f", cubic.lattice.basis[1].x, cubic.lattice.basis[1].y, cubic.lattice.basis[1].z);
            ImGui::Text("C:\t% 04.2f,\t% 04.2f,\t% 04.2f", cubic.lattice.basis[2].x, cubic.lattice.basis[2].y, cubic.lattice.basis[2].z);
            ImGui::Text("Lattice system: %s", LS_enum2str(cubic.lattice.mLatticeSystem).c_str());

            ImGui::Separator();
            ImGui::Checkbox("Thompson Tetrahedron", &showTT);

            ImGui::End();
        }

        // Add planes window
        {
            ImGui::Begin("Add Planes");
        
            for (int i = 0; i < planes.size(); i++)
            {
                ImGui::PushID(planes[i].objectID);

                ImGui::Checkbox("", &planes[i].objectDisplay);

                ImGui::SameLine();
                if (ImGui::InputInt3("hkl", planes[i].hklval)) 
                { 
                    planes[i].fUpdateRenderData(); 
                }

                ImGui::SameLine();
                if (ImGui::Button(" - ")) 
                { 
                    ImGui::PopID(); 
                    planes.erase(std::remove(planes.begin(), planes.end(), planes[i]), planes.end());
                    break; 
                }

                ImGui::PopID();
            }
        
            if (ImGui::Button("Add + "))
            {
                planes.push_back(PlaneObject(&cubic));
            }
        
            ImGui::End();
        }

        // Model window
        {
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

            ImGui::Begin("Model");

            if (ImGui::IsWindowHovered()) cursorOverMainViewport = 1;
            else cursorOverMainViewport = 0;

            ImVec2 vmin = ImGui::GetWindowContentRegionMin();
            ImVec2 vmax = ImGui::GetWindowContentRegionMax();
            ImVec2 sizexy = { vmax.x - vmin.x, vmax.y - vmin.y };
            mainAspect = sizexy.y > 0 ? sizexy.x / sizexy.y : 1.0f;

            ImVec2 pos = ImGui::GetCursorScreenPos();

            float texCoord[2] = {min(1.0, MainViewPortSize[0]/ MaxMainSize[0]), min(1.0, MainViewPortSize[1]/ MaxMainSize[1])};

            ImGui::GetWindowDrawList()->AddImage(
                (ImTextureID)screenTexture,
                ImVec2(ImGui::GetCursorScreenPos()),
                ImVec2(ImGui::GetCursorScreenPos().x + sizexy.x, ImGui::GetCursorScreenPos().y + sizexy.y), 
                ImVec2(0, texCoord[1]), ImVec2(texCoord[0], 0));

            MainViewPortPos[0] = pos.x; MainViewPortPos[1] = pos.y;

            if (MainViewPortSize[0] != sizexy.x || MainViewPortSize[1] != sizexy.y)
            {
                MainViewPortSize[0] = sizexy.x; MainViewPortSize[1] = sizexy.y;
                MainViewPortSizeHasChanged = 1;
            }
            else
                MainViewPortSizeHasChanged = 0;

            ImGui::End();
            ImGui::PopStyleVar();
        }
        
        // Stereogram window
        {
            ImGui::Begin("Stereographic Projection");
            ImVec2 vmin = ImGui::GetWindowContentRegionMin();
            ImVec2 vmax = ImGui::GetWindowContentRegionMax();
            ImVec2 sizexy = { vmax.x - vmin.x, vmax.y - vmin.y };
            float size = sizexy.x < sizexy.y ? sizexy.x : sizexy.y;
            float texCoord = min(1.0f, StereoViewPortSize / MaxStereoSize);
            ImGui::Image((ImTextureID)stereoTex, { size, size }, ImVec2(0, texCoord), ImVec2(texCoord, 0));
            if (size == StereoViewPortSize)
            {
                StereoViewPortSizeHasChanged = 0;
            }
            else
            {
                StereoViewPortSize = size;
                StereoViewPortSizeHasChanged = 1;
            }

            //ImGui::Text("Grid density:");
            //ImGui::SetNextItemWidth(200.0f);
            //ImGui::SliderInt("Azimuthal", &azimuthalGridDensity, 0, 10);
            //ImGui::SetNextItemWidth(200.0f);
            //ImGui::SliderInt("Polar", &polarGridDensity, 0, 10);

            int hkl[3] = { m1.uvw.x, m1.uvw.y, m1.uvw.z };
            if (ImGui::InputInt3("hkl", hkl))
            {
                m1.uvw.x = hkl[0];
                m1.uvw.y = hkl[1];
                m1.uvw.z = hkl[2];

                eqv = s432 * m1;
            }
            ImGui::Text("Multiplicity = %d\n", (int)eqv.size());
            ImGui::End();
        }


        // Output window
        {
            ImGui::Begin("Output");
            ImGui::Text("Debug info:");
            ImGui::Text("%d", selectedObjectID);
            if(cursorOverMainViewport)
                ImGui::Text(
                    "%d, %d", 
                    GLint(lastMouse[0] - MainViewPortPos[0]), 
                    GLint(MainViewPortSize[1] - (lastMouse[1] - MainViewPortPos[1]) - 1)
                );

            for (PlaneObject const& plane : planes)
            {
                if (selectedObjectID == plane.objectID + 11)
                {
                    ImGui::Text("Plane [hkl = %d %d %d]", plane.hklval[0], plane.hklval[1], plane.hklval[2]);
                    break;
                }
            }
            ImGui::End();
        }
    
        // Rendering ImGui VBOs
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll events
        M3Dwindow.fUpdateState();

        // Key input
        if (glfwGetKey(M3Dwindow.mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(M3Dwindow.mWindow, true);
    }

    // Cleanup
    ImGuiLayer_Quit();
    M3Dwindow.fQuit();
}