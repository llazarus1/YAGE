/*
 *  PositionBuffer.cpp
 *  Mountainhome
 *
 *  Created by loch on 2/25/11.
 *  Copyright 2011 Mountainhome Project. All rights reserved.
 *
 */

#include <Base/Assertion.h>
#include "PositionBuffer.h"

PositionBuffer::PositionBuffer(
    GLenum accessType,
    GLenum dataType,
    unsigned int elementSize,
    unsigned int elementCount,
    void *data
):
    Buffer(
        GL_ARRAY_BUFFER,
        accessType,
        dataType,
        elementSize,
        elementCount,
        data
    )
{}

PositionBuffer::PositionBuffer(
    GLenum accessType,
    GLenum dataType,
    unsigned int elementSize
):
    Buffer(
        GL_ARRAY_BUFFER,
        accessType,
        dataType,
        elementSize
    )
{}

void PositionBuffer::enable() {
    ASSERT(_handle);

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(_bufferType, _handle);

    glVertexPointer(_componentsPerElement, _dataType, 0, 0);

    glBindBuffer(_bufferType, 0);
}

void PositionBuffer::disable() {
    ASSERT(_handle);

    glDisableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(4, GL_FLOAT, 0, 0);
}
