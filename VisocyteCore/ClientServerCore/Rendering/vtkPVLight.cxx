/*=========================================================================

  Program:   Visocyte
  Module:    vtkPVLight.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPVLight.h"

#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkObjectFactory.h"
#include "vtkPVConfig.h"

#if VTK_MODULE_ENABLE_VTK_RenderingOSPRay
#include "vtkOSPRayLightNode.h"
#endif

vtkInformationKeyMacro(vtkPVLight, LIGHT_NAME, String);

vtkStandardNewMacro(vtkPVLight);
//----------------------------------------------------------------------------
vtkPVLight::vtkPVLight()
{
}

//----------------------------------------------------------------------------
vtkPVLight::~vtkPVLight()
{
}

//----------------------------------------------------------------------------
void vtkPVLight::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkPVLight::SetRadius(double r)
{
#if VTK_MODULE_ENABLE_VTK_RenderingOSPRay
  vtkOSPRayLightNode::SetRadius(r, this);
#else
  (void)r;
#endif
}

//----------------------------------------------------------------------------
void vtkPVLight::SetName(const std::string& name)
{
  GetInformation()->Set(vtkPVLight::LIGHT_NAME(), name);
}

//----------------------------------------------------------------------------
std::string vtkPVLight::GetName()
{
  if (GetInformation())
  {
    return GetInformation()->Get(vtkPVLight::LIGHT_NAME());
  }
  return "";
}

//----------------------------------------------------------------------------
void vtkPVLight::SetLightType(int t)
{
  if (t < VTK_LIGHT_TYPE_AMBIENT_LIGHT)
  {
    this->Superclass::SetLightType(t);
#if VTK_MODULE_ENABLE_VTK_RenderingOSPRay
    vtkOSPRayLightNode::SetIsAmbient(0, this);
#endif
  }
  else
  {
    this->Superclass::SetLightType(VTK_LIGHT_TYPE_HEADLIGHT);
#if VTK_MODULE_ENABLE_VTK_RenderingOSPRay
    vtkOSPRayLightNode::SetIsAmbient(1, this);
#endif
  }
}
