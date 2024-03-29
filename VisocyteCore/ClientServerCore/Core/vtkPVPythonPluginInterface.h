/*=========================================================================

  Program:   Visocyte
  Module:    vtkPVPythonPluginInterface.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkPVPythonPluginInterface
 *
 * vtkPVPythonPluginInterface defines the interface required by Visocyte plugins
 * that add python modules to Visocyte.
*/

#ifndef vtkPVPythonPluginInterface_h
#define vtkPVPythonPluginInterface_h

#include "vtkObject.h"
#include "vtkPVClientServerCoreCoreModule.h" //needed for exports
#include <string>                            // STL Header
#include <vector>                            // STL Header

class VTKPVCLIENTSERVERCORECORE_EXPORT vtkPVPythonPluginInterface
{
public:
  virtual ~vtkPVPythonPluginInterface();

  virtual void GetPythonSourceList(std::vector<std::string>& modules,
    std::vector<std::string>& sources, std::vector<int>& package_flags) = 0;
};

#endif

// VTK-HeaderTest-Exclude: vtkPVPythonPluginInterface.h
