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
#include <Base/Math3D.h>

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
    calculateComponentSize();
    allocate(_elementCount, data);
}

Buffer::Buffer(
    GLenum bufferType,
    GLenum accessType,
    GLenum dataType,
    unsigned int elementSize
):
    _bufferType(bufferType),
    _accessType(accessType),
    _dataType(dataType),
    _componentsPerElement(elementSize),
    _elementCount(0),
    _elementCapacity(0),
    _bytesPerComponent(0),
    _handle(0),
    _byteCount(0)
{
    calculateComponentSize();
}

Buffer::~Buffer() {
    if (_handle) {
        glDeleteBuffers(1, &_handle);
    }
}

void *Buffer::mapBufferData(GLenum accessType) {
    if (_handle) {
        glBindBuffer(_bufferType, _handle);
        return glMapBuffer(_bufferType, accessType);
    }

    return NULL;
}

bool Buffer::unmapBufferData() {
    bool retVal = false;

    if (_handle) {
        retVal = glUnmapBuffer(_bufferType);
        glBindBuffer(_bufferType, 0);
    }

    return retVal;
}

void Buffer::setData(void *data, int elementCount) {
    // Element count defaults to 0 if we're not changing size.
    if (elementCount) {
        resize(elementCount, false);
    }

    ASSERT(_handle);

    glBindBuffer(_bufferType, _handle);
    glBufferSubData(_bufferType, 0, _byteCount, data);
    glBindBuffer(_bufferType, 0);
}

void Buffer::resize(int elementCount, bool saveData) {
    if (elementCount > _elementCapacity) {
        reserve(elementCount, saveData);
    }
    _elementCount = elementCount;
}

void Buffer::reserve(int elementCapacity, bool saveData) {
    if (!_handle) {
        saveData = false;
    }

    unsigned char *data = NULL;

    if (saveData) {
        // We need to create a heap allocation the size of the actual buffer allocation,
        // but we only want to copy up to the size that is actually used.
        unsigned int allocSize = _bytesPerComponent * _componentsPerElement * elementCapacity;
        unsigned int copySize = _bytesPerComponent * _componentsPerElement * _elementCount;
        data = new unsigned char[allocSize];

        glBindBuffer(_bufferType, _handle);
        memcpy(data, glMapBuffer(_bufferType, GL_READ_ONLY), Math::Min(allocSize, copySize));
        ASSERT(glUnmapBuffer(_bufferType));
        glBindBuffer(_bufferType, 0);
    }

    allocate(elementCapacity, data);

    if (saveData) {
        delete[] data;
        data = NULL;
    }

    CheckGLErrors();
}

void Buffer::allocate(int elementCapacity, void *data) {
    _elementCapacity = elementCapacity;

    if (_elementCapacity < _elementCount) {
        _elementCount = _elementCapacity;
    }

    _byteCount = _bytesPerComponent * _componentsPerElement * _elementCapacity;

    if (!_handle) {
        glGenBuffers(1, &_handle);
    }

    glBindBuffer(_bufferType, _handle);
    glBufferData(_bufferType, _byteCount, data, _accessType);
    glBindBuffer(_bufferType, 0);

    CheckGLErrors();
}

void Buffer::calculateComponentSize() {
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
