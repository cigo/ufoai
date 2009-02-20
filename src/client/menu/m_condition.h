/**
 * @file m_condition.h
 */

/*
Copyright (C) 1997-2008 UFO:AI Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef CLIENT_MENU_M_CONDITION_H
#define CLIENT_MENU_M_CONDITION_H

#include "../../common/common.h"

/* prototype */
struct cvar_s;

#define MAX_MENUCONDITIONS 512

/**
 * @brief conditions for V_SPECIAL_IF
 */
typedef enum menuIfCondition_s {
	IF_INVALID = -1,
	/** float compares */
	IF_EQ = 0, /**< == */
	IF_LE, /**< <= */
	IF_GE, /**< >= */
	IF_GT, /**< > */
	IF_LT, /**< < */
	IF_NE = 5, /**< != */
	IF_EXISTS, /**< only cvar given - check for existence */

	/** string compares */
	IF_STR_EQ,	/**< eq */
	IF_STR_NE,	/**< ne */

	IF_SIZE
} menuIfCondition_t;

/**
 * @sa menuIfCondition_t
 */
typedef struct menuDepends_s {
	const char *var;
	const char *value;
	struct cvar_s *cvar;
	int cond;
} menuDepends_t;

qboolean MN_CheckCondition(menuDepends_t *condition);
qboolean MN_InitCondition(menuDepends_t *condition, const char *token);
menuDepends_t *MN_AllocCondition(const char *description) __attribute__ ((warn_unused_result));

#endif
