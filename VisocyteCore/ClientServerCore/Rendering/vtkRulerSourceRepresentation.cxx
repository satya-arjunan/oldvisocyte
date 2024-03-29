/*=========================================================================

  Program:   Visocyte
  Module:    vtkRulerSourceRepresentation.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkRulerSourceRepresentation.h"

#include "vtk3DWidgetRepresentation.h"
#include "vtkAbstractArray.h"
#include "vtkAbstractWidget.h"
#include "vtkAlgorithmOutput.h"
#include "vtkAxisActor2D.h"
#include "vtkDataSetAttributes.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPVCacheKeeper.h"
#include "vtkPVRenderView.h"
#include "vtkPointHandleRepresentation2D.h"
#include "vtkPointSource.h"
#include "vtkPolyData.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkTextProperty.h"
#include "vtkVariant.h"

vtkStandardNewMacro(vtkRulerSourceRepresentation);
vtkCxxSetObjectMacro(
  vtkRulerSourceRepresentation, DistanceRepresentation, vtkDistanceRepresentation2D);
//----------------------------------------------------------------------------
vtkRulerSourceRepresentation::vtkRulerSourceRepresentation()
{
  vtkSmartPointer<vtkPointHandleRepresentation2D> handle =
    vtkSmartPointer<vtkPointHandleRepresentation2D>::New();

  this->DistanceRepresentation = vtkDistanceRepresentation2D::New();
  this->DistanceRepresentation->SetHandleRepresentation(handle);
  this->DistanceRepresentation->InstantiateHandleRepresentation();
  this->DistanceRepresentation->GetAxis()->UseFontSizeFromPropertyOn();

  this->CacheKeeper->SetInputData(this->Clone.Get());
}

//----------------------------------------------------------------------------
vtkRulerSourceRepresentation::~vtkRulerSourceRepresentation()
{
  this->SetDistanceRepresentation(0);
}

//----------------------------------------------------------------------------
void vtkRulerSourceRepresentation::SetVisibility(bool val)
{
  this->Superclass::SetVisibility(val);
  if (this->DistanceRepresentation)
  {
    this->DistanceRepresentation->SetVisibility(val);
  }
}

//----------------------------------------------------------------------------
int vtkRulerSourceRepresentation::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//----------------------------------------------------------------------------
bool vtkRulerSourceRepresentation::AddToView(vtkView* view)
{
  vtkPVRenderView* rView = vtkPVRenderView::SafeDownCast(view);
  if (this->DistanceRepresentation && rView)
  {
    rView->GetRenderer(vtkPVRenderView::NON_COMPOSITED_RENDERER)
      ->AddActor(this->DistanceRepresentation);
  }
  return this->Superclass::AddToView(view);
}

//----------------------------------------------------------------------------
bool vtkRulerSourceRepresentation::RemoveFromView(vtkView* view)
{
  vtkPVRenderView* rView = vtkPVRenderView::SafeDownCast(view);
  if (this->DistanceRepresentation && rView)
  {
    rView->GetRenderer(vtkPVRenderView::NON_COMPOSITED_RENDERER)
      ->RemoveActor(this->DistanceRepresentation);
  }
  return this->Superclass::RemoveFromView(view);
}

//----------------------------------------------------------------------------
void vtkRulerSourceRepresentation::MarkModified()
{
  if (!this->GetUseCache())
  {
    // Cleanup caches when not using cache.
    this->CacheKeeper->RemoveAllCaches();
  }
  this->Superclass::MarkModified();
}

//----------------------------------------------------------------------------
bool vtkRulerSourceRepresentation::IsCached(double cache_key)
{
  return this->CacheKeeper->IsCached(cache_key);
}

//----------------------------------------------------------------------------
void vtkRulerSourceRepresentation::SetTextProperty(vtkTextProperty* prop)
{
  this->DistanceRepresentation->GetAxis()->SetTitleTextProperty(prop);
}

//----------------------------------------------------------------------------
void vtkRulerSourceRepresentation::SetAxisLineWidth(float width)
{
  this->DistanceRepresentation->GetAxisProperty()->SetLineWidth(width);
}

//----------------------------------------------------------------------------
void vtkRulerSourceRepresentation::SetAxisColor(double red, double green, double blue)
{
  this->DistanceRepresentation->GetAxisProperty()->SetColor(red, green, blue);
}

//----------------------------------------------------------------------------
void vtkRulerSourceRepresentation::SetLabelFormat(char* labelFormat)
{
  // Checking if the LabelFormat has changed is a bit cumbersome,
  // so we instead let the DistanceRepresentation handle it and
  // only call Modified() if the DistanceRepresentation says it has
  // changed.
  vtkMTimeType previousMTime = this->DistanceRepresentation->GetMTime();
  this->DistanceRepresentation->SetLabelFormat(labelFormat);
  if (this->DistanceRepresentation->GetMTime() != previousMTime)
  {
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkRulerSourceRepresentation::SetRulerMode(int choice)
{
  if (choice != this->GetRulerMode())
  {
    this->DistanceRepresentation->SetRulerMode(choice);
    this->Modified();
  }
}

//----------------------------------------------------------------------------
int vtkRulerSourceRepresentation::GetRulerMode()
{
  return this->DistanceRepresentation->GetRulerMode();
}

//----------------------------------------------------------------------------
void vtkRulerSourceRepresentation::SetRulerDistance(double distance)
{
  if (distance != this->GetRulerDistance())
  {
    this->DistanceRepresentation->SetRulerDistance(distance);
    this->Modified();
  }
}

//----------------------------------------------------------------------------
double vtkRulerSourceRepresentation::GetRulerDistance()
{
  return this->DistanceRepresentation->GetRulerDistance();
}

//----------------------------------------------------------------------------
void vtkRulerSourceRepresentation::SetScale(double scale)
{
  if (scale != this->GetScale())
  {
    this->DistanceRepresentation->SetScale(scale);
    this->Modified();
  }
}

//----------------------------------------------------------------------------
double vtkRulerSourceRepresentation::GetScale()
{
  return this->DistanceRepresentation->GetScale();
}

//----------------------------------------------------------------------------
void vtkRulerSourceRepresentation::SetNumberOfRulerTicks(int n)
{
  if (n != this->DistanceRepresentation->GetNumberOfRulerTicks())
  {
    this->DistanceRepresentation->SetNumberOfRulerTicks(n);
    this->Modified();
  }
}

//----------------------------------------------------------------------------
int vtkRulerSourceRepresentation::RequestData(
  vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // Pass caching information to the cache keeper.
  this->CacheKeeper->SetCachingEnabled(this->GetUseCache());
  this->CacheKeeper->SetCacheTime(this->GetCacheKey());

  if (inputVector[0]->GetNumberOfInformationObjects() == 1)
  {
    this->Clone->ShallowCopy(vtkPolyData::GetData(inputVector[0], 0));
  }
  this->Clone->Modified();
  this->CacheKeeper->Update();

  return this->Superclass::RequestData(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkRulerSourceRepresentation::ProcessViewRequest(
  vtkInformationRequestKey* request_type, vtkInformation* inInfo, vtkInformation* outInfo)
{
  if (!this->Superclass::ProcessViewRequest(request_type, inInfo, outInfo))
  {
    // i.e. this->GetVisibility() == false, hence nothing to do.
    return 0;
  }

  if (request_type == vtkPVView::REQUEST_UPDATE())
  {
    vtkPVRenderView::SetPiece(inInfo, this, this->CacheKeeper->GetOutputDataObject(0));
    // `gather_before_delivery` is true, since vtkLineSource (which is the
    // source for the ruler) doesn't produce any data on ranks except the root.
    vtkPVRenderView::SetDeliverToAllProcesses(inInfo, this, true);
  }
  else if (request_type == vtkPVView::REQUEST_RENDER())
  {
    vtkAlgorithmOutput* producerPort = vtkPVRenderView::GetPieceProducer(inInfo, this);

    // since there's no direct connection between the mapper and the collector,
    // we don't put an update-suppressor in the pipeline.

    vtkPolyData* line = vtkPolyData::SafeDownCast(
      producerPort->GetProducer()->GetOutputDataObject(producerPort->GetIndex()));
    if (line && line->GetNumberOfPoints() == 2)
    {
      this->DistanceRepresentation->SetPoint1WorldPosition(line->GetPoints()->GetPoint(0));
      this->DistanceRepresentation->SetPoint2WorldPosition(line->GetPoints()->GetPoint(1));
    }
    else
    {
      vtkWarningMacro(<< "Expected line to have 2 points, but it had " << line->GetNumberOfPoints()
                      << " points.");
    }
  }

  return 1;
}

//----------------------------------------------------------------------------
void vtkRulerSourceRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
