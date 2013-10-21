#pragma once

/*! \file codes.h
  \brief Return codes of interfaces.
  \author cxnf
  \version 1
  \date 2013-10-21
  \copyright GNU Public License
*/

/*! \enum codes
  \brief Return codes.
  Return codes used by interfaces.
*/
enum codes {
  Success           = 0,                          //!< Function finished successfully.

  Failed,                                         //!< Failed to finish. Function could recover (if required) and context object (if present) is in a valid state.

  NullPointer,                                    //!< Null pointer was given where valid pointer was required.
  MemAlloc,                                       //!< Failed to allocate memory.
  InvalidParam,                                   //!< One or more parameters contained invalid data.
  InvalidBuffer,                                  //!< Buffer was invalid or contained invalid fields.
};
