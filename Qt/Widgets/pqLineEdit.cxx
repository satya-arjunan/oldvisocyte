/*=========================================================================

   Program: Visocyte
   Module:    pqLineEdit.cxx

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
#include "pqLineEdit.h"

// Server Manager Includes.

// Qt Includes.

// Visocyte Includes.

//-----------------------------------------------------------------------------
pqLineEdit::pqLineEdit(QWidget* _parent)
  : Superclass(_parent)
  , EditingFinishedPending(false)
  , ResetCursorPositionOnEditingFinished(true)
{
  this->connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
  this->connect(this, SIGNAL(textEdited(const QString&)), this, SLOT(onTextEdited()));
}

//-----------------------------------------------------------------------------
pqLineEdit::pqLineEdit(const QString& _contents, QWidget* _parent)
  : Superclass(_contents, _parent)
  , EditingFinishedPending(false)
  , ResetCursorPositionOnEditingFinished(true)
{
  this->connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
  this->connect(this, SIGNAL(textEdited(const QString&)), this, SLOT(onTextEdited()));
}

//-----------------------------------------------------------------------------
pqLineEdit::~pqLineEdit()
{
}

//-----------------------------------------------------------------------------
void pqLineEdit::onTextEdited()
{
  this->EditingFinishedPending = true;
}

//-----------------------------------------------------------------------------
void pqLineEdit::onEditingFinished()
{
  if (this->EditingFinishedPending)
  {
    emit this->textChangedAndEditingFinished();
    this->EditingFinishedPending = false;
  }
  if (this->ResetCursorPositionOnEditingFinished)
  {
    this->setCursorPosition(0);
  }
}

//-----------------------------------------------------------------------------
void pqLineEdit::setTextAndResetCursor(const QString& val)
{
  if (this->text() != val)
  {
    this->Superclass::setText(val);
    this->setCursorPosition(0);
  }
}

//-----------------------------------------------------------------------------
void pqLineEdit::triggerTextChangedAndEditingFinished()
{
  // Since we do not update this->EditingFinishedPending when the text is
  // changed programmatically, textChangedAndEditingFinished() wasn't getting
  // fired after setText() was called during test playback. To overcome that
  // issue, the playback manually calls this method.
  this->onTextEdited();
  this->onEditingFinished();
}
