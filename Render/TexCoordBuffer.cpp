/*
 *  TexCoordBuffer.cpp
 *  Mountainhome
 *
 *  Created by loch on 2/25/11.
 *  Copyright 2011 Mountainhome Project. All rights reserved.
 *
 */

#include "TexCoordBuffer.h"
#include <Base/Assertion.h>

TexCoordBuffer::TexCoordBuffer(
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
    ),
    _activeChannel(-1)
{}

void TexCoordBuffer::enable(int channel) {
    ASSERT(_activeChannel == -1);
    _activeChannel = channel;

    glClientActiveTexture(GL_TEXTURE0 + _activeChannel);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindBuffer(_bufferType, _handle);

    glTexCoordPointer(_componentsPerElement, _dataType, 0, 0);

    glBindBuffer(_bufferType, 0);
}

void TexCoordBuffer::disable() {
    ASSERT(_activeChannel != -1);

    glClientActiveTexture(GL_TEXTURE0 + _activeChannel);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glTexCoordPointer(4, GL_FLOAT, 0, 0);

    _activeChannel = -1;
}
