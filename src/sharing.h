/* This file is part of Conboy.
 *
 * Copyright (C) 2010 Cornelius Hald
 *
 * Conboy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Conboy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Conboy. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SHARING_H__
#define __SHARING_H__

#include "config.h"
#include "conboy_note.h"

#ifdef WITH_SHARING
void conboy_share_note_via_sharing (ConboyNote *note);
#endif

#ifdef WITH_BT
void conboy_share_note_via_bluetooth (ConboyNote *note);
#endif

#ifdef WITH_MODEST
void conboy_share_note_via_email (ConboyNote *note);
#endif

#endif /* __SHARING_H__ */
