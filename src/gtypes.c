#include "gtypes.h"

/*! \file dtypes.c
  \brief Datatypes and operators implementation.
  \author Edwin Rommen
  \version 0.1
  \date 2013/09/26
  \copyright GNU Public License.
*/


#include <stddef.h>
#include <stdlib.h>


// ----------------- VertexBuffer Functions --------------------------------------------------------

enum codes initVertexBuffer(uint16_t size, VertexBuffer * vb) {
  // fail on NULL pointers
  if (!vb) {
    return NullPointer;
  }
  vb->vertices = (Vertex *)malloc(sizeof(Vertex) * size);
  // fail on malloc failure
  if (!vb->vertices) {
    return MemAlloc;
  }
  vb->size = size;
  return Success;
}

Vertex * getVertex(uint16_t index, VertexBuffer * vb) {
  // fail on NULL pointers
  if (!vb) {
    return NULL;
  }
  // fail on out of bounds
  if (index >= vb->size) {
    return NULL;
  }
  return &vb->vertices[index];
}


// ----------------- IndexBuffer Functions --------------------------------------------------------

enum codes initIndexBuffer(uint16_t lines, IndexBuffer * ib) {
  // fail on NULL pointers
  if (!ib) {
    return NullPointer;
  }
  ib->size = lines * 2;
  ib->indices = (uint16_t *)malloc(sizeof(uint16_t) * ib->size);
  // fail on malloc failure
  if (!ib->indices) {
    return MemAlloc;
  }
  return Success;
}


// ----------------- Mesh Functions ----------------------------------------------------------------

enum codes iterateLines(Mesh const * mesh, meshIterator fnIterator) {
  // fail on NULL pointers
  if (!mesh) {
    return NullPointer;
  }
  int i;
  for (i = 0; (i + 1) < mesh->indices.size; i += 2) {
    fnIterator(&mesh->vertices.vertices[mesh->indices.indices[i]], &mesh->vertices.vertices[mesh->indices.indices[i + 1]]);
  }
  return Success;
}
