/*=========================================================================

  Program:   Visocyte
  Module:    TestCGNSNoFlowSolutionPointers.cxx

  Copyright (c) Menno Deij - van Rijswijk, MARIN, The Netherlands
  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCGNSReader.h"
#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkTestUtilities.h"

#define vtk_assert(x)                                                                              \
  if (!(x))                                                                                        \
  {                                                                                                \
    cerr << "On line " << __LINE__ << " ERROR: Condition FAILED!! : " << #x << endl;               \
    return EXIT_FAILURE;                                                                           \
  }

int TestCGNSNoFlowSolutionPointers(int argc, char* argv[])
{
  char* fname =
    vtkTestUtilities::ExpandDataFileName(argc, argv, "Testing/Data/test_node_and_cell.cgns");
  vtkNew<vtkCGNSReader> reader;
  reader->SetFileName(fname);
  delete[] fname;

  reader->UpdateInformation();
  reader->EnableAllCellArrays();
  reader->EnableAllPointArrays();
  reader->Update();

  vtkMultiBlockDataSet* mb = reader->GetOutput();
  vtkDataSet* ds =
    vtkDataSet::SafeDownCast(vtkMultiBlockDataSet::SafeDownCast(mb->GetBlock(0))->GetBlock(0));
  vtk_assert(ds != nullptr);
  vtk_assert(ds->GetPointData()->GetArray("Pressure") != nullptr);
  vtk_assert(ds->GetCellData()->GetArray("Pressure") != nullptr);
  return EXIT_SUCCESS;
}
