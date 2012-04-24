/*
 *  VertexArray.cpp
 *  Mountainhome
 *
 *  Created by loch on 12/17/10.
 *  Copyright 2010 Mountainhome Project. All rights reserved.
 *
 */

#include <Base/Assertion.h>
#include "Buffer.h"

#include "VertexArray.h"
#include "GL_Helper.h"

VertexArray::VertexArray():
    _positions(NULL),
    _normals(NULL)
{
    _texCoords.resize(GetNumTextureUnits(), NULL);
}

VertexArray::~VertexArray() {
    deleteAllBuffers();
}

void VertexArray::deleteAllBuffers() {
    if (_positions) { delete _positions; _positions = NULL; }
    if (_normals)   { delete _normals;   _normals   = NULL; }

    // Delete out the individual textures. Don't clear the vector, though, as it's size
    // should be static.
    for (int i = 0; i < _texCoords.size(); i++) {
        if (_texCoords[i]) {
            delete _texCoords[i];
            _texCoords[i] = NULL;
        }
    }
    // Delete out the individual buffers and clear the whole array.
    for (int i = 0; i < _buffers.size(); i++) {
        if (_buffers[i]) {
            delete _buffers[i];
            _buffers[i] = NULL;
        }
    }

    _buffers.clear();
}

unsigned int VertexArray::getElementCount() const {
    if (getAttributeCount() == 0) { return 0; }
    if (_positions)      { return  _positions->getElementCount(); }
    if (_normals)        { return    _normals->getElementCount(); }
    if (_buffers.size()) { return _buffers[0]->getElementCount(); }

    for (int i = 0; i < _texCoords.size(); i++) {
        if (_texCoords[i]) { return _texCoords[i]->getElementCount(); }
    }

    return 0;
}

unsigned int VertexArray::getElementCapacity() const {
    if (getAttributeCount() == 0) { return 0; }
    if (_positions)      { return  _positions->getElementCapacity(); }
    if (_normals)        { return    _normals->getElementCapacity(); }
    if (_buffers.size()) { return _buffers[0]->getElementCapacity(); }

    for (int i = 0; i < _texCoords.size(); i++) {
        if (_texCoords[i]) { return _texCoords[i]->getElementCapacity(); }
    }

    return 0;
}

unsigned int VertexArray::getAttributeCount() const {
    return _buffers.size() + _texCoords.size() + (_positions ? 1 : 0) + (_normals ? 1 : 0);
}

void VertexArray::resize(int elementCount, bool saveData) {
    if (_positions) { _positions->resize(elementCount, saveData); }
    if (_normals) { _normals->resize(elementCount, saveData); }
    for (int i = 0; i < _texCoords.size(); i++) {
        if (_texCoords[i]) { _texCoords[i]->resize(elementCount, saveData); }
    }
    for (int i = 0; i < _buffers.size(); i++) {
        if (_buffers[i]) { _buffers[i]->resize(elementCount, saveData); }
    }
}

void VertexArray::reserve(int elementCapacity, bool saveData) {
    if (_positions) { _positions->reserve(elementCapacity, saveData); }
    if (_normals) { _normals->reserve(elementCapacity, saveData); }
    for (int i = 0; i < _texCoords.size(); i++) {
        if (_texCoords[i]) { _texCoords[i]->reserve(elementCapacity, saveData); }
    }
    for (int i = 0; i < _buffers.size(); i++) {
        if (_buffers[i]) { _buffers[i]->reserve(elementCapacity, saveData); }
    }
}

unsigned int VertexArray::addGenericAttributeBuffer(const std::string &name, GenericAttributeBuffer *buffer) {
    ASSERT(getElementCapacity() == 0 || getElementCapacity() == buffer->getElementCapacity());
    ASSERT(getElementCount() == 0 || getElementCount() == buffer->getElementCount());

    _buffers.push_back(buffer);
    _names.push_back(name);

    return _buffers.size() - 1;
}

GenericAttributeBuffer * VertexArray::getGenericAttributeBuffer(unsigned int index) const {
    return _buffers[index];
}

GenericAttributeBuffer * VertexArray::getGenericAttributeBuffer(const std::string &name) const {
    for (int i = 0; i < _buffers.size(); i++) {
        if (_names[i] == name) {
            return getGenericAttributeBuffer(i);
        }
    }

    THROW(InternalError, "The named GenericAttributeBuffer does not exist.");
    return NULL;
}

const VertexArrayLayout & VertexArray::getVertexArrayLayout() const {
    return _names;
}

void VertexArray::setPositionBuffer(PositionBuffer *buffer) {
    ASSERT(getElementCapacity() == 0 || getElementCapacity() == buffer->getElementCapacity());
    ASSERT(getElementCount() == 0 || getElementCount() == buffer->getElementCount());
    _positions = buffer;
}

PositionBuffer * VertexArray::getPositionBuffer() {
    return _positions;
}

void VertexArray::setNormalBuffer(NormalBuffer *buffer) {
    ASSERT(getElementCapacity() == 0 || getElementCapacity() == buffer->getElementCapacity());
    ASSERT(getElementCount() == 0 || getElementCount() == buffer->getElementCount());
    _normals = buffer;
}

NormalBuffer * VertexArray::getNormalBuffer() {
    return _normals;
}

void VertexArray::setTexCoordBuffer(int index, TexCoordBuffer *buffer) {
    ASSERT(getElementCapacity() == 0 || getElementCapacity() == buffer->getElementCapacity());
    ASSERT(getElementCount() == 0 || getElementCount() == buffer->getElementCount());
    _texCoords[index] = buffer;
}

TexCoordBuffer * VertexArray::getTexCoordBuffer(int index) {
    return _texCoords[index];
}

void VertexArray::enable() {
    if (_positions) { _positions->enable(); }
    if (_normals) { _normals->enable(); }
    for (int i = 0; i < _texCoords.size(); i++) {
        if (_texCoords[i]) { _texCoords[i]->enable(i); }
    }
    for (int i = 0; i < _buffers.size(); i++) {
        if (_buffers[i]) { _buffers[i]->enable(i); }
    }
}

void VertexArray::disable() {
    if (_positions) { _positions->disable(); }
    if (_normals) { _normals->disable(); }
    for (int i = 0; i < _texCoords.size(); i++) {
        if (_texCoords[i]) { _texCoords[i]->disable(); }
    }
    for (int i = 0; i < _buffers.size(); i++) {
        if (_buffers[i]) { _buffers[i]->disable(); }
    }
}
