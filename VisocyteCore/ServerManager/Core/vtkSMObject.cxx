/*=========================================================================

  Program:   Visocyte
  Module:    vtkSMObject.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMObject.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkSMObject);
//---------------------------------------------------------------------------
vtkSMObject::vtkSMObject()
{
}

//---------------------------------------------------------------------------
vtkSMObject::~vtkSMObject()
{
}

//---------------------------------------------------------------------------
void vtkSMObject::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
