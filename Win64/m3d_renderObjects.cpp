#include <M3D/m3d_basics.h>
#include <M3D/m3d_renderObjects.h>
#include <M3D/m3d_stereographic.h>
#include <M3D/m3d_crystals.h>


bool RenderObject::operator==(RenderObject const &object2)
{
	if (objectID == object2.objectID) { return true; }
	else { return false; }
}

RenderObject::RenderObject()
{
    objectID = vaObj.objectID;
}

PlaneObject::PlaneObject(Crystal* cr, int h_, int k_, int l_)
    :crystal(cr)
{
    hklval[0] = h_; hklval[1] = k_; hklval[2] = l_;
    crystal->fGeneratePlane(renderData, hklval[0], hklval[1], hklval[2], &planeISTri);
    unsigned int layout[] = { 3 };
    vaObj.fAddArrayBuffer(&vbID, sizeof(renderData), renderData, 3 * sizeof(float), 1, layout);
}
void PlaneObject::fRender() const
{
    fRenderFill();
    fRenderFrame();
}
void PlaneObject::fRenderFill() const
{
    vaObj.fBind();
    if (planeISTri)
    {
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    else
    {
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);
}
void PlaneObject::fRenderFrame() const
{
    vaObj.fBind();
    if (planeISTri)
    {
        glDrawArrays(GL_LINE_LOOP, 0, 3);
    }
    else
    {
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }
    glBindVertexArray(0);
}
void PlaneObject::fUpdatehkl(int h_, int k_, int l_)
{
    hklval[0] = h_; hklval[1] = k_; hklval[2] = l_;
    fUpdateRenderData();
}

glm::vec3 PlaneObject::fGetNormal() const
{
    glm::vec3 normal = 
        (float)hklval[0] * crystal->lattice.reciprocal[0] +
        (float)hklval[1] * crystal->lattice.reciprocal[1] +
        (float)hklval[2] * crystal->lattice.reciprocal[2];
    return glm::normalize(normal);
}

void PlaneObject::fRenderHighlighted() const
{
}

void PlaneObject::fUpdateRenderData()
{
    crystal->fGeneratePlane(renderData, hklval[0], hklval[1], hklval[2], &planeISTri);
    glNamedBufferSubData(vbID, 0, sizeof(renderData), renderData);
}

glm::vec2 PlaneObject::fGetPole() const
{
    return fProjectPoint(fGetNormal());
}

AABBObject::AABBObject()
{
    GLfloat VerticesBB[108] = {
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,

     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,

    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f
    };
    unsigned int layout[] = { 3 };
    vaObj.fAddArrayBuffer(&vbID, sizeof(VerticesBB), VerticesBB, 3 * sizeof(float), 1, layout);
}

void AABBObject::fRender() const
{
    vaObj.fBind();
    glDrawArrays(GL_LINE_STRIP, 0, 24);
    glDrawArrays(GL_LINE_STRIP, 24, 12);
    glBindVertexArray(0);
}

void AABBObject::fRenderHighlighted() const
{
}

void AABBObject::fUpdateRenderData()
{
}

GizmoObject::GizmoObject()
{
    GLfloat VerticesGizmo[18] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };
    unsigned int layout[] = { 3 };
    vaObj.fAddArrayBuffer(&vbID, sizeof(VerticesGizmo), VerticesGizmo, 3 * sizeof(float), 1, layout);
}

void GizmoObject::fRender() const
{
    vaObj.fBind();
    glDrawArrays(GL_LINE_STRIP, 0, 24);
    glDrawArrays(GL_LINE_STRIP, 24, 12);
    glBindVertexArray(0);
}

void GizmoObject::fRenderHighlighted() const
{
}

void GizmoObject::fUpdateRenderData()
{
}
