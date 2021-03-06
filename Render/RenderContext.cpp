/*
 *  RenderContext.cpp
 *  Engine
 *
 *  Created by Brent Wilson on 4/4/07.
 *  Copyright 2007 Brent Wilson. All rights reserved.
 *
 */

#include "GL_Helper.h"
#include "RenderContext.h"
#include "Viewport.h"
#include "Texture.h"
#include "Shader.h"

RenderContext::RenderContext():
    _viewport(0, 0, 0, 0),
    _renderableCount(0),
    _primitiveCount(0),
    _vertexCount(0)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // The default values are fucked. Set them to avoid issues in Get/SetpolygonMode.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    setCullMode(BACK);
    setTransparency(false);
    setDepthTest(true);
    setWireframe(false);

#if SYS_PLATFORM != PLATFORM_APPLE
    glewExperimental = true;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        Error(glewGetErrorString(err));
    }
#endif

    // build the renderer info string
    char version[256], renderer[256];
    int i;
    strncpy(renderer, (const char *)glGetString(GL_RENDERER), 255);
    if (0 == strcmp(renderer, "Generic")) {
        strncpy(renderer, "Apple Generic", 255);
    }
    else if ((strlen(renderer) > 14) && (0 == strcmp(renderer+strlen(renderer)-14, " OpenGL Engine"))) {
        renderer[strlen(renderer)-14] = 0;
    }
    strncat(renderer, " ", 255);
    strncpy(version, (const char *)glGetString(GL_VERSION), 255);
    for (i = 0; i < 256; i++) {
        char c[2] = { version[i], 0};
        if (((c[0] >= '0' ) && (c[0] <= '9')) || c[0] == '.') strncat(renderer, c, 255);
        else break;
    }
    Info("Renderer: " << renderer);

}

RenderContext::~RenderContext() {}

void RenderContext::setViewport(const Viewport &viewport) {
    glViewport(viewport.xPos, viewport.yPos, viewport.width, viewport.height);
    _viewport = viewport;
}

const Viewport & RenderContext::getViewport() const {
    return _viewport;
}

void RenderContext::setGlobalAmbient(const Color4 &ambientColor) {
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor.ptr());
    CheckGLErrors();
}

void RenderContext::clear(const Color4 &clearColor) {
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CheckGLErrors();
}

void RenderContext::setFog(const Color4 &color, float density, float fogStartDist, float fogEndDist) {
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogfv(GL_FOG_COLOR, color.array);
    glFogf(GL_FOG_DENSITY, density);
    glHint(GL_FOG_HINT, GL_NICEST);
    glFogf(GL_FOG_START, fogStartDist);
    glFogf(GL_FOG_END, fogEndDist);
    glEnable(GL_FOG);

    CheckGLErrors();
}

void RenderContext::setProjectionMatrix(const Matrix &mat) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(mat.getMatrix());
    glMatrixMode(GL_MODELVIEW);
    CheckGLErrors();
}

void RenderContext::setModelViewMatrix(const Matrix &mat) {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mat.getMatrix());
    CheckGLErrors();
}

void RenderContext::render2D(int width, int height, RenderableList &list) {
    LightList emptyList;

    // Turn off depth testing for ortho
    // XXXBMW: TODO - I don't like this. I'd much rather have a push/pop scenario built into RenderParameterContainer...
    bool oldVal = getDepthTest();
    setDepthTest(false);

    // Call the standard render function with ortho parameters
    render(Matrix::Identity(), Matrix::Ortho(0, width, 0, height), list, emptyList);

    // Reset depth testing to what it was before
    setDepthTest(oldVal);
}

