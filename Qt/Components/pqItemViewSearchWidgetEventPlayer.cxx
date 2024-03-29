/*=========================================================================

   Program: Visocyte
   Module:  pqItemViewSearchWidgetEventPlayer.cxx

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
#include "pqItemViewSearchWidgetEventPlayer.h"
#include "pqItemViewSearchWidget.h"

#include <QAbstractItemView>
#include <QPointer>

//----------------------------------------------------------------------------
pqItemViewSearchWidgetEventPlayer::pqItemViewSearchWidgetEventPlayer(QObject* p)
  : Superclass(p)
{
}

//----------------------------------------------------------------------------
pqItemViewSearchWidgetEventPlayer::~pqItemViewSearchWidgetEventPlayer()
{
}

//----------------------------------------------------------------------------
bool pqItemViewSearchWidgetEventPlayer::playEvent(
  QObject* w, const QString& command, const QString& arguments, bool& error)
{
  Q_UNUSED(error);
  if (command == pqItemViewSearchWidgetEventPlayer::EVENT_NAME())
  {
    if (arguments == "ctrlF")
    {
      QPointer<QAbstractItemView> focusItemView = qobject_cast<QAbstractItemView*>(w);
      if (!focusItemView)
      {
        return false;
      }
      QPointer<pqItemViewSearchWidget> searchWidget = new pqItemViewSearchWidget(focusItemView);
      searchWidget->setAttribute(Qt::WA_DeleteOnClose, true);
      searchWidget->showSearchWidget();
    }
    return true;
  }
  else
  {
    return false;
  }
}

//-----------------------------------------------------------------------------
const QString& pqItemViewSearchWidgetEventPlayer::EVENT_NAME()
{
  static const QString eventName("launchSearchWidget");
  return eventName;
}
