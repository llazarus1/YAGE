/*
 *  NormalBuffer.cpp
 *  Mountainhome
 *
 *  Created by loch on 2/25/11.
 *  Copyright 2011 Mountainhome Project. All rights reserved.
 *
 */

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

void NormalBuffer::enable() {
    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(_bufferType, _handle);

    glNormalPointer(_dataType, 0, 0);

    glBindBuffer(_bufferType, 0);
}

void NormalBuffer::disable() {
    glDisableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, 0);
}
