/*=========================================================================

   Program: Visocyte
   Module:    pqSelectionInputWidget.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
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

=========================================================================*/
#include "pqSelectionInputWidget.h"

#include "vtkEventQtSlotConnect.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyIterator.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"

#include <QTextStream>
#include <QtDebug>

#include "pqApplicationCore.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqProxy.h"
#include "pqSMAdaptor.h"
#include "pqSelectionManager.h"

#include <string>

#include "ui_pqSelectionInputWidget.h"
class pqSelectionInputWidget::UI : public Ui::pqSelectionInputWidget
{
};

namespace
{
/// given vtkSelectionNode::SelectionField, returns a string for it.
const char* getSelectionFieldTypeAsString(int fieldType)
{
  switch (fieldType)
  {
    case vtkSelectionNode::CELL:
      return "Cells";
    case vtkSelectionNode::POINT:
      return "Points";
    case vtkSelectionNode::FIELD:
      return "Fields (not supported)";
    case vtkSelectionNode::VERTEX:
      return "Vertices";
    case vtkSelectionNode::EDGE:
      return "Edges";
    case vtkSelectionNode::ROW:
      return "Rows";
    default:
      return "Unknown (not supported)";
  }
}
}
//-----------------------------------------------------------------------------
pqSelectionInputWidget::pqSelectionInputWidget(QWidget* _parent)
  : QWidget(_parent)
{
  this->ui = new pqSelectionInputWidget::UI();
  this->ui->setupUi(this);

  QObject::connect(
    this->ui->pushButtonCopySelection, SIGNAL(clicked()), this, SLOT(copyActiveSelection()));

  pqSelectionManager* selMan =
    qobject_cast<pqSelectionManager*>(pqApplicationCore::instance()->manager("SelectionManager"));

  if (selMan)
  {
    QObject::connect(
      selMan, SIGNAL(selectionChanged(pqOutputPort*)), this, SLOT(onActiveSelectionChanged()));
  }
}

//-----------------------------------------------------------------------------
pqSelectionInputWidget::~pqSelectionInputWidget()
{
  delete this->ui;
}