void RenderContext::render(const Matrix &view, const Matrix &projection, RenderableList &list, LightList &lights) {
    // Assume the correct render target is enabled and has been cleared if it needs to be.

    // Set the projection and view matrices, to give our Shaders access to a
    // ViewProjection matrix.
    setProjectionMatrix(projection);

    // Set the modelview matrix to our view matrix to make sure lights are accounting for
    // the camera when they're enabled.
    if (lights.size()) {
        glEnable(GL_LIGHTING);
        setModelViewMatrix(view);
        LightList::iterator lightItr = lights.begin();
        for (int i = 0; lightItr != lights.end(); i++, lightItr++) {
            (*lightItr)->enable(i);
        }
    } else {
        glDisable(GL_LIGHTING);
    }

    // Ignore any ShaderParameters people may have set. Who knows why they set them here?
    pushParameters(NULL);

    // The Renderable operator< defines how sorting happens. Here, we assume it's based on
    // Material. A system that can just intelligently track ALL current state and just not
    // change things unless required would be ideal, but this works for now. Also, only
    // sort if we're doing depth testing, otherwise order matters.
    if (getDepthTest()) {
        list.sort();
    }

    CheckGLErrors();

    bool newlyActive = false;
    Material *active = NULL;
    RenderableList::iterator itr;
    for (itr = list.begin(); itr != list.end(); itr++) {
        // Update the material, if we need to. Be sure to do this BEFORE calling
        // preRenderNotice on the Renderable, to avoid RenderParameterContainer push/pop
        // issues.
        if ((*itr)->getMaterial() != active) {
            if (active) {
                active->disable();
            }

            active = (*itr)->getMaterial();

            active->enable();

            newlyActive = true;
        }

        // Call this right now, as preRenderNotice may result in changes to the
        // Renderable's internal RenderOperation.
        (*itr)->preRenderNotice();

        _renderableCount += 1;

        // Only do this work if there is a vertex array and we have vertices in that array.
        if ((*itr)->getRenderOperation() &&
            (*itr)->getRenderOperation()->getVertexArray() &&
            (*itr)->getRenderOperation()->getVertexCount())
        {
            _primitiveCount += (*itr)->getRenderOperation()->getPrimitiveCount();
            _vertexCount += (*itr)->getRenderOperation()->getVertexCount();

            // This relies on the RenderOperation, so it should be done *after* the
            // preRenderNotice, but it also should only be done when a new material is set,
            // so we use the newlyActive variable.
            if (newlyActive) {
                ///\todo I feel pretty strongly that this should be handled differently...
                /// I'd like to do it once, and forget about it. This requires more research
                /// into how XNA and others handle it.
                (*itr)->getMaterial()->getShader()->bindAttributesToChannel(
                    (*itr)->getRenderOperation()->getVertexArray()->getVertexArrayLayout());
                newlyActive = false;
            }

            // Render the Renderable. Don't use its render method because it sets the Material.
            setModelViewMatrix(view * (*itr)->getModelMatrix());

            (*itr)->getRenderOperation()->render();
        }

        (*itr)->postRenderNotice();
    }

    if (active) {
        active->disable();
    }

    // Always match the push!
    popParameters();

    if (lights.size()) {
        glDisable(GL_LIGHTING);
        LightList::iterator lightItr = lights.begin();
        for (; lightItr != lights.end(); lightItr++) {
            (*lightItr)->disable();
        }
    }

    CheckGLErrors();
}

void RenderContext::renderTexture(Texture *src) {
    // Get the scene ready.
    clear(Color4(1, 1, 1, 1));
    setProjectionMatrix(Matrix::Ortho(0, 1, 0, 1));
    setModelViewMatrix(Matrix::Identity());

    CheckGLErrors();

    // Setup the vertices.
    float vertices[] = {
        0, 0, 0, 0,
        1, 0, 1, 0,
        1, 1, 1, 1,
        0, 1, 0, 1
    };

    glVertexPointer(2, GL_FLOAT, 2, vertices);
    glTexCoordPointer(2, GL_FLOAT, 2, vertices + 2); // TODO Not sure if this is correct.

    // Do the drawing.
    if (src) { src->enable(); }
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glDrawArrays(GL_QUADS, 0, 4);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    if (src) { src->disable(); }

    CheckGLErrors();
}

int RenderContext::getRenderableCount() const { return _renderableCount; }
int RenderContext::getPrimitiveCount() const { return _primitiveCount; }
int RenderContext::getVertexCount() const { return _vertexCount; }

void RenderContext::resetCounts() {
    _renderableCount = 0;
    _primitiveCount = 0;
    _vertexCount = 0;
}









