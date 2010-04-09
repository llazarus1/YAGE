/*
 *  MHMHIndexedWorldModel.cpp
 *  Mountainhome
 *
 *  Created by loch on 4/8/10.
 *  Copyright 2010 Mountainhome Project. All rights reserved.
 *
 */

#include <Render/RenderContext.h>

#include "MHIndexedWorldModel.h"

MHIndexedWorldModel::MHIndexedWorldModel(unsigned int *indices, int indexCount, Vector3 *verts, Vector3 *norms, Vector2 *texCoords, int vertexCount):
MHWorldModel(verts, norms, texCoords, vertexCount), _indices(indices), _indexCount(indexCount) {}

MHIndexedWorldModel::~MHIndexedWorldModel() {
    delete []_indices; _indices = NULL;
}

void MHIndexedWorldModel::render(RenderContext *context) {
    context->addToPrimitiveCount(_indexCount / 3 * 2);
    context->addToVertexCount(_count * 2);
    context->addToModelCount(1);

    if (_verts) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, _verts);
    }

    if (_norms) {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, _norms);
    }

    if (_texCoords) {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, _texCoords);
    }

    glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, _indices);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    if (0 && _norms) {
        glDisable(GL_LIGHTING);

        glUseProgramObjectARB(0);

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_2D, NULL);
        glDisable(GL_TEXTURE_2D);

        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture(GL_TEXTURE_2D, NULL);
        glDisable(GL_TEXTURE_2D);

        glBegin(GL_LINES);
        for (int i = 0; i < _count; i++) {
            float color = pow(_norms[i].z, 5.0);
            glColor4f(1.0, color, color, 1.0);
            glVertex3fv(_verts[i].array);
            glVertex3fv((_verts[i] + _norms[i]).array);
        }
        glEnd();

        glEnable(GL_LIGHTING);
    }
}