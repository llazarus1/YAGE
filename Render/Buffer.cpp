/*
 *  Buffer.cpp
 *  Mountainhome
 *
 *  Created by loch on 12/17/10.
 *  Copyright 2010 Mountainhome Project. All rights reserved.
 *
 */

#include "Buffer.h"

#include <Base/Assertion.h>

Buffer::Buffer(
    GLenum bufferType,
    GLenum accessType,
    GLenum dataType,
    unsigned int elementSize,
    unsigned int elementCount,
    void *data
):
    _bufferType(bufferType),
    _accessType(accessType),
    _dataType(dataType),
    _componentsPerElement(elementSize),
    _elementCount(elementCount),
    _elementCapacity(0),
    _bytesPerComponent(0),
    _handle(0),
    _byteCount(0)
{
    switch(_dataType) {
        case GL_BYTE:
            // Don't use signed indices. No reason for it.
            ASSERT(_bufferType != GL_ELEMENT_ARRAY_BUFFER);
        case GL_UNSIGNED_BYTE:
            _bytesPerComponent = sizeof(char);
            break;
        case GL_SHORT:
            // Don't use signed indices. No reason for it.
            ASSERT(_bufferType != GL_ELEMENT_ARRAY_BUFFER);
        case GL_UNSIGNED_SHORT:
            _bytesPerComponent = sizeof(short);
            break;
        case GL_INT:
            // Don't use signed indices. No reason for it.
            ASSERT(_bufferType != GL_ELEMENT_ARRAY_BUFFER);
        case GL_UNSIGNED_INT:
            _bytesPerComponent = sizeof(int);
            break;
        case GL_FLOAT:
            // Can't have floating point indices.
            ASSERT(_bufferType != GL_ELEMENT_ARRAY_BUFFER);
            _bytesPerComponent = sizeof(float);
            break;
        case GL_DOUBLE:
            // Can't have floating point indices.
            ASSERT(_bufferType != GL_ELEMENT_ARRAY_BUFFER);
            _bytesPerComponent = sizeof(double);
            break;
        default:
            THROW(InvalidStateError, "The specified type is invalid.");
    }

    allocate(_elementCount, data);
}

Buffer::~Buffer() {
    glDeleteBuffers(1, &_handle);
}

void *Buffer::mapBuffer(GLenum accessType) {
    return glMapBuffer(_handle, accessType);
}

bool Buffer::unmapBuffer() {
    return glUnmapBuffer(_handle);
}

void Buffer::setData(void *data, int elementCount) {
    resize(elementCount, false);
    glBufferSubData(_handle, 0, elementCount, data);
}

void Buffer::resize(int elementCount, bool saveData) {
    _elementCount = elementCount;
    if (_elementCount > _elementCapacity) {
        reserve(_elementCount, saveData);
    }
}

void Buffer::reserve(int elementCapacity, bool saveData) {
    int oldHandle = _handle;
    void *data = NULL;

    if (saveData) {
        data = glMapBuffer(oldHandle, GL_READ_ONLY);
    }

    allocate(elementCapacity, data);

    if (saveData) {
        glUnmapBuffer(oldHandle);
    }

    glDeleteBuffers(1, &oldHandle);
}

void Buffer::allocate(int elementCapacity, void *data) {
    _elementCapacity = elementCapacity;

    if (_elementCapacity < _elementCount) {
        _elementCount = _elementCapacity;
    }

    _byteCount = _bytesPerComponent * _componentsPerElement * _elementCapacity;

    glGenBuffers(1, &_handle);
    glBindBuffer(_bufferType, _handle);
    glBufferData(_bufferType, _byteCount, data, _accessType);
    glBindBuffer(_bufferType, 0);
}

GLenum Buffer::getAccessType() {
    return _accessType;
}

GLenum Buffer::getDataType() {
    return _dataType;
}

unsigned int Buffer::getElementCount() {
    return _elementCount;
}

unsigned int Buffer::getElementCapacity() {
    return _elementCapacity;
}
