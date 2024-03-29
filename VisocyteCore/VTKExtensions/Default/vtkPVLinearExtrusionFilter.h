/*=========================================================================

  Program:   Visocyte
  Module:    vtkPVLinearExtrusionFilter.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkPVLinearExtrusionFilter
 * @brief   change a default value
 *
 * vtkPVLinearExtrusionFilter is a subclass of vtkPLinearExtrusionFilter.
 * The only difference is changing the default extrusion type to vector
 * extrusion
*/

#ifndef vtkPVLinearExtrusionFilter_h
#define vtkPVLinearExtrusionFilter_h

#include "vtkPLinearExtrusionFilter.h"
#include "vtkPVVTKExtensionsDefaultModule.h" //needed for exports

class VTKPVVTKEXTENSIONSDEFAULT_EXPORT vtkPVLinearExtrusionFilter : public vtkPLinearExtrusionFilter
{
public:
  static vtkPVLinearExtrusionFilter* New();
  vtkTypeMacro(vtkPVLinearExtrusionFilter, vtkPLinearExtrusionFilter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkPVLinearExtrusionFilter();
  ~vtkPVLinearExtrusionFilter() override{};

private:
  vtkPVLinearExtrusionFilter(const vtkPVLinearExtrusionFilter&) = delete;
  void operator=(const vtkPVLinearExtrusionFilter&) = delete;
};

#endif