//void RenderContext::renderBoundingBox(const AABB3 &boundingBox, const Color4 &color) {
//    _primitiveCount += 12;
//    _vertexCount += 18;
//    _modelCount++;
//
//    glColor4f(color.r, color.g, color.b, color.a);
//    const Vector3 &min = boundingBox.getMin();
//    const Vector3 &max = boundingBox.getMax();
//
//    glLineWidth(1);
//    glBegin(GL_LINE_STRIP); {
//        glVertex3f(min[0], min[1], max[2]);
//        glVertex3f(max[0], min[1], max[2]);
//        glVertex3f(max[0], max[1], max[2]);
//        glVertex3f(min[0], max[1], max[2]);
//        glVertex3f(min[0], min[1], max[2]);
//    } glEnd();
//
//    glBegin(GL_LINE_STRIP); {
//        glVertex3f(min[0], min[1], min[2]);
//        glVertex3f(max[0], min[1], min[2]);
//        glVertex3f(max[0], max[1], min[2]);
//        glVertex3f(min[0], max[1], min[2]);
//        glVertex3f(min[0], min[1], min[2]);
//    } glEnd();
//
//    glBegin(GL_LINES); {
//        glVertex3f(min[0], min[1], min[2]);
//        glVertex3f(min[0], min[1], max[2]);
//
//        glVertex3f(max[0], max[1], min[2]);
//        glVertex3f(max[0], max[1], max[2]);
//
//        glVertex3f(min[0], max[1], min[2]);
//        glVertex3f(min[0], max[1], max[2]);
//
//        glVertex3f(max[0], min[1], min[2]);
//        glVertex3f(max[0], min[1], max[2]);
//    } glEnd();
//}
//
//void RenderContext::renderTriangles(Vector3 *vertices, int number, const Color4 &color) {
//    _primitiveCount += number;
//    _vertexCount += number * 3;
//    _modelCount++;
//
///// \todo Make this use not immediate mode!!!!!!!!!!
//
//    glColor4f(color.r, color.g, color.b, color.a);
//    glBegin(GL_TRIANGLES); {
//        for (int i = 0; i < number * 3; i++) {
//            glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
//        }
//    } glEnd();
//}
//
//void RenderContext::resetModelviewMatrix() {
//    setModelMatrix(Matrix::Identity());
//    setViewMatrix(Matrix::Identity());
//}
//
//void RenderContext::setViewport(int x, int y, int width, int height) const {
//    //Info("Setting viewport to: " << x << " " << y << " " << width << " " << height);
//    glViewport(x, y, width, height);
//}
//
//void RenderContext::clearBuffers(const Color4 &clearColor) const {
//    //Info("Clearing buffers to: " << clearColor);
//    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//}
//
/////\todo Get rid of these!
//void RenderContext::addToPrimitiveCount(int count) const { _primitiveCount += count; }
//void RenderContext::addToVertexCount(int count) const { _vertexCount += count; }
//void RenderContext::addToModelCount(int count) const { _modelCount += count; }
//
//void RenderContext::resetMetrics() {
//    _primitiveCount = 0;
//    _vertexCount = 0;
//    _modelCount = 0;
//}
//
//void RenderContext::setProjectionMatrix(const Matrix &projection) {
//    _projectionMatrix = projection;
//
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glMultMatrixf(projection.getMatrix());
//    glMatrixMode(GL_MODELVIEW);
//}
//
//const Matrix& RenderContext::getModelviewMatrix() {
//    return _modelviewMatrix;
//}
//
//void RenderContext::setViewMatrix(const Matrix &viewMat) {
//    _viewMatrix = viewMat;
//    _modelviewMatrix = _viewMatrix * _modelMatrix;
//    glLoadMatrixf(_modelviewMatrix.getMatrix());
//}
//
//void RenderContext::setModelMatrix(const Matrix &modelMat) {
//    _modelMatrix = modelMat;
//    _modelviewMatrix = _viewMatrix * _modelMatrix;
//    glLoadMatrixf(_modelviewMatrix.getMatrix());
//}
//
//Matrix RenderContext::getModelviewProjectionMatrix() {
//    return _projectionMatrix * _modelviewMatrix;
//}
