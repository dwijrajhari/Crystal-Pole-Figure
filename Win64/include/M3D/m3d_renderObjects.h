#ifndef M3D_RENDEROBJECTS
#define M3D_RENDEROBJECTS

#include <vector>
#include <M3D/m3d_vao.h>
#include <glm/glm.hpp>

class RenderObject
{
public:
    VertexArrayObject vaObj;
    unsigned int vbID, objectID;

	bool objectDisplay  = 1;
	bool objectSelected = 0;

	virtual void fUpdateRenderData() = 0;
	virtual void fRender() const = 0;
    virtual void fRenderHighlighted() const = 0;

	bool operator==(RenderObject const &object2);

    RenderObject();
};

class Crystal;

class PlaneObject : public RenderObject
{
public:
    float renderData[12];

    Crystal* crystal;

    int hklval[3];
    bool planeISTri = true;
    glm::vec4 faceColor = { 0.5, 0.5, 0.5, 0.5 };
    glm::vec4 edgeColor = { 0.5, 0.5, 0.5, 0.5 };
    
    
    PlaneObject(Crystal* cr, int h_ = 0, int k_ = 0, int l_ = 0);

    void fRender() const override;
    void fRenderFill() const;
    void fRenderFrame() const;
    void fRenderHighlighted() const  override;
    void fUpdateRenderData() override;
    void fUpdatehkl(int h_, int k_, int l_);
    glm::vec3 fGetNormal() const;
    glm::vec2 fGetPole() const;

};

class AABBObject : public RenderObject
{
public:
    AABBObject();
    void fRender() const override;
    void fRenderHighlighted() const  override;
    void fUpdateRenderData() override;
};

class GizmoObject : public RenderObject
{
public:
    GizmoObject();
    void fRender() const override;
    void fRenderHighlighted() const  override;
    void fUpdateRenderData() override;
};


#endif
