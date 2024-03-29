/*=========================================================================

  Program:   Visocyte
  Module:    vtkPVEnsembleDataReaderInformation.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkPVEnsembleDataReaderInformation
 * @brief   Information obeject to
 * collect file information from vtkEnsembleDataReader.
 *
 * Gather information about data files from vtkEnsembleDataReader.
*/

#ifndef vtkPVEnsembleDataReaderInformation_h
#define vtkPVEnsembleDataReaderInformation_h

#include "vtkPVClientServerCoreDefaultModule.h" //needed for exports
#include "vtkPVInformation.h"

class VTKPVCLIENTSERVERCOREDEFAULT_EXPORT vtkPVEnsembleDataReaderInformation
  : public vtkPVInformation
{
public:
  static vtkPVEnsembleDataReaderInformation* New();
  vtkTypeMacro(vtkPVEnsembleDataReaderInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Transfer information about a single object into this object.
   */
  void CopyFromObject(vtkObject*) override;

  //@{
  /**
   * Manage a serialized version of the information.
   */
  void CopyToStream(vtkClientServerStream*) override;
  void CopyFromStream(const vtkClientServerStream*) override;
  //@}

  /**
   * Get number of files contained in the ensemble.
   */
  virtual unsigned int GetFileCount();

  /**
   * Get the file path for the input row index.
   */
  virtual vtkStdString GetFilePath(const unsigned int);

protected:
  vtkPVEnsembleDataReaderInformation();
  ~vtkPVEnsembleDataReaderInformation() override;

private:
  vtkPVEnsembleDataReaderInformation(const vtkPVEnsembleDataReaderInformation&) = delete;
  void operator=(const vtkPVEnsembleDataReaderInformation&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
