#ifndef GTYPES_H
#define GTYPES_H

/*! \file gtypes.h
  \brief Graphics datatypes and operator.
  \author Edwin Rommen
  \version 0.1
  \date 2013/09/25
  \copyright GNU Public License.
*/


#include "dtypes.h"
#include <stdint.h>


typedef uint16_t Color;                           //!< Define 'color' type.


// ----------------- Structs -----------------------------------------------------------------------

/*! \struct Vertex
  \brief Vertex datatype.
  Vertex with components: coord, color.
*/
typedef struct Vertex {
  Vector coord;                                   //!< Vertex coordinate.
  Color color;                                    //!< Color.
} Vertex;

/*! \struct VertexBuffer
  \brief Vertex buffer datastruct.
  Vertex buffer containing all vertices of a mesh.
*/
typedef struct VertexBuffer {
  Vertex * vertices;                              //!< Vertices in buffer.
  uint16_t size;                                  //!< Amount of vertices in buffer.
} VertexBuffer;

/*! \struct IndexBuffer
  \brief Index buffer datastruct.
  Index buffer containing all indices of a mesh.
  Every line is determined by the indices at an even index and following odd index (2 indices per line).
*/
typedef struct IndexBuffer {
  uint16_t * indices;                             //!< Indices in buffer.
  uint16_t size;                                  //!< Amount of indices in buffer. If this isn't an even number, something went horribly wrong and the buffer is corrupt.
} IndexBuffer;

/*! \struct Mesh
  \brief Mesh datatype.
  Mesh type combines a vertex and index buffer, providing a convenient way to keep the two buffers together.
*/
typedef struct Mesh {
  VertexBuffer vertices;
  IndexBuffer indices;
} Mesh;


typedef void (*meshIterator)(Vertex const *, Vertex const *); //!< Callback function to iterate through all lines in a mesh.


// ----------------- Vertex Functions --------------------------------------------------------------


// ----------------- VertexBuffer Functions --------------------------------------------------------

/*! \brief Initializes a vertex buffer.
  Initializes a vertex buffer for specified amount of vertices.
  \param size Amount of vertices that will fit in the buffer.
  \param vb Pointer to vertex buffer to initialize.
  \return Result code.
  \see codes
*/
enum codes initVertexBuffer(uint16_t size, VertexBuffer * vb);

/*! \brief Gets a vertex.
  Returns a pointer to the vertex at given index.
  \param index Zero based index of vertex.
  \param vb Pointer to vertex buffer.
  \return Pointer to vertex.
*/
Vertex * getVertex(uint16_t index, VertexBuffer * vb);


// ----------------- IndexBuffer Functions --------------------------------------------------------

/*! \brief Initializes a index buffer.
  Initializes an index buffer for specified amount of lines.
  Be aware that every line requires 2 indices, resulting in a buffer that is twice the given size!
  \param lines Amount of lines that will fit in the buffer.
  \param ib Pointer to index buffer to initialize.
  \return Result code.
  \see codes
*/
enum codes initIndexBuffer(uint16_t lines, IndexBuffer * ib);


// ----------------- Mesh Functions ----------------------------------------------------------------

/*! \brief Iterates through all lines.
  Calls 'fnIterator' for each line found in the given mesh.
  \param mesh Pointer to mesh.
  \param fnIterator Callback function for each line.
  \return Result code.
  \see codes
*/
enum codes iterateLines(Mesh const * mesh, meshIterator fnIterator);


#endif // GTYPES_H
