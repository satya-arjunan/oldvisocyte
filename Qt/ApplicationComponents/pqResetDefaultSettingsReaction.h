/*=========================================================================

   Program: Visocyte
   Module:  pqResetDefaultSettingsReaction.h

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
#ifndef pqResetDefaultSettingsReaction_h
#define pqResetDefaultSettingsReaction_h

#include "pqReaction.h"

#include <QStringList>

/**
 * @class pqResetDefaultSettingsReaction
 * @brief reaction to restore user settings to default.
 * @ingroup Reactions
 *
 * pqResetDefaultSettingsReaction can restore user settings to default. It
 * pops up a prompt indicating whether the user wants to generate backups for
 * settings being restored. If so, backups are generated.
 */
class PQAPPLICATIONCOMPONENTS_EXPORT pqResetDefaultSettingsReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;

public:
  pqResetDefaultSettingsReaction(QAction* parent);
  ~pqResetDefaultSettingsReaction() override;

  /**
   * Reset to default settings. Application must be restarted for the changes to
   * take effect.
   */
  virtual void resetSettingsToDefault();

protected:
  /**
  * Called when the action is triggered.
  */
  void onTriggered() override { this->resetSettingsToDefault(); }

  virtual void clearSettings();
  virtual QStringList backupSettings();

private:
  Q_DISABLE_COPY(pqResetDefaultSettingsReaction)
};

#endif