//-----------------------------------------------------------------------------
void pqSelectionInputWidget::updateLabels()
{
  if (!this->SelectionSource)
  {
    this->ui->label->setText("No selection");
    this->ui->textBrowser->setText("");
    return;
  }

  this->ui->label->setText("Copied Selection");

  int fieldType =
    pqSMAdaptor::getElementProperty(this->SelectionSource->GetProperty("FieldType")).toInt();

  const char* fieldTypeAsString = getSelectionFieldTypeAsString(fieldType);

  const char* xmlname = this->SelectionSource->GetXMLName();
  QString text = QString("Type: ");
  QTextStream columnValues(&text, QIODevice::ReadWrite);
  if (strcmp(xmlname, "FrustumSelectionSource") == 0)
  {
    columnValues << "Frustum Selection" << endl
                 << "Elements: " << fieldTypeAsString << endl
                 << endl
                 << "Values:" << endl;
    vtkSMProperty* dvp = this->SelectionSource->GetProperty("Frustum");
    QList<QVariant> value = pqSMAdaptor::getMultipleElementProperty(dvp);
    for (int cc = 0; cc < value.size(); cc++)
    {
      if ((cc % 4) == 0)
      {
        columnValues << endl;
      }
      columnValues << value[cc].toDouble() << "\t";
    }
  }
  else if (strcmp(xmlname, "GlobalIDSelectionSource") == 0)
  {
    columnValues << "Global ID Selection" << endl
                 << "Elements: " << fieldTypeAsString << endl
                 << endl
                 << endl;
    QList<QVariant> value =
      pqSMAdaptor::getMultipleElementProperty(this->SelectionSource->GetProperty("IDs"));
    columnValues << "Global ID" << endl;
    foreach (QVariant val, value)
    {
      columnValues << val.toString() << endl;
    }
  }
  else if (strcmp(xmlname, "IDSelectionSource") == 0)
  {
    columnValues << QString("ID Selection") << endl
                 << "Elements: " << fieldTypeAsString << endl
                 << endl
                 << endl;
    columnValues << "Process ID"
                 << "\t\t"
                 << "Index" << endl;
    vtkSMProperty* idvp = this->SelectionSource->GetProperty("IDs");
    QList<QVariant> value = pqSMAdaptor::getMultipleElementProperty(idvp);
    for (int cc = 0; cc < value.size(); cc++)
    {
      if ((cc % 2) == 0)
      {
        columnValues << endl;
      }
      columnValues << value[cc].toString() << "\t\t";
    }
  }
  else if (strcmp(xmlname, "CompositeDataIDSelectionSource") == 0)
  {
    columnValues << QString("ID Selection") << endl
                 << "Elements: " << fieldTypeAsString << endl
                 << endl
                 << endl;
    columnValues << "Composite ID"
                 << "\t"
                 << "Process ID"
                 << "\t\t"
                 << "Index" << endl;
    vtkSMProperty* idvp = this->SelectionSource->GetProperty("IDs");
    QList<QVariant> value = pqSMAdaptor::getMultipleElementProperty(idvp);
    for (int cc = 0; cc < value.size(); cc++)
    {
      if ((cc % 3) == 0)
      {
        columnValues << endl;
      }
      columnValues << value[cc].toString() << "\t\t";
    }
  }
  else if (strcmp(xmlname, "HierarchicalDataIDSelectionSource") == 0)
  {
    columnValues << QString("ID Selection") << endl
                 << "Elements: " << fieldTypeAsString << endl
                 << endl
                 << endl;
    columnValues << "Level"
                 << "\t\t"
                 << "Dataset"
                 << "\t\t"
                 << "Index" << endl;
    vtkSMProperty* idvp = this->SelectionSource->GetProperty("IDs");
    QList<QVariant> value = pqSMAdaptor::getMultipleElementProperty(idvp);
    for (int cc = 0; cc < value.size(); cc++)
    {
      if ((cc % 3) == 0)
      {
        columnValues << endl;
      }
      columnValues << value[cc].toString() << "\t\t";
    }
  }
  else if (strcmp(xmlname, "LocationSelectionSource") == 0)
  {
    columnValues << "Location-based Selection" << endl << endl << endl;
    columnValues << "Probe Locations" << endl;
    vtkSMProperty* idvp = this->SelectionSource->GetProperty("Locations");
    QList<QVariant> value = pqSMAdaptor::getMultipleElementProperty(idvp);
    for (int cc = 0; cc < value.size(); cc++)
    {
      if ((cc % 3) == 0)
      {
        columnValues << endl;
      }
      columnValues << value[cc].toString() << "\t\t";
    }
  }
  else if (strcmp(xmlname, "BlockSelectionSource") == 0)
  {
    columnValues << "Block Selection" << endl << endl << endl;
    columnValues << "Blocks" << endl;
    vtkSMProperty* prop = this->SelectionSource->GetProperty("Blocks");
    QList<QVariant> values = pqSMAdaptor::getMultipleElementProperty(prop);
    foreach (const QVariant& value, values)
    {
      columnValues << value.toString() << endl;
    }
  }
  else if (strcmp(xmlname, "ThresholdSelectionSource") == 0)
  {
    columnValues << "Threshold Selection" << endl;
    columnValues << "Elements: " << fieldTypeAsString << endl;
    columnValues
      << "Array: "
      << pqSMAdaptor::getElementProperty(this->SelectionSource->GetProperty("ArrayName")).toString()
      << endl
      << endl;
    QList<QVariant> values =
      pqSMAdaptor::getMultipleElementProperty(this->SelectionSource->GetProperty("Thresholds"));
    for (int cc = 0; cc < values.size(); cc++)
    {
      const QVariant& value = values[cc];
      columnValues << value.toString();
      if (cc % 2)
      {
        columnValues << endl;
      }
      else
      {
        columnValues << "\t\t";
      }
    }
  }
  else if (strcmp(xmlname, "SelectionQuerySource") == 0)
  {
    columnValues << "Query" << endl;
    this->SelectionSource->UpdatePropertyInformation();
    columnValues << vtkSMPropertyHelper(this->SelectionSource, "UserFriendlyText").GetAsString()
                 << endl
                 << "Elements: " << fieldTypeAsString;
  }
  else
  {
    columnValues << "None" << endl;
  }

  this->ui->textBrowser->setText(text);
  columnValues.flush();
}

