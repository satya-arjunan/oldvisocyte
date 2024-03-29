/*=========================================================================

  Program:   Visocyte
  Module:    vtkPVDReader.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkPVDReader
 * @brief   Visocyte-specific vtkXMLCollectionReader subclass
 *
 * vtkPVDReader subclasses vtkXMLCollectionReader to add
 * Visocyte-specific methods.
*/

#ifndef vtkPVDReader_h
#define vtkPVDReader_h

#include "vtkPVVTKExtensionsDefaultModule.h" //needed for exports
#include "vtkXMLCollectionReader.h"

class VTKPVVTKEXTENSIONSDEFAULT_EXPORT vtkPVDReader : public vtkXMLCollectionReader
{
public:
  static vtkPVDReader* New();
  vtkTypeMacro(vtkPVDReader, vtkXMLCollectionReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /**
   * Get/Set the required value for the timestep attribute.  The value
   * should be referenced by its index.  Only data sets matching this
   * value will be read.  An out-of-range index will remove the
   * restriction.
   */
  void SetTimeStep(int index) override;
  int GetTimeStep() override;
  //@}

protected:
  vtkPVDReader();
  ~vtkPVDReader() override;

  void ReadXMLData() override;

  int RequestDataObject(vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;

  // Set TimeStepRange
  void SetupOutputInformation(vtkInformation* outInfo) override;

private:
  vtkPVDReader(const vtkPVDReader&) = delete;
  void operator=(const vtkPVDReader&) = delete;
};

#endif
