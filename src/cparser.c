#include "cparser.h"

/*! \file cparser.c
  \brief Basic text stream tokenizer.
  \author cxnf
  \version 1
  \date 2013-10-21
  \copyright GNU Public License
*/

#include "clist.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

// ----------------- Struct definitions -----------------------------------

/*! \struct ParseContext
  \brief Context of tokenizer.
*/
struct ParseContext {
  FILE * file;                                    //!< File parsed by tokenizer.
  List * list;                                    //!< List to mimic dynamic string.
  int iLine;                                      //!< Line number of tokenizer.
  int iColumn;                                    //!< Column number of tokenizer.
  parserCallback fnCallback;                      //!< Callback for external token processing.
};


// ----------------- Local Function declarations --------------------------

/*! \brief Report error.
  Reports an error to stdin.
  \param ccaMessage Message to print.
  \param cppcContext Context of the tokenizer.
*/
static inline void reportError(const char * ccaMessage, const struct ParseContext * cppcContext);
/*! \brief Free memory.
  Frees all memory allocated by tokenizer for current context.
  \param ppcContext Context to free.
*/
static inline void cleanUp(struct ParseContext * ppcContext);
/*! \brief Report error and clean up.
  Reports an error to stdin and frees memory.
  \param ccaMessage Message to print.
  \param ppcContext Context of the tokenizer.
*/
static inline void reportAndClean(const char * ccaMessage, struct ParseContext * ppcContext);

/*! \brief Reads a character.
  Reads a character from the parsed file and consumes it.
  \param ppcContext Context of the tokenizer.
  \return Char of EOF.
*/
static inline int read(struct ParseContext * ppcContext);

/*! \brief Handles EOF.
  Checks 'character' for EOF.
  When EOF callback is called with 'failure' as type and \0 as token.
  \param character Char to check.
  \param failure Type of failure token.
  \param ppcContext Context of the tokenizer.
  \return Success if not EOF, else error code.
*/
static inline enum ParseResults endOnEOF(int character, enum TokenType failure, struct ParseContext * ppcContext);

/*! \brief Sends expression to callback.
  Sends an expression to the callback.
  \param type Type of token.
  \param ppcContext Context of tokenizer.
  \return Parse result.
*/  
static enum ParseResults sendExpression(enum TokenType type, struct ParseContext * ppcContext);


// ----------------- Global Function definitions --------------------------
enum ParseResults parseFile(const char * path, parserCallback fnCallback) {
  struct ParseContext pcContext;
  pcContext.file = fopen(path, "r");
  if (!pcContext.file) {
    return 0;
  }
  
  pcContext.list = createList(NULL);
  pcContext.iLine = 1;
  pcContext.iColumn = 0;
  pcContext.fnCallback = fnCallback;
  enum ParseResults result = ROk;