//-----------------------------------------------------------------------------
void pqSelectionInputWidget::initializeDefaultValueIfNeeded()
{
  if (!this->SelectionSource)
  {
    this->copyActiveSelection();
  }
}

//-----------------------------------------------------------------------------
// This will update the UnacceptedSelectionSource proxy with a clone of the
// active selection proxy. The filter proxy is not modified yet.
void pqSelectionInputWidget::copyActiveSelection()
{
  pqSelectionManager* selMan =
    (pqSelectionManager*)(pqApplicationCore::instance()->manager("SelectionManager"));

  if (!selMan)
  {
    qDebug() << "No selection manager was detected. "
                "Don't know where to get the active selection from.";
    return;
  }

  pqOutputPort* port = selMan->getSelectedPort();
  if (!port)
  {
    this->setSelection(nullptr);
    return;
  }

  vtkSMProxy* activeSelection = port->getSelectionInput();
  if (!activeSelection)
  {
    this->setSelection(nullptr);
    return;
  }

  vtkSMSessionProxyManager* pxm = activeSelection->GetSessionProxyManager();
  vtkSMProxy* newSource =
    pxm->NewProxy(activeSelection->GetXMLGroup(), activeSelection->GetXMLName());
  newSource->Copy(activeSelection);
  newSource->UpdateVTKObjects();
  this->setSelection(newSource);
  newSource->Delete();
}

//-----------------------------------------------------------------------------
void pqSelectionInputWidget::setSelection(pqSMProxy newSelection)
{
  if (this->SelectionSource == newSelection)
    return;

  this->SelectionSource = newSelection;

  this->updateLabels();
  emit this->selectionChanged(this->SelectionSource);
}

//-----------------------------------------------------------------------------
void pqSelectionInputWidget::onActiveSelectionChanged()
{
  // The selection has changed, either a new selection was created
  // or an old one cleared.
  this->ui->label->setText("Copied Selection (Active Selection Changed)");
}

//-----------------------------------------------------------------------------
void pqSelectionInputWidget::postAccept()
{
  // Select proper ProxyManager
  vtkSMProxy* sel_source = this->selection();
  vtkSMSessionProxyManager* pxm = sel_source ? sel_source->GetSessionProxyManager() : NULL;

  if (!sel_source)
  {
    return;
  }

  // Unregister any de-referenced proxy sources.
  vtkSMProxyIterator* iter = vtkSMProxyIterator::New();
  iter->SetSession(sel_source->GetSession());
  iter->SetModeToOneGroup();
  for (iter->Begin("selection_sources"); !iter->IsAtEnd();)
  {
    vtkSMProxy* proxy = iter->GetProxy();
    if (proxy->GetNumberOfConsumers() == 0)
    {
      std::string key = iter->GetKey();
      iter->Next();
      pxm->UnRegisterProxy("selection_sources", key.c_str(), proxy);
    }
    else
    {
      iter->Next();
    }
  }
  iter->Delete();
}

//-----------------------------------------------------------------------------
void pqSelectionInputWidget::preAccept()
{
  // Select proper ProxyManager
  vtkSMProxy* sel_source = this->selection();
  vtkSMSessionProxyManager* pxm = sel_source ? sel_source->GetSessionProxyManager() : NULL;

  // Register the new selection proxy.
  if (sel_source && pxm)
  {
    if (!pxm->GetProxyName("selection_sources", sel_source))
    {
      std::string key = std::string("selection_source.") + sel_source->GetGlobalIDAsString();
      pxm->RegisterProxy("selection_sources", key.c_str(), sel_source);
    }
  }
}
