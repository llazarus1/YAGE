/*
 *  NormalBuffer.cpp
 *  Mountainhome
 *
 *  Created by loch on 2/25/11.
 *  Copyright 2011 Mountainhome Project. All rights reserved.
 *
 */

#include <Base/Assertion.h>
#include "NormalBuffer.h"

NormalBuffer::NormalBuffer(
    GLenum accessType,
    GLenum dataType,
    unsigned int elementCount,
    void *data
):
    Buffer(
        GL_ARRAY_BUFFER,
        accessType,
        dataType,
        3,
        elementCount,
        data
    )
{}

NormalBuffer::NormalBuffer(
    GLenum accessType,
    GLenum dataType
):
    Buffer(
        GL_ARRAY_BUFFER,
        accessType,
        dataType,
        3
    )
{}

void NormalBuffer::enable() {
    ASSERT(_handle);

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(_bufferType, _handle);

    glNormalPointer(_dataType, 0, 0);

    glBindBuffer(_bufferType, 0);
}

void NormalBuffer::disable() {
    ASSERT(_handle);

    glDisableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, 0);
}
