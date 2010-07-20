/*
 *  SceneNode.cpp
 *  System
 *
 *  Created by loch on 2/1/10.
 *  Copyright 2010 Brent Wilson. All rights reserved.
 *
 */

#include <Base/Assertion.h>
#include "SceneNode.h"
#include "Camera.h"

const std::string SceneNode::TypeName = "SceneNode";

SceneNode::SceneNode(const std::string &name):
_dirty(true), _fixedYawAxis(true), _yawAxis(0,1,0), _derivedPosition(0.0), _position(0.0),
_parent(NULL), _type(TypeName), _name(name) {}

SceneNode::SceneNode(const std::string &name, const std::string &type):
_dirty(true), _fixedYawAxis(true), _yawAxis(0,1,0), _derivedPosition(0.0), _position(0.0),
_parent(NULL), _type(type), _name(name) {}

SceneNode::~SceneNode() {
    if (_parent) {
        _parent->dettach(this);
    }
}

void SceneNode::addVisibleObjectsToQueue(Camera *camera, RenderQueue *queue) {
    SceneNodeMap::iterator itr = _children.begin();
    for (; itr != _children.end(); itr++) {
        // Only render an entity if some part of it is contained by the frustum.
        if (camera->getFrustum()->checkAABB(itr->second->_derivedBoundingBox)) {
            itr->second->addVisibleObjectsToQueue(camera, queue);
        }
    }
}

const std::string &SceneNode::getName()    const { return _name;   }
const std::string &SceneNode::getType()    const { return _type;   }
SceneNode* SceneNode::getParent() const { return _parent; }

void SceneNode::attach(SceneNode *obj) {
    // Take care of the object's current parent.
    if (obj->_parent) {
        obj->_parent->dettach(obj);
    }

    // Drop it in the _children map and set its new parent.
    _children[obj->getName()] = obj;
    obj->_parent = this;
}

void SceneNode::dettach(SceneNode *obj) {
    _children.erase(obj->getName());
    obj->_parent = NULL;
}

void SceneNode::dettachAllChildren() {
    SceneNodeMap::iterator itr = _children.begin();
    for (; itr != _children.end(); itr++) {
        itr->second->_parent = NULL;
    }

    _children.clear();
}

void SceneNode::updateDerivedValues() {
    if (!isDirty()) { return; }
    if (getParent()) {
        _derivedOrientation = getParent()->_derivedOrientation * _orientation;
        _derivedPosition = getParent()->_derivedPosition + _position;
    } else {
        _derivedOrientation = _orientation;
        _derivedPosition = _position;    
    }

    updateImplementationValues();
    setDirty(false);
} // updateDerivedValues

void SceneNode::updateImplementationValues() {
    if (_children.size() == 0) {
        _derivedBoundingBox.clear();
    } else {
        bool first = true;
        SceneNodeMap::iterator itr = _children.begin();
        for (; itr != _children.end(); itr++) {
            itr->second->updateDerivedValues();
            if (first) { _derivedBoundingBox = itr->second->_derivedBoundingBox; first = false; }
            else       { _derivedBoundingBox.encompass(itr->second->_derivedBoundingBox);       }
        }
    }
}

Matrix SceneNode::getDerivedPositionalMatrix() const {
    Matrix mat(_derivedOrientation);
    mat.setTranslation(_derivedPosition);
    return mat;
}

Matrix SceneNode::getLocalPositionalMatrix() const {
    Matrix mat(_orientation);
    mat.setTranslation(_position);
    return mat;
}

void SceneNode::setDirty(bool value) {
    _dirty = value;
    // Only cascade dirty calls upwards.
    if (_dirty && getParent()) {
        getParent()->setDirty();
    }
}

bool SceneNode::isDirty() const {
    return _dirty;
}

void SceneNode::setOrientation(const Quaternion &newOrientation) {
    _orientation = newOrientation;
    setDirty();
}

void SceneNode::setPosition(Real x, Real y, Real z) {
    setPosition(Vector3(x, y, z));
    setDirty();
}

void SceneNode::setPosition(const Vector3 &newPosition) {
    _position = newPosition;
    setDirty();
}

void SceneNode::moveAbsolute(const Vector3 &difference) {
    _position += difference;
    setDirty();
}

void SceneNode::rotate(const Quaternion &rot)             {
    _orientation = rot * _orientation;
    setDirty();
}

Quaternion SceneNode::getDerivedOrientation() const { return _derivedOrientation; }
Quaternion SceneNode::getLocalOrientation() const { return _orientation; }

Vector3 SceneNode::getDerivedPosition() const { return _derivedPosition; }
Vector3 SceneNode::getLocalPosition() const { return _position; }

void SceneNode::moveRelative(const Vector3 &difference) { moveAbsolute(_orientation * difference); }
void SceneNode::moveForward (Real dist) { moveRelative(Vector3(0, 0, -1) * dist); }

void SceneNode::moveBackward(Real dist) { moveForward(-dist);                     }
void SceneNode::moveUpward  (Real dist) { moveRelative(Vector3(0, 1, 0) * dist);  }
void SceneNode::moveDownward(Real dist) { moveUpward(-dist);                      }
void SceneNode::strafeRight (Real dist) { moveRelative(Vector3(1, 0, 0) * dist);  }
void SceneNode::strafeLeft  (Real dist) { strafeRight(-dist);                     }

void SceneNode::rotate(Radian angle, const Vector3 &axis) { rotate(Quaternion(angle, axis));         }
void SceneNode::rotate(Degree angle, const Vector3 &axis) { rotate(Quaternion(Radian(angle), axis)); }

void SceneNode::adjustPitch(Radian angle) { rotate(angle, _orientation * Vector3(1, 0, 0)); }
void SceneNode::adjustRoll (Radian angle) { rotate(angle, _orientation * Vector3(0, 0, 1)); }
void SceneNode::adjustYaw  (Radian angle) {
    rotate(angle, (_fixedYawAxis ? _yawAxis : _orientation * Vector3(0, 1, 0)));
}

Vector3 SceneNode::getFixedYawAxis() const { return _yawAxis; }
void SceneNode::setFixedYawAxis(bool fixed, const Vector3 &axis) {
    _fixedYawAxis = fixed;
    _yawAxis = axis;
}