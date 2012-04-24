/*
 *  Buffer.h
 *  Mountainhome
 *
 *  Created by loch on 12/17/10.
 *  Copyright 2010 Mountainhome Project. All rights reserved.
 *
 */

#ifndef _BUFFER_H_
#define _BUFFER_H_
#include "GL_Helper.h"

/*! A generic graphics hardware buffer. This class is absolutely not meant to be used by
 *  itself. it JUST provides a place to put shared interface logic between the IndexBuffer
 *  class and the GenericAttributeBuffer class.
 * \seealso GenericAttributeBuffer
 * \seealso IndexBuffer */
class Buffer {
public:
    Buffer(
        GLenum bufferType,
        GLenum accessType,
        GLenum dataType,
        unsigned int elementSize,
        unsigned int elementCount,
        void *data = NULL
    );

    virtual ~Buffer();

    void setData(void *data);

    void *mapBufferData(GLenum accessType);

    bool unmapBufferData();

    void resize(int elementCount, bool saveData);

    void reserve(int elementCapacity, bool saveData);

    GLenum getAccessType();

    GLenum getDataType();

    unsigned int getElementCount();

    unsigned int getElementCapacity();

protected:
    /*! Specifies the type of the buffer. */
    GLenum _bufferType;

    /*! How the data in the buffer will be accessed. Determines where the data will live
     *  in memory. Either vidmem, sysmem, or both. */
    GLenum _accessType;

    /*! Represents the type of data contained in the buffer. */
    GLenum _dataType;

    /*! The number of components per element. Must be either 1, 2, 3, or 4. */
    unsigned int _componentsPerElement;

    /*! The number of elements in the buffer. */
    unsigned int _elementCount;

    /*! The number of elements the buffer can hold. */
    unsigned int _elementCapacity;

    /*! The number of bytes in each element component. */
    unsigned int _bytesPerComponent;

    /*! The unique identifier for the buffer. */
    GLuint _handle;

    /*! The number of bytes in the buffer. */
    unsigned int _byteCount;

private:
    void allocate(int elementCapacity, void *data);

};

// Provide a single include point for these files:
#include "GenericAttributeBuffer.h"
#include "IndexBuffer.h"
#include "PositionBuffer.h"
#include "NormalBuffer.h"
#include "TexCoordBuffer.h"

#endif
