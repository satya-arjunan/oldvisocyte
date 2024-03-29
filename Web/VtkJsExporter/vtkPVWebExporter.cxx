/*=========================================================================

  Program:   Visocyte
  Module:    vtkPVWebExporter.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkPVWebExporter.h"

#include "vtkObjectFactory.h"

#if VTK_MODULE_ENABLE_VTK_PythonInterpreter && VTK_MODULE_ENABLE_VTK_Python &&                     \
  VTK_MODULE_ENABLE_VTK_WrappingPythonCore
#include "vtkPython.h"
#include "vtkPythonInterpreter.h"
#include "vtkPythonUtil.h"
#include "vtkSmartPyObject.h"
#endif

vtkStandardNewMacro(vtkPVWebExporter);
//----------------------------------------------------------------------------
vtkPVWebExporter::vtkPVWebExporter()
{
  this->VisocyteGlanceHTML = nullptr;
}

//----------------------------------------------------------------------------
vtkPVWebExporter::~vtkPVWebExporter()
{
  delete[] this->VisocyteGlanceHTML;
}

//----------------------------------------------------------------------------
void vtkPVWebExporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkPVWebExporter::Write()
{
  this->Superclass::Write();

#if VTK_MODULE_ENABLE_VTK_PythonInterpreter && VTK_MODULE_ENABLE_VTK_Python &&                     \
  VTK_MODULE_ENABLE_VTK_WrappingPythonCore

  // ensure Python interpreter is initialized.
  vtkPythonInterpreter::Initialize();
  vtkPythonScopeGilEnsurer gilEnsurer;
  try
  {
    vtkSmartPyObject module(PyImport_ImportModule("vtk.web.vtkjs_helper"));
    if (!module || PyErr_Occurred())
    {
      vtkGenericWarningMacro("Failed to import vtk.web.vtkjs_helper module.");
      throw 1;
    }

    PyObject_CallMethod(module, const_cast<char*>("convertDirectoryToZipFile"),
      const_cast<char*>("(s)"), const_cast<char*>(this->FileName));
    if (PyErr_Occurred())
    {
      vtkGenericWarningMacro("Failed to bundle vtkjs file");
      throw 1;
    }

    if (this->VisocyteGlanceHTML)
    {
      PyObject_CallMethod(module, const_cast<char*>("addDataToViewer"), const_cast<char*>("(ss)"),
        const_cast<char*>(this->FileName), const_cast<char*>(this->VisocyteGlanceHTML));
    }
    if (PyErr_Occurred())
    {
      vtkGenericWarningMacro("Failed to bundle vtkjs data file into HTML viewer");
      throw 1;
    }
  }
  catch (int)
  {
    if (PyErr_Occurred())
    {
      PyErr_Print();
      PyErr_Clear();
    }
  }
#endif
}
