/*
 *  kwelcome.cpp - part of the KDE Help Center
 *
 *  Copyright (C) 1998,99 Matthias Elter (me@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "kwelcome.h"
#include <klocale.h>

KWelcome::KWelcome()
{
  setCaption(i18n("Welcome to the K Desktop Environment!"));
  setMinimumSize(600, 420);
  setMaximumSize(680, 480);
  setGeometry(QApplication::desktop()->width()/2 - width()/2,
			  QApplication::desktop()->height()/2 - height()/2,
			  600, 420);
  
  view = new KWelcomeWidget(this);
  setFrameBorderWidth(2);
  setView(view, TRUE);
}
