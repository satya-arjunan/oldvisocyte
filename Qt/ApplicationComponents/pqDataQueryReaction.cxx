/*=========================================================================

   Program: Visocyte
   Module:    pqDataQueryReaction.cxx

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
#include "pqDataQueryReaction.h"

#include "pqCoreUtilities.h"
#include "pqFindDataDialog.h"

#include <QMessageBox>

static QPointer<pqFindDataDialog> pqFindDataSingleton;

//-----------------------------------------------------------------------------
pqDataQueryReaction::pqDataQueryReaction(QAction* parentObject)
  : Superclass(parentObject)
{
}

//-----------------------------------------------------------------------------
pqDataQueryReaction::~pqDataQueryReaction()
{
}

//-----------------------------------------------------------------------------
void pqDataQueryReaction::showQueryDialog()
{
#if VTK_MODULE_ENABLE_Visocyte_pqPython
  if (pqFindDataSingleton.isNull())
  {
    pqFindDataDialog* dialog = new pqFindDataDialog(pqCoreUtilities::mainWidget());
    pqFindDataSingleton = dialog;
  }

  pqFindDataSingleton->show();
  pqFindDataSingleton->raise();
#else
  QMessageBox::warning(0, "Selection Not Supported",
    "Error: Find Data requires that Visocyte be built with Python enabled. "
    "To enable Python set the CMake flag 'VISOCYTE_ENABLE_PYTHON' to True "
    "and ensure that the 'Visocyte::pqPython'module is available.");
#endif
}
