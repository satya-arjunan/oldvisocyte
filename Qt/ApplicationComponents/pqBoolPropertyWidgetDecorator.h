/*=========================================================================

   Program: Visocyte
   Module:    $RCSfile$

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   Visocyte is a free software; you can redistribute it and/or modify it
   under the terms of the Visocyte license version 1.2.

   See License_v1.2.txt for the full Visocyte license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#ifndef pqBoolPropertyWidgetDecorator_h
#define pqBoolPropertyWidgetDecorator_h

#include "pqApplicationComponentsModule.h"
#include "pqPropertyWidgetDecorator.h"
#include "vtkWeakPointer.h"

/**
* pqBoolPropertyWidgetDecorator is a base class for enable/disable
* or show/hide widgets based on the status of another property not
* directly controlled by the widget.
*/
class PQAPPLICATIONCOMPONENTS_EXPORT pqBoolPropertyWidgetDecorator
  : public pqPropertyWidgetDecorator
{
  Q_OBJECT
  typedef pqPropertyWidgetDecorator Superclass;

public:
  pqBoolPropertyWidgetDecorator(vtkPVXMLElement* config, pqPropertyWidget* parent);
  ~pqBoolPropertyWidgetDecorator() override;

  bool isBoolProperty() const { return this->BoolProperty; }

signals:
  void boolPropertyChanged();

protected:
  vtkWeakPointer<vtkSMProperty> Property;
  QString Function;
  int Index;
  unsigned long ObserverId;
  bool BoolProperty;

private:
  Q_DISABLE_COPY(pqBoolPropertyWidgetDecorator)

  /**
  * updates the enabled state.
  */
  void updateBoolPropertyState();
  /**
  * update this->BoolProperty and fires boolPropertyChanged
  */
  void setBoolProperty(bool val);
};

#endif
