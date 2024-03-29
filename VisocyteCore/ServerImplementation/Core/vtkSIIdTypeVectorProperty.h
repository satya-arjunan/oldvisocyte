/*=========================================================================

  Program:   Visocyte
  Module:    vtkSIIdTypeVectorProperty.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkSIIdTypeVectorProperty
 *
 * IdType ServerSide Property use to set IdType array as method parameter.
*/

#ifndef vtkSIIdTypeVectorProperty_h
#define vtkSIIdTypeVectorProperty_h

#include "vtkPVServerImplementationCoreModule.h" //needed for exports
#include "vtkSIVectorProperty.h"
#include "vtkSIVectorPropertyTemplate.h" // real superclass

#ifndef __WRAP__
#define vtkSIVectorProperty vtkSIVectorPropertyTemplate<vtkIdType, bool>
#endif
class VTKPVSERVERIMPLEMENTATIONCORE_EXPORT vtkSIIdTypeVectorProperty : public vtkSIVectorProperty
#ifndef __WRAP__
#undef vtkSIVectorProperty
#endif
{
public:
  static vtkSIIdTypeVectorProperty* New();
  vtkTypeMacro(vtkSIIdTypeVectorProperty, vtkSIVectorProperty);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkSIIdTypeVectorProperty();
  ~vtkSIIdTypeVectorProperty() override;

private:
  vtkSIIdTypeVectorProperty(const vtkSIIdTypeVectorProperty&) = delete;
  void operator=(const vtkSIIdTypeVectorProperty&) = delete;
};

#endif