  // cast c to void* and pass to list when adding
  // a copy of the char is then kept by the list as pointer
  // while the pointer IS the actual char
  // as long as c is larger then zero, this does not matter for the list
  int c = read(&pcContext);
  while (c != EOF) {
    int unkown = 1;

    // select token catagory
    // NO TOKEN
    if (c == '#' && pcContext.iColumn == 1) {
      unkown = 0;

      do {
	if ((c = read(&pcContext)) == EOF) {
	  return ROk;
	}
      } while (c != '\n' && c != '\r');
      do {
	if ((c = read(&pcContext)) == EOF) {
	  return ROk;
	}
      } while (c == '\n' || c == '\r');

      ++pcContext.iLine;
      pcContext.iColumn = 1;
    }

    // END LINE
    if (c == '\n' || c == '\r') {
      unkown = 0;

      c = read(&pcContext);
      if ((result = endOnEOF(c, TTEndLine, &pcContext))) {
	return result;
      }
      if (c == '\n') {
	c = read(&pcContext);
	if ((result = endOnEOF(c, TTEndLine, &pcContext))) {
	  return result;
	}
      }
      if (!(*pcContext.fnCallback)(TTEndLine, "\0")) {
	reportAndClean("Parsing cancelled", &pcContext);
	return RErrCanceled;
      }

      ++pcContext.iLine;
      pcContext.iColumn = 1;
    }
    
    // NUMBER (start with number or sign) (dot is only allowed after a number AND when a number follows)
    if (c == '-' || (c >= '0' && c <= '9')) {
      // number state
      char hashead = 0;
      char hasdot = 0;
      char hastail = 0;
      unkown = 0;
      
      if (c == '-') {
	addEntry(pcContext.list, getEnd(pcContext.list), (void *)c);
	c = read(&pcContext);
	if ((result = endOnEOF(c, TTNumber, &pcContext))) {
	  return result;
	}
      }

      while ((c >= '0' && c <= '9') || c == '.') {
	if (c == '.') {
	  if (hasdot || !hashead) {
	    reportAndClean("Invalid number", &pcContext);
	    return RErrInvalidToken;
	  }
	  hasdot = 1;
	} else if (!hashead) {
	  hashead = 1;
	} else {
	  hastail = 1;
	}
	addEntry(pcContext.list, getEnd(pcContext.list), (void *)c);
	c = read(&pcContext);
	if ((result = endOnEOF(c, TTNumber, &pcContext))) {
	  return result;
	}
      }

      if (!hastail && hasdot) {
	reportAndClean("Invalid number", &pcContext);
	return RErrInvalidToken;
      }
      
      sendExpression(TTNumber, &pcContext);
      clearList(pcContext.list);
    }

    // TEXT (starts at a-zA-Z_ stops at whitespace)
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
      unkown = 0;
 
      while (!isspace(c)) {
	addEntry(pcContext.list, getEnd(pcContext.list), (void *)c);
	c = read(&pcContext);
	if ((result = endOnEOF(c, TTText, &pcContext))) {
	  return result;
	}
      }
      
      sendExpression(TTText, &pcContext);
      clearList(pcContext.list);
    }
    
    if (unkown) {
      c = read(&pcContext);
      if ((result = endOnEOF(c, TTEndLine, &pcContext))) {
	return result;
      }
    }
  }

  cleanUp(&pcContext);
  return ROk;
}


// ----------------- Local Function definitions ---------------------------
static inline void reportError(const char * ccaMessage, const struct ParseContext * cppcContext) {
  printf("Errror: %s at line %d[%d]\n", ccaMessage, cppcContext->iLine, cppcContext->iColumn);
}
static inline void cleanUp(struct ParseContext * ppcContext) {
  destroyList(ppcContext->list);
  fclose(ppcContext->file);
}
static inline void reportAndClean(const char * ccaMessage, struct ParseContext * ppcContext) {
  reportError(ccaMessage, ppcContext);
  cleanUp(ppcContext);
}

static inline int read(struct ParseContext * ppcContext) {
  ++ppcContext->iColumn;
  return fgetc(ppcContext->file);
}

static inline enum ParseResults endOnEOF(int character, enum TokenType failure, struct ParseContext * ppcContext) {
  if (character == EOF) {
    if (!(*ppcContext->fnCallback)(failure, "\0")) {
      reportAndClean("Parsing cancelled", ppcContext);
      return RErrCanceled;
    }
  }
  return ROk;
}

static enum ParseResults sendExpression(enum TokenType type, struct ParseContext * ppcContext) {
  int iSize;
  if ((iSize = getSize(ppcContext->list)) > 0) {
    char * token = (char *)malloc(sizeof(char) * (iSize + 1));
    int i;
    Iterator * iter;
    for (i = 0, iter = getBegin(ppcContext->list); i < iSize && iter; ++i, moveNext(&iter)) {
      token[i] = (char)getCurrent(iter);
    }
    token[i] = '\0';
    if (!(*ppcContext->fnCallback)(type, (Token)token)) {
      return RErrCanceled;
    }
    return ROk;
  } else {
    return RErrInvalidToken;
  }  
}
