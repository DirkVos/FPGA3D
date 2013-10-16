#pragma once

/*! \file parser.h
  \brief Interface of wavefront parser.
  \author Edwin Rommen
  \version 1
  \date 2013-10-16
  \copyright GNU Public License
*/

#include "gtypes.h"                               // Declarations of graphics types.
#include "codes.h"                                // Definitions of all return codes.

/*! \brief Loads a wavefront into memory.
  Reads contents of 'path' and stores the parsed result in 'mesh'.
  The mesh pointed to by 'mesh' should be allocated, vertex and index buffer should not be allocated.
  \param path Path to wavefront file.
  \param mesh Pointer to resulting mesh.
  \return Return code.
*/
enum codes loadWavefront(char const * path, Mesh * mesh);


/*! \brief Destroys a mesh.
  Frees memory allocated by 'loadWavefront'.
  \param mesh Pointer to mesh to free.
*/
enum codes destroyWavefront(Mesh * mesh);
