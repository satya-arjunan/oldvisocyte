/*=========================================================================

  Program:   Visocyte
  Module:    vtkPVServerManagerPluginInterface.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkPVServerManagerPluginInterface
 *
 * vtkPVServerManagerPluginInterface defines the interface needed to be
 * implemented by a server-manager plugin i.e. a plugin that adds new
 * filters/readers/proxies etc. to Visocyte.
*/

#ifndef vtkPVServerManagerPluginInterface_h
#define vtkPVServerManagerPluginInterface_h

#include "vtkClientServerInterpreterInitializer.h"
#include "vtkPVClientServerCoreCoreModule.h" //needed for exports
#include <string>                            // STL Header
#include <vector>                            // STL Header

class VTKPVCLIENTSERVERCORECORE_EXPORT vtkPVServerManagerPluginInterface
{
public:
  virtual ~vtkPVServerManagerPluginInterface();

  /**
   * Obtain the server-manager configuration xmls, if any.
   */
  virtual void GetXMLs(std::vector<std::string>& vtkNotUsed(xmls)) = 0;

  //@{
  /**
   * Returns the callback function to call to initialize the interpretor for the
   * new vtk/server-manager classes added by this plugin. Returning NULL is
   * perfectly valid.
   */
  virtual vtkClientServerInterpreterInitializer::InterpreterInitializationCallback
  GetInitializeInterpreterCallback() = 0;
};
//@}

#endif

// VTK-HeaderTest-Exclude: vtkPVServerManagerPluginInterface.h
