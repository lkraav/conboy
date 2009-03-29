/*******************************************************************************
 * Copyright (c) 2007-2008 INdT.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.

 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 ============================================================================
 Name        : localisation.h
 Author      : Cornelius Hald
 Version     : 0.1
 Description : Header with localization facilities
 ============================================================================
 */
#ifndef LOCALISATION_H
#define LOCALISATION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif                          /* HAVE_CONFIG_H */

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) gettext(String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#define locale_init() setlocale(LC_ALL, "");\
    bindtextdomain(GETTEXT_PACKAGE, localedir);\
    textdomain(GETTEXT_PACKAGE);
#else                           /* NLS is disabled */
#define locale_init() 
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain)
#define bind_textdomain_codeset(Domain,Codeset) (Codeset)
#endif                          /* ENABLE_NLS */

#endif /* LOCALISATION_H */
#ifndef LOCALISATION_H
#define LOCALISATION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif                          /* HAVE_CONFIG_H */

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) gettext(String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#define locale_init() setlocale(LC_ALL, "");\
    bindtextdomain(GETTEXT_PACKAGE, localedir);\
    textdomain(GETTEXT_PACKAGE);
#else                           /* NLS is disabled */
#define locale_init() 
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain)
#define bind_textdomain_codeset(Domain,Codeset) (Codeset)
#endif                          /* ENABLE_NLS */

#endif /* LOCALISATION_H */

