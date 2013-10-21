#include "parser.h"

#include "clist.h"
#include "cparser.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*! \file parser.c
  \brief Wavefront obj parser.
  \author cxnf
  \version 0.1
  \date 2013-10-21
  \copyright GNU Public License
*/


// ----------------- Parser state ----------------------------------------------------------------

enum Command {
  CmdNone,                                        //!< Look for command.
  CmdWait,                                        //!< Ignore until line end.
  CmdVertex,                                      //!< Vertex parse mode, 3 or 4 before line end.
  CmdFace,                                        //!< Face parse mode, 3 or 4 before line end.
};

typedef struct Context {
  uint8_t counter;                                //!< Counts processed numbers after a command.
  enum Command state;                             //!< Current command state.
  List * verts;                                   //!< List of parsed vertices.
  List * inds;                                    //!< List of parsed faces.
  void * data;                                    //!< Working object. Can be vertex or face.
} Context;

Context context;

// ----------------- cparser callback --------------------------------------------------------------

void parseLine(void) {
  switch (context.state) {
  case CmdFace:
    if (context.counter < 2) {
      free(context.data);
      break;
    }
    int i;
    for (i = 1; i < context.counter; ++i) {
      addEntry(context.inds, getEnd(context.inds), (void *)((uint16_t *)context.data)[i - 1]);
      addEntry(context.inds, getEnd(context.inds), (void *)((uint16_t *)context.data)[i]);
    }
    addEntry(context.inds, getEnd(context.inds), (void *)((uint16_t *)context.data)[i]);
    addEntry(context.inds, getEnd(context.inds), (void *)((uint16_t *)context.data)[0]);
    //TODO: clean dubble lines, where each line is undirected
    free(context.data);
    break;
    
  case CmdVertex:
    if (context.counter != 3 && context.counter != 4) {
      printf("Vertex has not enough components: %d\n", context.counter);
      free(context.data);
      break;
    }
    addEntry(context.verts, getEnd(context.verts), context.data);
    context.data = NULL;
    break;
    
  default: break;
  }

  context.counter = 0;
  context.state = CmdNone;
}

void parseText(Token token) {
  if (!token) {
    return;
  }

  if (strlen(token) > 0) {
    switch (context.state) {
    case CmdWait:
      return;

    case CmdNone:
      if (!strcmp(token, "v")) {
	context.state = CmdVertex;
	context.data = (Vertex *)malloc(sizeof(Vertex));
	memset(context.data, 0, sizeof(Vertex));
      } else if (!strcmp(token, "f")) {
	context.state = CmdFace;
	context.data = (uint16_t *)malloc(sizeof(uint16_t) * 4);
	memset(context.data, 0, sizeof(uint16_t) * 4);
      } else {
	printf("|%s|:skipped\n", token);
      }
      break;

    case CmdVertex:
    case CmdFace:
      printf("|%s|:invalid\n", token);
      break;

    default:
      printf("|%s|:ignored\n", token);
      context.state = CmdWait;
      break;
    }
  }
}

void parseNumber(Token token) {
  if (!token) {
    return;
  }

  if (strlen(token) > 0) {
    switch (context.state) {
    case CmdVertex: {
      switch (context.counter) {
      case 0:
	((Vertex *)context.data)->coord.x = atof(token);
	break;

      case 1:
	((Vertex *)context.data)->coord.y = atof(token);
	break;

      case 2:
	((Vertex *)context.data)->coord.z = atof(token);
	break;

      default: break;
      }
    }
      break;
      
    case CmdFace: {
      ((uint16_t *)context.data)[context.counter] = atoi(token);
    }
      break;

    default: break;
    }

    ++context.counter;
  }
}


int8_t cparserCallback(enum TokenType type, Token token) {
  switch (type) {
  case TTEndLine:
    parseLine();
    return 2;

  case TTText:
    parseText(token);
    break;

  case TTNumber:
    parseNumber(token);
    break;
  }

  if (context.state == CmdNone) {
    context.state = CmdWait;
  }

  return 1;
}

// ----------------- Functions ---------------------------------------------------------------------

enum codes loadWavefront(char const * path, Mesh * mesh) {
  if (!mesh || !path) {
    return NullPointer;
  }

  context.counter = 0;
  context.state = CmdNone;
  context.inds = createList(NULL);
  context.verts = createList(free);
  context.data = NULL;

  if (parseFile(path, cparserCallback)) {
    destroyList(context.verts);
    destroyList(context.inds);
    return Failed;
  }
  
  mesh->vertices.size = getSize(context.verts);
  mesh->indices.size = getSize(context.inds);
  if (mesh->indices.size % 2 != 0) {
    destroyList(context.verts);
    destroyList(context.inds);
    return Failed;
  }

  mesh->vertices.vertices = (Vertex *)malloc(sizeof(Vertex) * mesh->vertices.size);
  mesh->indices.indices = (uint16_t *)malloc(sizeof(uint16_t) * mesh->indices.size);

  int i;
  Iterator * iter;
  for (i = 0, iter = getBegin(context.verts); iter; ++i, moveNext(&iter)) {
    memcpy(&mesh->vertices.vertices[i], getCurrent(iter), sizeof(Vertex));
  }
  for (i = 0, iter = getBegin(context.inds); iter; ++i, moveNext(&iter)) {
    void * ptr = getCurrent(iter);
    memcpy(&mesh->indices.indices[i], (void *)&ptr, sizeof(uint16_t));
  }
  
  destroyList(context.verts);
  destroyList(context.inds);

  return Success;
}

enum codes destroyWavefront(Mesh * mesh) {
  if (!mesh) {
    return NullPointer;
  }
  if (mesh->vertices.vertices) {
    free(mesh->vertices.vertices);
    mesh->vertices.vertices = NULL;
    mesh->vertices.size = 0;
  }
  if (mesh->indices.indices) {
    free(mesh->indices.indices);
    mesh->indices.indices = NULL;
    mesh->indices.size = 0;
  }
  
  return Success;
}
