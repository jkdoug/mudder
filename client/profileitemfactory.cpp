/*
  Mudder, a cross-platform text gaming client

  Copyright (C) 2014 Jason Douglas
  larkin.dischai@gmail.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/


#include "profileitemfactory.h"
#include "profileitem.h"
#include "accelerator.h"
#include "alias.h"
#include "event.h"
#include "group.h"
#include "timer.h"
#include "trigger.h"
#include "variable.h"
#include "editsetting.h"
#include "editvariable.h"

ProfileItem * ProfileItemFactory::create(const QString &name, Group *parent)
{
    if (name == "accelerator")
    {
        return new Accelerator(parent);
    }
    else if (name == "alias")
    {
        return new Alias(parent);
    }
    else if (name == "event")
    {
        return new Event(parent);
    }
    else if (name == "group")
    {
        return new Group(parent);
    }
    else if (name == "timer")
    {
        return new Timer(parent);
    }
    else if (name == "trigger")
    {
        return new Trigger(parent);
    }
    else if (name == "variable")
    {
        return new Variable(parent);
    }

    return 0;
}

EditSetting * ProfileItemFactory::editor(const QString &name)
{
    if (name == "variable")
    {
        return new EditVariable;
    }

    return 0;
}
