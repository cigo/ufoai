/**
 * @file m_node_container.c
 * @todo move container list code out
 * @todo improve the code genericity
 * @todo rename 'visibleRows' and 'EXTRADATA(node).scrollNum' into 'heightCache' or something like that
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

#include "../../client.h"
#include "../../renderer/r_draw.h"
#include "../../renderer/r_mesh.h"
#include "../../cl_actor.h"
#include "../../cl_team.h"
#include "../../cl_game.h"
#include "../m_main.h"
#include "../m_parse.h"
#include "../m_font.h"
#include "../m_dragndrop.h"
#include "../m_tooltip.h"
#include "../m_nodes.h"
#include "../m_input.h"
#include "m_node_model.h"
#include "m_node_container.h"
#include "m_node_abstractnode.h"

inventory_t *menuInventory = NULL;

#define EXTRADATA(node) (node->u.container)

/** self cache for drag item
 * @note we can use a global var because we only can have 1 source node at a time
 */
static int dragInfoFromX = -1;
static int dragInfoFromY = -1;

/** self cache for the preview and dropped item
 * @note we can use a global var because we only can have 1 target node at a time
 */
static int dragInfoToX = -1;
static int dragInfoToY = -1;

/** The current invList pointer (only used for ignoring the dragged item
 * for finding free space right now)
 */
static const invList_t *dragInfoIC;

/**
 * @brief Set a filter
 */
void MN_ContainerNodeSetFilter (menuNode_t* node, int num)
{
	if (EXTRADATA(node).filterEquipType != num) {
		EXTRADATA(node).filterEquipType = num;
		EXTRADATA(node).scrollCur = 0;
		EXTRADATA(node).scrollNum = 0;
		EXTRADATA(node).scrollTotalNum = -1;	/**< invalidate the value */
	}
}

/**
 * @brief Update display of scroll buttons.
 * @note The cvars "mn_cont_scroll_prev_hover" and "mn_cont_scroll_next_hover" are
 * set by the "in" and "out" functions of the scroll buttons.
 */
static void MN_ContainerNodeUpdateScroll (menuNode_t* node)
{
	if (node->u.container.onViewChange) {
		MN_ExecuteEventActions(node, node->u.container.onViewChange);
	}
}

#if 0	/* maybe not need */

/**
 * @brief Scrolls one item forward in a scrollable container.
 * @sa Console command "scrollcont_next"
 * @todo Add check so we do only increase if there are still hidden items after the last displayed one.
 */
static void MN_ScrollContainerNext_f (void)
{
	menuNode_t* node = MN_GetNodeFromCurrentMenu("equip");

	/* Can be called from everywhere. */
	if (!node || !menuInventory)
		return;

	/* Check if the end of the currently visible items still is not the last of the displayable items. */
	if (EXTRADATA(node).scrollCur < EXTRADATA(node).scrollTotalNum
	 && EXTRADATA(node).scrollCur + EXTRADATA(node).scrollNum < EXTRADATA(node).scrollTotalNum) {
		EXTRADATA(node).scrollCur++;
		Com_DPrintf(DEBUG_CLIENT, "MN_ScrollContainerNext_f: Increased current scroll index: %i (num: %i, total: %i).\n",
			EXTRADATA(node).scrollCur,
			EXTRADATA(node).scrollNum,
			EXTRADATA(node).scrollTotalNum);
	} else
		Com_DPrintf(DEBUG_CLIENT, "MN_ScrollContainerNext_f: Current scroll index already %i (num: %i, total: %i)).\n",
			EXTRADATA(node).scrollCur,
			EXTRADATA(node).scrollNum,
			EXTRADATA(node).scrollTotalNum);

	/* Update display of scroll buttons. */
	MN_ContainerNodeUpdateScroll(node);
}

/**
 * @brief Scrolls one item backwards in a scrollable container.
 * @sa Console command "scrollcont_prev"
 */
static void MN_ScrollContainerPrev_f (void)
{
	menuNode_t* node = MN_GetNodeFromCurrentMenu("equip");

	/* Can be called from everywhere. */
	if (!node || !menuInventory)
		return;

	if (EXTRADATA(node).scrollCur > 0) {
		EXTRADATA(node).scrollCur--;
		Com_DPrintf(DEBUG_CLIENT, "MN_ScrollContainerNext_f: Decreased current scroll index: %i (num: %i, total: %i).\n",
			EXTRADATA(node).scrollCur,
			EXTRADATA(node).scrollNum,
			EXTRADATA(node).scrollTotalNum);
	} else
		Com_DPrintf(DEBUG_CLIENT, "MN_ScrollContainerNext_f: Current scroll index already %i (num: %i, total: %i).\n",
			EXTRADATA(node).scrollCur,
			EXTRADATA(node).scrollNum,
			EXTRADATA(node).scrollTotalNum);

	/* Update display of scroll buttons. */
	MN_ContainerNodeUpdateScroll(node);
}

/**
 * @brief Scrolls items in a scrollable container.
 */
static void MN_ScrollContainerScroll_f (void)
{
	int offset;

	if (Cmd_Argc() < 2) {
		Com_Printf("Usage: %s <+/-offset>\n", Cmd_Argv(0));
		return;
	}

	/* Can be called from everywhere. */
	if (!menuInventory)
		return;

	offset = atoi(Cmd_Argv(1));
	while (offset > 0) {
		MN_ScrollContainerNext_f();
		offset--;
	}
	while (offset < 0) {
		MN_ScrollContainerPrev_f();
		offset++;
	}
}

#endif

static inline qboolean MN_IsScrollContainerNode (const menuNode_t* const node)
{
	return EXTRADATA(node).container && EXTRADATA(node).container->scroll;
}

/**
 * @brief Draws an item to the screen
 *
 * @param[in] org Node position on the screen (pixel). Single nodes: Use the center of the node.
 * @param[in] item The item to draw.
 * @param[in] x Position in container. Set this to -1 if it's drawn in a single container.
 * @param[in] y Position in container. Set this to -1 if it's drawn in a single container.
 * @param[in] scale
 * @param[in] color
 * @sa SCR_DrawCursor
 * Used to draw an item to the equipment containers. First look whether the objDef_t
 * includes an image - if there is none then draw the model
 */
void MN_DrawItem (menuNode_t *node, const vec3_t org, const item_t *item, int x, int y, const vec3_t scale, const vec4_t color)
{
	objDef_t *od = item->t;
	vec4_t col;
	vec3_t origin;

	assert(od);
	assert(org[2] > -1000 && org[2] < 1000); 	/*< prevent use of vec2_t for org */

	Vector4Copy(color, col);
	/* no ammo in this weapon - highlight this item */
	if (od->weapon && od->reload && !item->a) {
		col[1] *= 0.5;
		col[2] *= 0.5;
	}

	VectorCopy(org, origin);

	/* Calculate correct location of the image or the model (depends on rotation) */
	/** @todo Change the rotation of the image as well, right now only the location is changed.
	 * How is image-rotation handled right now? */
	if (x >= 0 || y >= 0) {
		/* Add offset of location in container. */
		origin[0] += x * C_UNIT;
		origin[1] += y * C_UNIT;

		/* Add offset for item-center (depends on rotation). */
		if (item->rotated) {
			origin[0] += od->sy * C_UNIT / 2.0;
			origin[1] += od->sx * C_UNIT / 2.0;
			/** @todo Image size calculation depends on handling of image-rotation.
			imgWidth = od->sy * C_UNIT;
			imgHeight = od->sx * C_UNIT;
			*/
		} else {
			origin[0] += od->sx * C_UNIT / 2.0;
			origin[1] += od->sy * C_UNIT / 2.0;
		}
	}

	/* don't handle the od->tech->image here - it's very ufopedia specific in most cases */
	if (od->image[0] != '\0') {
		const int imgWidth = od->sx * C_UNIT;
		const int imgHeight = od->sy * C_UNIT;

		/* Draw the image. */
		R_DrawNormPic(origin[0], origin[1], imgWidth, imgHeight, 0, 0, 0, 0, ALIGN_CC, qtrue, od->image);
	} else {
		menuModel_t *menuModel = NULL;
		const char *modelName = od->model;
		if (od->tech && od->tech->mdl) {
			menuModel = MN_GetMenuModel(od->tech->mdl);
			/* the model from the tech structure has higher priority, because the item model itself
			 * is mainly for the battlescape or the geoscape - only use that as a fallback */
			modelName = od->tech->mdl;
		}

		/* no model definition in the tech struct, not in the fallback object definition */
		if (modelName == NULL || modelName[0] == '\0') {
			Com_Printf("MN_DrawItem: No model given for item: '%s'\n", od->id);
			return;
		}

		if (menuModel && node) {
			MN_DrawModelNode(node, modelName);
		} else {
			modelInfo_t mi;
			vec3_t angles = {-10, 160, 70};
			vec3_t size = {scale[0], scale[1], scale[2]};

			if (item->rotated)
				angles[0] -= 90;

			memset(&mi, 0, sizeof(mi));
			mi.origin = origin;
			mi.angles = angles;
			mi.center = od->center;
			mi.scale = size;
			mi.color = col;
			mi.name = modelName;
			if (od->scale)
				VectorScale(size, od->scale, size);

			/* draw the model */
			R_DrawModelDirect(&mi, NULL, NULL);
		}
	}
}

/**
 * @brief Generate tooltip text for an item.
 * @param[in] item The item we want to generate the tooltip text for.
 * @param[in,out] tooltiptext Pointer to a string the information should be written into.
 * @param[in] string_maxlength Max. string size of tooltiptext.
 * @return Number of lines
 */
static void MN_GetItemTooltip (item_t item, char *tooltiptext, size_t string_maxlength)
{
	int i;
	objDef_t *weapon;

	assert(item.t);

	if (item.amount > 1)
		Com_sprintf(tooltiptext, string_maxlength, "%i x %s\n", item.amount, _(item.t->name));
	else
		Com_sprintf(tooltiptext, string_maxlength, "%s\n", _(item.t->name));

	/* Only display further info if item.t is researched */
	if (RS_IsResearched_ptr(item.t->tech)) {
		if (item.t->weapon) {
			/* Get info about used ammo (if there is any) */
			if (item.t == item.m) {
				/* Item has no ammo but might have shot-count */
				if (item.a) {
					Q_strcat(tooltiptext, va(_("Ammo: %i\n"), item.a), string_maxlength);
				}
			} else if (item.m) {
				/* Search for used ammo and display name + ammo count */
				Q_strcat(tooltiptext, va(_("%s loaded\n"), _(item.m->name)), string_maxlength);
				Q_strcat(tooltiptext, va(_("Ammo: %i\n"),  item.a), string_maxlength);
			}
		} else if (item.t->numWeapons) {
			/* Check if this is a non-weapon and non-ammo item */
			if (!(item.t->numWeapons == 1 && item.t->weapons[0] == item.t)) {
				/* If it's ammo get the weapon names it can be used in */
				Q_strcat(tooltiptext, _("Usable in:\n"), string_maxlength);
				for (i = 0; i < item.t->numWeapons; i++) {
					weapon = item.t->weapons[i];
					if (RS_IsResearched_ptr(weapon->tech)) {
						Q_strcat(tooltiptext, va("* %s\n", _(weapon->name)), string_maxlength);
					}
				}
			}
		}
	}
}

/**
 * @brief Draws the rectangle in a 'free' style on position posx/posy (pixel) in the size sizex/sizey (pixel)
 */
static void MN_DrawDisabled (const menuNode_t* node)
{
	const vec4_t color = { 0.3f, 0.3f, 0.3f, 0.7f };
	vec2_t nodepos;

	MN_GetNodeAbsPos(node, nodepos);
	R_DrawFill(nodepos[0], nodepos[1], node->size[0], node->size[1], ALIGN_UL, color);
}

/**
 * @brief Draws the rectangle in a 'free' style on position posx/posy (pixel) in the size sizex/sizey (pixel)
 */
static void MN_DrawFree (int container, const menuNode_t *node, int posx, int posy, int sizex, int sizey, qboolean showTUs)
{
	const vec4_t color = { 0.0f, 1.0f, 0.0f, 0.7f };
	invDef_t* inv = &csi.ids[container];
	vec2_t nodepos;

	MN_GetNodeAbsPos(node, nodepos);
	R_DrawFill(posx, posy, sizex, sizey, ALIGN_UL, color);

	/* if showTUs is true (only the first time in none single containers)
	 * and we are connected to a game */
	if (showTUs && cls.state == ca_active){
		R_FontDrawString("f_verysmall", 0, nodepos[0] + 3, nodepos[1] + 3,
			nodepos[0] + 3, nodepos[1] + 3, node->size[0] - 6, 0, 0,
			va(_("In: %i Out: %i"), inv->in, inv->out), 0, 0, NULL, qfalse, 0);
	}
}

/**
 * @brief Draws the free and usable inventory positions when dragging an item.
 * @note Only call this function in dragging mode
 */
static void MN_ContainerNodeDrawFreeSpace (menuNode_t *node, inventory_t *inv)
{
	const objDef_t *od = MN_DNDGetItem()->t;	/**< Get the 'type' of the dragged item. */
	vec2_t nodepos;

	/* Draw only in dragging-mode and not for the equip-floor */
	assert(MN_DNDIsDragging());
	assert(inv);

	MN_GetNodeAbsPos(node, nodepos);
	/* if single container (hands, extension, headgear) */
	if (EXTRADATA(node).container->single) {
		/* if container is free or the dragged-item is in it */
		if (MN_DNDIsSourceNode(node) || Com_CheckToInventory(inv, od, EXTRADATA(node).container, 0, 0, dragInfoIC))
			MN_DrawFree(EXTRADATA(node).container->id, node, nodepos[0], nodepos[1], node->size[0], node->size[1], qtrue);
	} else {
		/* The shape of the free positions. */
		uint32_t free[SHAPE_BIG_MAX_HEIGHT];
		qboolean showTUs = qtrue;
		int x, y;

		memset(free, 0, sizeof(free));

		for (y = 0; y < SHAPE_BIG_MAX_HEIGHT; y++) {
			for (x = 0; x < SHAPE_BIG_MAX_WIDTH; x++) {
				/* Check if the current position is usable (topleft of the item). */

				/* Add '1's to each position the item is 'blocking'. */
				const int checkedTo = Com_CheckToInventory(inv, od, EXTRADATA(node).container, x, y, dragInfoIC);
				if (checkedTo & INV_FITS)				/* Item can be placed normally. */
					Com_MergeShapes(free, (uint32_t)od->shape, x, y);
				if (checkedTo & INV_FITS_ONLY_ROTATED)	/* Item can be placed rotated. */
					Com_MergeShapes(free, Com_ShapeRotate((uint32_t)od->shape), x, y);

				/* Only draw on existing positions. */
				if (Com_CheckShape(EXTRADATA(node).container->shape, x, y)) {
					if (Com_CheckShape(free, x, y)) {
						MN_DrawFree(EXTRADATA(node).container->id, node, nodepos[0] + x * C_UNIT, nodepos[1] + y * C_UNIT, C_UNIT, C_UNIT, showTUs);
						showTUs = qfalse;
					}
				}
			}	/* for x */
		}	/* for y */
	}
}

/**
 * @brief Calculates the size of a container node and links the container
 * into the node (uses the @c invDef_t shape bitmask to determine the size)
 * @param[in,out] node The node to get the size for
 */
static void MN_ContainerNodeLoaded (menuNode_t* const node)
{
	const char *name;
	invDef_t *container;
	int i, j;

	/** @todo find a better way to add more equip node, without this hack */
	name = node->name;
	if (!Q_strncmp(node->name, "equip_", 6)) {
		name = "equip";
	}

	for (i = 0, container = csi.ids; i < csi.numIDs; container++, i++)
		if (!Q_strncmp(name, container->name, sizeof(node->name)))
			break;

	/* not find */
	if (i == csi.numIDs)
		return;

	EXTRADATA(node).container = container;

	if (MN_IsScrollContainerNode(node)) {
		/* No need to calculate the size, this is done in the menu script */
	} else {
		/* Start on the last bit of the shape mask. */
		for (i = 31; i >= 0; i--) {
			for (j = 0; j < SHAPE_BIG_MAX_HEIGHT; j++)
				if (container->shape[j] & (1 << i))
					break;
			if (j < SHAPE_BIG_MAX_HEIGHT)
				break;
		}
		node->size[0] = C_UNIT * (i + 1) + 0.01;

		/* start on the lower row of the shape mask */
		for (i = SHAPE_BIG_MAX_HEIGHT - 1; i >= 0; i--)
			if (container->shape[i] & ~0x0)
				break;
		node->size[1] = C_UNIT * (i + 1) + 0.01;
	}
}

static const vec3_t scale = {3.5, 3.5, 3.5};
/** @todo it may be nice to vectorise that */
static const vec4_t colorDefault = {1, 1, 1, 1};
static const vec4_t colorLoadable = {0.5, 1, 0.5, 1};
static const vec4_t colorDisabled = {0.5, 0.5, 0.5, 1};
static const vec4_t colorDisabledHiden = {0.5, 0.5, 0.5, 0.5};
static const vec4_t colorDisabledLoadable = {0.5, 0.25, 0.25, 1.0};
static const vec4_t colorPreview = { 0.5, 0.5, 1, 1 };	/**< Make the preview item look blueish */

/**
 * @brief Draw a container which only contains one item
 * @param node
 * @param hightlightType
 */
static void MN_ContainerNodeDrawSingle (menuNode_t *node, objDef_t *highlightType)
{
	vec4_t color;
	vec3_t pos;
	qboolean disabled = qfalse;

	MN_GetNodeAbsPos(node, pos);
	pos[0] += node->size[0] / 2.0;
	pos[1] += node->size[1] / 2.0;
	pos[2] = 0;

	/* Single item container (special case for left hand). */
	if (EXTRADATA(node).container->id == csi.idLeft && !menuInventory->c[csi.idLeft]) {
		if (menuInventory->c[csi.idRight]) {
			const item_t *item = &menuInventory->c[csi.idRight]->item;
			assert(item);
			assert(item->t);

			if (item->t->holdTwoHanded) {
				if (highlightType && INVSH_LoadableInWeapon(highlightType, item->t))
					memcpy(color, colorLoadable, sizeof(vec4_t));
				else
					memcpy(color, colorDefault, sizeof(vec4_t));
				color[3] = 0.5;
				MN_DrawItem(node, pos, item, -1, -1, scale, color);
			}
		}
	} else if (menuInventory->c[EXTRADATA(node).container->id]) {
		const item_t *item;

		if (menuInventory->c[csi.idRight]) {
			item = &menuInventory->c[csi.idRight]->item;
			/* If there is a weapon in the right hand that needs two hands to shoot it
			 * and there is a weapon in the left, then draw a disabled marker for the
			 * fireTwoHanded weapon. */
			assert(item);
			assert(item->t);
			if (EXTRADATA(node).container->id == csi.idRight && item->t->fireTwoHanded && menuInventory->c[csi.idLeft]) {
				disabled = qtrue;
				MN_DrawDisabled(node);
			}
		}

		item = &menuInventory->c[EXTRADATA(node).container->id]->item;
		assert(item);
		assert(item->t);
		if (highlightType && INVSH_LoadableInWeapon(highlightType, item->t)) {
			if (disabled)
				Vector4Copy(colorDisabledLoadable, color);
			else
				Vector4Copy(colorLoadable, color);
		} else {
			if (disabled)
				Vector4Copy(colorDisabled, color);
			else
				Vector4Copy(colorDefault, color);
		}
		if (disabled)
			color[3] = 0.5;
		MN_DrawItem(node, pos, item, -1, -1, scale, color);
	}
}

/**
 * @brief Searches if there is an item at location (x/y) in a scrollable container. You can also provide an item to search for directly (x/y is ignored in that case).
 * @note x = x-th item in a row, y = row. i.e. x/y does not equal the "grid" coordinates as used in those containers.
 * @param[in] i Pointer to the inventory where we will search.
 * @param[in] container Container in the inventory.
 * @param[in] x/y Position in the scrollable container that you want to check. Ignored if "item" is set.
 * @param[in] item The item to search. Will ignore "x" and "y" if set, it'll also search invisible items.
 * @return invList_t Pointer to the invList_t/item that is located at x/y or equals "item".
 * @sa Com_SearchInInventory
 */
static invList_t *MN_ContainerNodeGetExistingItem (const menuNode_t *node, objDef_t *item, const itemFilterTypes_t filterType)
{
	return Com_SearchInInventoryWithFilter(menuInventory, EXTRADATA(node).container, NONE, NONE, item, filterType);
}

/**
 * @brief Filter an object according to a node configuration
 * @param[in] obj Object request filtering
 * @param[out] itemFound Item found in the container (can be null)
 * @return True is the object is allowed
 */
static qboolean MN_ContainerNodeFilterItem (const menuNode_t const *node, int displayType, int displayAvailable, objDef_t *obj, invList_t **itemFound)
{
	qboolean isAmmo;
	qboolean isWeapon;
	qboolean isArmour;

	/* skip none researched objects */
	if (!RS_IsResearched_ptr(obj->tech))
		return qfalse;

	/** @todo not sure its the right check */
	isArmour = !Q_strcmp(obj->type, "armour");
	isAmmo = obj->numWeapons != 0 && !Q_strncmp(obj->type, "ammo", 4);
	isWeapon = obj->weapon || obj->isMisc || isArmour;

	if (isArmour && obj->useable != GAME_GetCurrentTeam())
		return qfalse;

	if ((displayType == 0 && isAmmo) || (displayType == 1 && isWeapon))
		return qfalse;

	if (!INV_ItemMatchesFilter(obj, EXTRADATA(node).filterEquipType))
		return qfalse;

	*itemFound = MN_ContainerNodeGetExistingItem(node, obj, EXTRADATA(node).filterEquipType);
	if (displayAvailable != -1) {
		if ((displayAvailable == 1 && *itemFound == NULL) || (displayAvailable == 0 && *itemFound != NULL))
			return qfalse;
	}

	return qtrue;
}

static void MN_ContainerNodeDrawItems (menuNode_t *node, objDef_t *highlightType,
	int *currentHeight, int displayType, int displayAvailable)
{
	qboolean outOfNode = qfalse;
	vec2_t nodepos;
	int items = 0;
	int id;
	int rowHeight = 0;
	const int cellWidth = node->size[0] / EXTRADATA(node).columns;

	MN_GetNodeAbsPos(node, nodepos);

	if (*currentHeight >= node->size[1]) {
		outOfNode = qtrue;
	}

	for (id = 0; id < csi.numODs; id++) {
		objDef_t *obj = &csi.ods[id];
		item_t tempItem = {1, NULL, obj, 0, 0};
		vec3_t pos;
		vec3_t ammopos;
		const float *color;
		qboolean isHilight = qfalse;
		int amount;
		const int col = items % node->u.container.columns;
		int cellHeight = 0;
		invList_t *icItem;

		if (!MN_ContainerNodeFilterItem(node, displayType, displayAvailable, obj, &icItem))
			continue;

		/* skip items over and bellow the node view */
		if (outOfNode || *currentHeight < EXTRADATA(node).scrollCur) {
			int height;
			R_FontTextSize("f_verysmall", _(obj->name),
				cellWidth - 5, LONGLINES_WRAP, NULL, &height, NULL, NULL);
			height += obj->sy * C_UNIT + 10;
			if (height > rowHeight)
				rowHeight = height;

			if (outOfNode || *currentHeight + rowHeight < EXTRADATA(node).scrollCur) {
				if (col == EXTRADATA(node).columns - 1) {
					*currentHeight += rowHeight;
					rowHeight = 0;
				}
				continue;
			}
		}

		Vector2Copy(nodepos, pos);
		pos[0] += cellWidth * col;
		pos[1] += *currentHeight - EXTRADATA(node).scrollCur;
		pos[2] = 0;

		if (highlightType) {
			if (!Q_strncmp(obj->type, "ammo", MAX_VAR))
				isHilight = INVSH_LoadableInWeapon(obj, highlightType);
			else
				isHilight = INVSH_LoadableInWeapon(highlightType, obj);
		}

		if (icItem != NULL) {
			if (isHilight)
				color = colorLoadable;
			else
				color = colorDefault;
		} else {
			if (isHilight)
				color = colorDisabledLoadable;
			else
				color = colorDisabledHiden;
		}

		if (icItem)
			amount = icItem->item.amount;
		else
			amount = 0;

		/* draw item */
		pos[0] += obj->sx * C_UNIT / 2.0;
		pos[1] += obj->sy * C_UNIT / 2.0;
		MN_DrawItem(node, pos, &tempItem, -1, -1, scale, color);
		R_FontDrawString("f_verysmall", ALIGN_LC,
			pos[0] + obj->sx * C_UNIT / 2.0, pos[1] + obj->sy * C_UNIT / 2.0,
			pos[0] + obj->sx * C_UNIT / 2.0, pos[1] + obj->sy * C_UNIT / 2.0,
			C_UNIT,	0,	/* maxWidth/maxHeight */
			0, va("x%i", amount), 0, 0, NULL, qfalse, 0);
		pos[0] -= obj->sx * C_UNIT / 2.0;
		pos[1] += obj->sy * C_UNIT / 2.0;
		cellHeight += obj->sy * C_UNIT;

		/* save position for ammo */
		Vector2Copy(pos, ammopos);
		ammopos[2] = 0;
		ammopos[0] += obj->sx * C_UNIT + 10;

		/* draw the item name. */
		cellHeight += R_FontDrawString("f_verysmall", ALIGN_UL,
			pos[0], pos[1],
			pos[0], nodepos[1],
			cellWidth - 5, 200,	/* max width/height */
			0, _(obj->name), 0, 0, NULL, qfalse, LONGLINES_WRAP);

		/* draw ammos of weapon */
		if (obj->weapon && EXTRADATA(node).displayAmmoOfWeapon) {
			int ammoIdx;
			for (ammoIdx = 0; ammoIdx < obj->numAmmos; ammoIdx++) {
				tempItem.t = obj->ammos[ammoIdx];

				/* skip unusable ammo */
				if (!tempItem.t->tech || !RS_IsResearched_ptr(tempItem.t->tech))
					continue;

				/* find and skip unexisting ammo */
				icItem = MN_ContainerNodeGetExistingItem(node, tempItem.t, EXTRADATA(node).filterEquipType);
				if (!icItem)
					continue;

				/* Calculate the center of the item model/image. */
				ammopos[0] += icItem->item.t->sx * C_UNIT / 2.0;
				ammopos[1] -= icItem->item.t->sy * C_UNIT / 2.0;
				MN_DrawItem(node, ammopos, &tempItem, -1, -1, scale, colorDefault);
				R_FontDrawString("f_verysmall", ALIGN_LC,
					ammopos[0] + icItem->item.t->sx * C_UNIT / 2.0, ammopos[1] + icItem->item.t->sy * C_UNIT / 2.0,
					ammopos[0] + icItem->item.t->sx * C_UNIT / 2.0, ammopos[1] + icItem->item.t->sy * C_UNIT / 2.0,
					C_UNIT,	/* maxWidth */
					0,	/* maxHeight */
					0, va("x%i", icItem->item.amount), 0, 0, NULL, qfalse, 0);
				ammopos[0] += icItem->item.t->sx * C_UNIT / 2.0;
				ammopos[1] += icItem->item.t->sy * C_UNIT / 2.0;
			}
		}
		cellHeight += 10;

		if (cellHeight > rowHeight) {
			rowHeight = cellHeight;
		}

		/* add a marge between rows */
		if (col == EXTRADATA(node).columns - 1) {
			*currentHeight += rowHeight;
			rowHeight = 0;
			if (*currentHeight - EXTRADATA(node).scrollCur >= node->size[1])
				outOfNode = qtrue;
		}

		/* count items */
		items++;
	}

	if (rowHeight != 0) {
		*currentHeight += rowHeight;
	}
}

/**
 * @brief Draw the inventory of the base
 */
static void MN_ContainerNodeDrawBaseInventory (menuNode_t *node, objDef_t *highlightType)
{
	qboolean updateScroll = qfalse;
	int currentHeight = 0;
	int visibleRows = 0;
	int totalRows = 0;

	R_BeginClipRect(node->pos[0], node->pos[1], node->size[0], node->size[1]);
	currentHeight = 0;

	if (EXTRADATA(node).displayAvailableOnTop) {
		/* available items */
		if (EXTRADATA(node).displayWeapon)
			MN_ContainerNodeDrawItems(node, highlightType, &currentHeight, 0, 1);
		if (EXTRADATA(node).displayAmmo)
			MN_ContainerNodeDrawItems(node, highlightType, &currentHeight, 1, 1);
		/* unavailable items */
		if (EXTRADATA(node).displayUnavailableItem) {
			if (EXTRADATA(node).displayWeapon)
				MN_ContainerNodeDrawItems(node, highlightType, &currentHeight, 0, 0);
			if (EXTRADATA(node).displayAmmo)
				MN_ContainerNodeDrawItems(node, highlightType, &currentHeight, 1, 0);
		}
	} else {
		const int availableFilter = (EXTRADATA(node).displayUnavailableItem) ? -1 : 1;
		if (EXTRADATA(node).displayWeapon)
			MN_ContainerNodeDrawItems (node, highlightType, &currentHeight, 0, availableFilter);
		if (EXTRADATA(node).displayAmmo)
			MN_ContainerNodeDrawItems (node, highlightType, &currentHeight, 1, availableFilter);
	}

	R_EndClipRect();
	totalRows = currentHeight;
	visibleRows = node->size[1];

#if 0
	R_FontDrawString("f_verysmall", ALIGN_UL,
		node->pos[0], node->pos[1], node->pos[0], node->pos[1],
		0,	0,	/* maxWidth/maxHeight */
		0, va("%i %i/%i", EXTRADATA(node).scrollCur, visibleRows, totalRows), 0, 0, NULL, qfalse, 0);
#endif

	/* Update display of scroll buttons if something changed. */
	if (visibleRows != EXTRADATA(node).scrollNum || totalRows != EXTRADATA(node).scrollTotalNum) {
		EXTRADATA(node).scrollTotalNum = totalRows;
		EXTRADATA(node).scrollNum = visibleRows;
		updateScroll = qtrue;
	}
	if (EXTRADATA(node).scrollCur > totalRows - visibleRows) {
		EXTRADATA(node).scrollCur = totalRows - visibleRows;
		updateScroll = qtrue;
	}
	if (EXTRADATA(node).scrollCur < 0) {
		EXTRADATA(node).scrollCur = 0;
		updateScroll = qtrue;
	}

	if (updateScroll)
		MN_ContainerNodeUpdateScroll(node);
}

/**
 * @brief Draw a grip container
 */
static void MN_ContainerNodeDrawGrid (menuNode_t *node, objDef_t *highlightType)
{
	const invList_t *ic;
	vec3_t pos;

	MN_GetNodeAbsPos(node, pos);
	pos[2] = 0;

	for (ic = menuInventory->c[EXTRADATA(node).container->id]; ic; ic = ic->next) {
		assert(ic->item.t);
		if (highlightType && INVSH_LoadableInWeapon(highlightType, ic->item.t))
			MN_DrawItem(node, pos, &ic->item, ic->x, ic->y, scale, colorLoadable);
		else
			MN_DrawItem(node, pos, &ic->item, ic->x, ic->y, scale, colorDefault);
	}
}

/**
 * @brief Draw a preview of the DND item dropped into the node
 */
static void MN_ContainerNodeDrawDropPreview (menuNode_t *target)
{
	item_t previewItem;
	int checkedTo;
	vec3_t origine;

	/* no preview into scrollable list */
	if (MN_IsScrollContainerNode(target))
		return;

	/* copy the DND item to not change the original one */
	previewItem = *MN_DNDGetItem();
	previewItem.rotated = qfalse;
	checkedTo = Com_CheckToInventory(menuInventory, previewItem.t, EXTRADATA(target).container, dragInfoToX, dragInfoToY, dragInfoIC);
	if (checkedTo == INV_FITS_ONLY_ROTATED)
		previewItem.rotated = qtrue;

	/* no place found */
	if (!checkedTo)
		return;

	/* Hack, no preview for armour, we dont want it out of the armour container (and armour container is not visible) */
	if (!Q_strncmp(previewItem.t->type, "armour", MAX_VAR))
		return;

	MN_GetNodeAbsPos(target, origine);
	origine[2] = -40;

	/* Get center of single container for placement of preview item */
	if (EXTRADATA(target).container->single) {
		origine[0] += target->size[0] / 2.0;
		origine[1] += target->size[1] / 2.0;
	/* This is a "grid" container - we need to calculate the item-position
	 * (on the screen) from stored placement in the container and the
	 * calculated rotation info. */
	} else {
		if (previewItem.rotated) {
			origine[0] += (dragInfoToX + previewItem.t->sy / 2.0) * C_UNIT;
			origine[1] += (dragInfoToY + previewItem.t->sx / 2.0) * C_UNIT;
		} else {
			origine[0] += (dragInfoToX + previewItem.t->sx / 2.0) * C_UNIT;
			origine[1] += (dragInfoToY + previewItem.t->sy / 2.0) * C_UNIT;
		}
	}

	MN_DrawItem(NULL, origine, &previewItem, -1, -1, scale, colorPreview);
}

/**
 * @brief Main function to draw a container node
 */
static void MN_ContainerNodeDraw (menuNode_t *node)
{
	objDef_t *highlightType = NULL;

	if (!EXTRADATA(node).container)
		return;
	if (!menuInventory)
		return;
	/* is container invisible */
	if (node->color[3] < 0.001)
		return;

	/* Highlight weapons that the dragged ammo (if it is one) can be loaded into. */
	if (MN_DNDIsDragging() && MN_DNDGetType() == DND_ITEM) {
		highlightType = MN_DNDGetItem()->t;
	}

	if (EXTRADATA(node).container->single) {
		MN_ContainerNodeDrawSingle(node, highlightType);
	} else {
		if (MN_IsScrollContainerNode(node)) {
			MN_ContainerNodeDrawBaseInventory(node, highlightType);
		} else {
			MN_ContainerNodeDrawGrid(node, highlightType);
		}
	}

	/* Draw free space if dragging - but not for idEquip */
	if (MN_DNDIsDragging() && EXTRADATA(node).container->id != csi.idEquip)
		MN_ContainerNodeDrawFreeSpace(node, menuInventory);

	if (MN_DNDIsTargetNode(node))
		MN_ContainerNodeDrawDropPreview(node);
}

/**
 * @note this function is a copy-paste of MN_ContainerNodeDrawItems (with remove of unneed code)
 */
static invList_t *MN_ContainerNodeGetItemFromSplitedList (const menuNode_t* const node,
	int *currentHeight, int displayType, int displayAvailable,
	int mouseX, int mouseY, int* contX, int* contY)
{
	qboolean outOfNode = qfalse;
	vec2_t nodepos;
	int items = 0;
	int id;
	int rowHeight = 0;
	const int cellWidth = node->size[0] / EXTRADATA(node).columns;
	int tempX, tempY;

	if (!contX)
		contX = &tempX;
	if (!contY)
		contY = &tempY;

	MN_GetNodeAbsPos(node, nodepos);

	if (*currentHeight >= node->size[1]) {
		outOfNode = qtrue;
	}

	for (id = 0; id < csi.numODs; id++) {
		objDef_t *obj = &csi.ods[id];
		vec2_t pos;
		vec2_t ammopos;
		const int col = items % node->u.container.columns;
		int cellHeight = 0;
		invList_t *icItem;
		int height;

		if (!MN_ContainerNodeFilterItem(node, displayType, displayAvailable, obj, &icItem))
			continue;

		/* skip items over and bellow the node view */
		if (outOfNode || *currentHeight < EXTRADATA(node).scrollCur) {
			int height;
			R_FontTextSize("f_verysmall", _(obj->name),
				cellWidth - 5, LONGLINES_WRAP, NULL, &height, NULL, NULL);
			height += obj->sy * C_UNIT + 10;
			if (height > rowHeight)
				rowHeight = height;

			if (outOfNode || *currentHeight + rowHeight < EXTRADATA(node).scrollCur) {
				if (col == EXTRADATA(node).columns - 1) {
					*currentHeight += rowHeight;
					rowHeight = 0;
				}
				continue;
			}
		}

		Vector2Copy(nodepos, pos);
		pos[0] += cellWidth * col;
		pos[1] += *currentHeight - EXTRADATA(node).scrollCur;

		/* check item */
		if (mouseY < pos[1])
			break;
		if (mouseX >= pos[0] && mouseX < pos[0] + obj->sx * C_UNIT
		 && mouseY >= pos[1] && mouseY < pos[1] + obj->sy * C_UNIT) {
			if (icItem) {
				*contX = icItem->x;
				*contY = icItem->y;
				return icItem;
			} else {
				return NULL;
			}
		}
		pos[1] += obj->sy * C_UNIT;
		cellHeight += obj->sy * C_UNIT;

		/* save position for ammo */
		Vector2Copy(pos, ammopos);
		ammopos[0] += obj->sx * C_UNIT + 10;

		/* draw the item name. */
		R_FontTextSize("f_verysmall", _(obj->name),
			cellWidth - 5, LONGLINES_WRAP, NULL, &height, NULL, NULL);
		cellHeight += height;

		/* draw ammos of weapon */
		if (obj->weapon && EXTRADATA(node).displayAmmoOfWeapon) {
			int ammoIdx;
			for (ammoIdx = 0; ammoIdx < obj->numAmmos; ammoIdx++) {
				objDef_t *objammo = obj->ammos[ammoIdx];

				/* skip unusable ammo */
				if (!objammo->tech || !RS_IsResearched_ptr(objammo->tech))
					continue;

				/* find and skip unexisting ammo */
				icItem = MN_ContainerNodeGetExistingItem(node, objammo, EXTRADATA(node).filterEquipType);
				if (!icItem)
					continue;

				/* check ammo (ammopos in on the left-lower corner) */
				if (mouseX < ammopos[0] || mouseY >= ammopos[1])
					break;
				if (mouseX >= ammopos[0] && mouseX < ammopos[0] + objammo->sx * C_UNIT
				 && mouseY >= ammopos[1] - objammo->sy * C_UNIT && mouseY < ammopos[1]) {
					*contX = icItem->x;
					*contY = icItem->y;
					return icItem;
				}
				ammopos[0] += objammo->sx * C_UNIT;
			}
		}
		cellHeight += 10;

		if (cellHeight > rowHeight) {
			rowHeight = cellHeight;
		}

		/* add a marge between rows */
		if (col == EXTRADATA(node).columns - 1) {
			*currentHeight += rowHeight;
			rowHeight = 0;
			if (*currentHeight - EXTRADATA(node).scrollCur >= node->size[1])
				return NULL;
		}

		/* count items */
		items++;
	}

	if (rowHeight != 0) {
		*currentHeight += rowHeight;
		rowHeight = 0;
	}

	*contX = NONE;
	*contY = NONE;
	return NULL;
}

/**
 * @brief Gets location of the item the mouse is over
 * @param[in] node	The container-node.
 * @param[in] mouseX	X location of the mouse.
 * @param[in] mouseY	Y location of the mouse.
 * @param[out] contX	X location in the container (index of item in row).
 * @param[out] contY	Y location in the container (row).
 * @sa MN_ContainerNodeSearchInScrollableContainer
 */
static invList_t *MN_ContainerNodeGetItemAtPosition (const menuNode_t* const node, int mouseX, int mouseY, int* contX, int* contY)
{
	invList_t *result = NULL;
	/* Get coordinates inside a scrollable container (if it is one). */
	if (MN_IsScrollContainerNode(node)) {
		int currentHeight = 0;
		if (EXTRADATA(node).displayAvailableOnTop) {
			/* available items */
			if (EXTRADATA(node).displayWeapon)
				result = MN_ContainerNodeGetItemFromSplitedList (node, &currentHeight, 0, 1, mouseX, mouseY, contX, contY);
			if (!result && EXTRADATA(node).displayAmmo)
				result = MN_ContainerNodeGetItemFromSplitedList (node, &currentHeight, 1, 1, mouseX, mouseY, contX, contY);
			/* unavailable items */
			if (!result && EXTRADATA(node).displayUnavailableItem) {
				if (EXTRADATA(node).displayWeapon)
					result = MN_ContainerNodeGetItemFromSplitedList (node, &currentHeight, 0, 0, mouseX, mouseY, contX, contY);
				if (!result && EXTRADATA(node).displayAmmo)
					result = MN_ContainerNodeGetItemFromSplitedList (node, &currentHeight, 1, 0, mouseX, mouseY, contX, contY);
			}
		} else {
			const int availableFilter = (EXTRADATA(node).displayUnavailableItem)?-1:1;
			if (EXTRADATA(node).displayWeapon)
				result = MN_ContainerNodeGetItemFromSplitedList (node, &currentHeight,0, availableFilter, mouseX, mouseY, contX, contY);
			if (!result && EXTRADATA(node).displayAmmo)
				result = MN_ContainerNodeGetItemFromSplitedList (node, &currentHeight, 1, availableFilter, mouseX, mouseY, contX, contY);
		}
	} else {
		vec2_t nodepos;
		int fromX, fromY;

		MN_GetNodeAbsPos(node, nodepos);
		/* Normalize screen coordinates to container coordinates. */
		fromX = (int) (mouseX - nodepos[0]) / C_UNIT;
		fromY = (int) (mouseY - nodepos[1]) / C_UNIT;
		if (contX)
			*contX = fromX;
		if (contY)
			*contY = fromY;

		result = Com_SearchInInventory(menuInventory, EXTRADATA(node).container, fromX, fromY);
	}
	return result;
}

/**
 * @brief Custom tooltip for container node
 * @param[in] node Node we request to draw tooltip
 * @param[in] x Position x of the mouse
 * @param[in] y Position y of the mouse
 */
static void MN_ContainerNodeDrawTooltip (menuNode_t *node, int x, int y)
{
	static char tooltiptext[MAX_VAR * 2];
	const invList_t *itemHover;
	vec2_t nodepos;

	MN_GetNodeAbsPos(node, nodepos);

	/* Find out where the mouse is. */
	itemHover = MN_ContainerNodeGetItemAtPosition(node, x, y, NULL, NULL);

	if (itemHover) {
		const int itemToolTipWidth = 250;

		/* Get name and info about item */
		MN_GetItemTooltip(itemHover->item, tooltiptext, sizeof(tooltiptext));
#ifdef DEBUG
		/* Display stored container-coordinates of the item. */
		Q_strcat(tooltiptext, va("\n%i/%i", itemHover->x, itemHover->y), sizeof(tooltiptext));
#endif
		MN_DrawTooltip("f_small", tooltiptext, x, y, itemToolTipWidth, 0);
	}
}

/**
 * @brief Try to autoplace an item at a position
 * when right-click was used in the inventory.
 * @param[in] node The node in the menu that the mouse is over (i.e. a container node).
 * @param[in] base The base we are in.
 * @param[in] mouseX/mouseY Mouse coordinates.
 * @param[in] rightClick If we want to auto-assign items instead of dragging them this has to be qtrue.
 * @todo Ungeneric function, Not sure we can do it in a generic way
 */
static void MN_ContainerNodeAutoPlace (menuNode_t* node, int mouseX, int mouseY)
{
	int sel;
#if 0	/* see bellow #1 */
	int id;
#endif
	invList_t *ic;
	int fromX, fromY;

	if (!menuInventory)
		return;

	/* don't allow this in tactical missions */
	if (CL_OnBattlescape())
		return;

	sel = cl_selected->integer;
	if (sel < 0)
		return;

	assert(EXTRADATA(node).container);

	ic = MN_ContainerNodeGetItemAtPosition(node, mouseX, mouseY, &fromX, &fromY);
	Com_DPrintf(DEBUG_CLIENT, "MN_ContainerNodeAutoPlace: item %i/%i selected from scrollable container.\n", fromX, fromY);
	if (!ic)
		return;
#if 0	/* see bellow #1 */
	id = ic->item.t->idx;
#endif

	/* Right click: automatic item assignment/removal. */
	if (EXTRADATA(node).container->id != csi.idEquip) {
		/* Move back to idEquip (ground, floor) container. */
		INV_MoveItem(menuInventory, &csi.ids[csi.idEquip], NONE, NONE, EXTRADATA(node).container, ic);
	} else {
		qboolean packed = qfalse;
		int px, py;
		assert(ic->item.t);
		/* armour can only have one target */
		if (!Q_strncmp(ic->item.t->type, "armour", MAX_VAR)) {
			packed = INV_MoveItem(menuInventory, &csi.ids[csi.idArmour], 0, 0, EXTRADATA(node).container, ic);
		/* ammo or item */
		} else if (!Q_strncmp(ic->item.t->type, "ammo", MAX_VAR)) {
			Com_FindSpace(menuInventory, &ic->item, &csi.ids[csi.idBelt], &px, &py, NULL);
			packed = INV_MoveItem(menuInventory, &csi.ids[csi.idBelt], px, py, EXTRADATA(node).container, ic);
			if (!packed) {
				Com_FindSpace(menuInventory, &ic->item, &csi.ids[csi.idHolster], &px, &py, NULL);
				packed = INV_MoveItem(menuInventory, &csi.ids[csi.idHolster], px, py, EXTRADATA(node).container, ic);
			}
			if (!packed) {
				Com_FindSpace(menuInventory, &ic->item, &csi.ids[csi.idBackpack], &px, &py, NULL);
				packed = INV_MoveItem( menuInventory, &csi.ids[csi.idBackpack], px, py, EXTRADATA(node).container, ic);
			}
			/* Finally try left and right hand. There is no other place to put it now. */
			if (!packed) {
				const invList_t *rightHand = Com_SearchInInventory(menuInventory, &csi.ids[csi.idRight], 0, 0);

				/* Only try left hand if right hand is empty or no twohanded weapon/item is in it. */
				if (!rightHand || (rightHand && !rightHand->item.t->fireTwoHanded)) {
					Com_FindSpace(menuInventory, &ic->item, &csi.ids[csi.idLeft], &px, &py, NULL);
					packed = INV_MoveItem(menuInventory, &csi.ids[csi.idLeft], px, py, EXTRADATA(node).container, ic);
				}
			}
			if (!packed) {
				Com_FindSpace(menuInventory, &ic->item, &csi.ids[csi.idRight], &px, &py, NULL);
				packed = INV_MoveItem(menuInventory, &csi.ids[csi.idRight], px, py, EXTRADATA(node).container, ic);
			}
		} else {
			if (ic->item.t->headgear) {
				Com_FindSpace(menuInventory, &ic->item, &csi.ids[csi.idHeadgear], &px, &py, NULL);
				packed = INV_MoveItem(menuInventory, &csi.ids[csi.idHeadgear], px, py, EXTRADATA(node).container, ic);
			} else {
				/* left and right are single containers, but this might change - it's cleaner to check
				 * for available space here, too */
				Com_FindSpace(menuInventory, &ic->item, &csi.ids[csi.idRight], &px, &py, NULL);
				packed = INV_MoveItem(menuInventory, &csi.ids[csi.idRight], px, py, EXTRADATA(node).container, ic);
				if (!packed) {
					const invList_t *rightHand = Com_SearchInInventory(menuInventory, &csi.ids[csi.idRight], 0, 0);

					/* Only try left hand if right hand is empty or no twohanded weapon/item is in it. */
					if (!rightHand || (rightHand && !rightHand->item.t->fireTwoHanded)) {
						Com_FindSpace(menuInventory, &ic->item, &csi.ids[csi.idLeft], &px, &py, NULL);
						packed = INV_MoveItem(menuInventory, &csi.ids[csi.idLeft], px, py, EXTRADATA(node).container, ic);
					}
				}
				if (!packed) {
					Com_FindSpace(menuInventory, &ic->item, &csi.ids[csi.idBelt], &px, &py, NULL);
					packed = INV_MoveItem(menuInventory, &csi.ids[csi.idBelt], px, py, EXTRADATA(node).container, ic);
				}
				if (!packed) {
					Com_FindSpace(menuInventory, &ic->item, &csi.ids[csi.idHolster], &px, &py, NULL);
					packed = INV_MoveItem(menuInventory, &csi.ids[csi.idHolster], px, py, EXTRADATA(node).container, ic);
				}
				if (!packed) {
					Com_FindSpace(menuInventory, &ic->item, &csi.ids[csi.idBackpack], &px, &py, NULL);
					packed = INV_MoveItem(menuInventory, &csi.ids[csi.idBackpack], px, py, EXTRADATA(node).container, ic);
				}
			}
		}
		/* no need to continue here - placement wasn't successful at all */
		if (!packed)
			return;
	}

	/** HACK: Hard to know where the item is located now, but if its an armor
	 * we fire the change event of the armour container. At least to
	 * update the actor skin.
	 * The right way is to compute the source and the target container
	 * and fire the change event for both */
	if (!Q_strncmp(ic->item.t->type, "armour", MAX_VAR)) {
		const menuNode_t *armour = MN_GetNode(node->menu, "armour");
		if (armour && armour->onChange)
			MN_ExecuteEventActions(armour, armour->onChange);
	}

	/* Update display of scroll buttons. */
	if (MN_IsScrollContainerNode(node))
		MN_ContainerNodeUpdateScroll(node);
}

static int oldMouseX = 0;
static int oldMouseY = 0;

static void MN_ContainerNodeCapturedMouseMove (menuNode_t *node, int x, int y)
{
	const int delta = abs(oldMouseX - x) + abs(oldMouseY - y);
	if (delta > 15) {
		MN_DNDDragItem(node, &(dragInfoIC->item));
		MN_MouseRelease();
	}
}

static void MN_ContainerNodeMouseDown (menuNode_t *node, int x, int y, int button)
{
	switch (button) {
	case K_MOUSE1:
	{
		/* start drag and drop */
		int fromX, fromY;
		dragInfoIC = MN_ContainerNodeGetItemAtPosition(node, x, y, &fromX, &fromY);
		if (dragInfoIC) {
			dragInfoFromX = fromX;
			dragInfoFromY = fromY;
			oldMouseX = x;
			oldMouseY = y;
			MN_SetMouseCapture(node);
			EXTRADATA(node).lastSelectedId = dragInfoIC->item.t->idx;
			if (EXTRADATA(node).onSelect) {
				MN_ExecuteEventActions(node, EXTRADATA(node).onSelect);
			}
		}
		break;
	}
	case K_MOUSE2:
		if (MN_DNDIsDragging()) {
			MN_DNDAbort();
		} else {
			/* auto place */
			MN_ContainerNodeAutoPlace(node, x, y);
		}
		break;
	default:
		break;
	}
}

static void MN_ContainerNodeMouseUp (menuNode_t *node, int x, int y, int button)
{
	if (button != K_MOUSE1)
		return;
	if (MN_GetMouseCapture() == node) {
		MN_MouseRelease();
	}
	if (MN_DNDIsDragging()) {
		MN_DNDDrop();
	}
}
static void MN_ContainerNodeWheel (menuNode_t *node, qboolean down, int x, int y)
{
	if (MN_IsScrollContainerNode(node)) {
		const int delta = 20;
		if (down) {
			const int lenght = EXTRADATA(node).scrollTotalNum - EXTRADATA(node).scrollNum;
			if (EXTRADATA(node).scrollCur < lenght) {
				EXTRADATA(node).scrollCur += delta;
				if (EXTRADATA(node).scrollCur > lenght)
					EXTRADATA(node).scrollCur = lenght;
				MN_ContainerNodeUpdateScroll(node);
			}
		} else {
			if (EXTRADATA(node).scrollCur > 0) {
				EXTRADATA(node).scrollCur -= delta;
				if (EXTRADATA(node).scrollCur < 0)
					EXTRADATA(node).scrollCur = 0;
				MN_ContainerNodeUpdateScroll(node);
			}
		}
	}
}

static void MN_ContainerNodeLoading (menuNode_t *node)
{
	EXTRADATA(node).container = NULL;
	EXTRADATA(node).columns = 1;
	node->color[3] = 1.0;
}

/**
 * @brief Call when a DND enter into the node
 */
static qboolean MN_ContainerNodeDNDEnter (menuNode_t *target)
{
	/* accept items only, if we have a container */
	return MN_DNDGetType() == DND_ITEM && EXTRADATA(target).container;
}

/**
 * @brief Call into the target when the DND hover it
 * @return True if the DND is accepted
 */
static qboolean MN_ContainerNodeDNDMove (menuNode_t *target, int x, int y)
{
	vec2_t nodepos;
	qboolean exists;
	int itemX = 0;
	int itemY = 0;
	int checkedTo = INV_DOES_NOT_FIT;
	item_t *dragItem = MN_DNDGetItem();

	/* we already check it when the node accepte the DND */
	assert(EXTRADATA(target).container);

	MN_GetNodeAbsPos(target, nodepos);

	/** We calculate the position of the top-left corner of the dragged
	 * item in oder to compensate for the centered-drawn cursor-item.
	 * Or to be more exact, we calculate the relative offset from the cursor
	 * location to the middle of the top-left square of the item.
	 * @sa MN_LeftClick */
	if (dragItem->t) {
		itemX = C_UNIT * dragItem->t->sx / 2;	/* Half item-width. */
		itemY = C_UNIT * dragItem->t->sy / 2;	/* Half item-height. */

		/* Place relative center in the middle of the square. */
		itemX -= C_UNIT / 2;
		itemY -= C_UNIT / 2;
	}

	dragInfoToX = (mousePosX - nodepos[0] - itemX) / C_UNIT;
	dragInfoToY = (mousePosY - nodepos[1] - itemY) / C_UNIT;

	/** Check if the items already exists in the container. i.e. there is already at least one item.
	 * @sa Com_AddToInventory */
	exists = qfalse;
	if ((EXTRADATA(target).container->id == csi.idFloor || EXTRADATA(target).container->id == csi.idEquip)
		&& (dragInfoToX  < 0 || dragInfoToY < 0 || dragInfoToX >= SHAPE_BIG_MAX_WIDTH || dragInfoToY >= SHAPE_BIG_MAX_HEIGHT)
		&& Com_ExistsInInventory(menuInventory, EXTRADATA(target).container, *dragItem)) {
		exists = qtrue;
	}

	/** Search for a suitable position to render the item at if
	 * the container is "single", the cursor is out of bound of the container.
	 */
	if (!exists && dragItem->t && (EXTRADATA(target).container->single
		|| dragInfoToX  < 0 || dragInfoToY < 0
		|| dragInfoToX >= SHAPE_BIG_MAX_WIDTH || dragInfoToY >= SHAPE_BIG_MAX_HEIGHT)) {
#if 0
/* ... or there is something in the way. */
/* We would need to check for weapon/ammo as well here, otherwise a preview would be drawn as well when hovering over the correct weapon to reload. */
		|| (Com_CheckToInventory(menuInventory, dragItem->t, EXTRADATA(target).container, dragInfoToX, dragInfoToY) == INV_DOES_NOT_FIT)) {
#endif
		Com_FindSpace(menuInventory, dragItem, EXTRADATA(target).container, &dragInfoToX, &dragInfoToY, dragInfoIC);
	}

	if (!MN_IsScrollContainerNode(target)) {
		checkedTo = Com_CheckToInventory(menuInventory, dragItem->t, EXTRADATA(target).container, dragInfoToX, dragInfoToY, dragInfoIC);
		return checkedTo != 0;
	}

	return qtrue;
}

/**
 * @brief Call when a DND enter into the node
 */
static void MN_ContainerNodeDNDLeave (menuNode_t *node)
{
	dragInfoToX = -1;
	dragInfoToY = -1;
}

/**
 * @brief Call into the source when the DND end
 */
static qboolean MN_ContainerNodeDNDFinished (menuNode_t *source, qboolean isDroped)
{
	item_t *dragItem = MN_DNDGetItem();

	/* if the target can't finalyse the DND we stop */
	if (!isDroped) {
		return qfalse;
	}

	/* on tactical mission */
	if (CL_OnBattlescape()) {
		const menuNode_t *target = MN_DNDGetTargetNode();
		assert(EXTRADATA(source).container);
		assert(target);
		assert(EXTRADATA(target).container);
		MSG_Write_PA(PA_INVMOVE, selActor->entnum,
			EXTRADATA(source).container->id, dragInfoFromX, dragInfoFromY,
			EXTRADATA(target).container->id, dragInfoToX, dragInfoToY);
	} else {
		menuNode_t *target;

#if 0 /* is it realy need? */
		if (EXTRADATA(source).container->id == csi.idArmour) {
			/** hackhack @todo This is only because armour containers (and their nodes) are
			 * handled differently than normal containers somehow.
			 * dragInfo is not updated in MN_DrawMenus for them, this needs to be fixed.
			 * In a perfect world EXTRADATA(node).container would always be the same as dragInfo.to here. */
			if (MN_DNDGetTargetNode() == source) {
				/* dragInfo.to = EXTRADATA(source).container; */
				dragInfoToX = 0;
				dragInfoToY = 0;
			}
		}
#endif
		target = MN_DNDGetTargetNode();
		if (target) {
			invList_t *fItem;
			/* menu */
			/** @todo Is filterEquipType here is need?, we can use anyway Com_SearchInInventory if we disable dragInfoFromX/Y when we start DND */
			if (MN_IsScrollContainerNode(source)) {
				const int equipType = EXTRADATA(source).filterEquipType;
				fItem = MN_ContainerNodeGetExistingItem(source, dragItem->t, equipType);
			} else
				fItem = Com_SearchInInventory(menuInventory, EXTRADATA(source).container, dragInfoFromX, dragInfoFromY);

			/** @todo We must split the move in two. Here, we sould not know how to add the item to the target (see dndDrop) */
			assert(EXTRADATA(target).container);
			INV_MoveItem(menuInventory,
				EXTRADATA(target).container, dragInfoToX, dragInfoToY,
				EXTRADATA(source).container, fItem);
			if (source->onChange)
				MN_ExecuteEventActions(source, source->onChange);
			if (source != target && target->onChange)
				MN_ExecuteEventActions(target, target->onChange);
		}
	}

	dragInfoFromX = -1;
	dragInfoFromY = -1;
	return qtrue;
}

static const value_t properties[] = {
	{"displayweapon", V_BOOL, offsetof(menuNode_t, u.container.displayWeapon),  MEMBER_SIZEOF(menuNode_t, u.container.displayWeapon)},
	{"displayammo", V_BOOL, offsetof(menuNode_t, u.container.displayAmmo),  MEMBER_SIZEOF(menuNode_t, u.container.displayAmmo)},
	{"displayunavailableitem", V_BOOL, offsetof(menuNode_t, u.container.displayUnavailableItem),  MEMBER_SIZEOF(menuNode_t, u.container.displayUnavailableItem)},
	{"displayavailableontop", V_BOOL, offsetof(menuNode_t, u.container.displayAvailableOnTop),  MEMBER_SIZEOF(menuNode_t, u.container.displayAvailableOnTop)},
	{"displayammoofweapon", V_BOOL, offsetof(menuNode_t, u.container.displayAmmoOfWeapon),  MEMBER_SIZEOF(menuNode_t, u.container.displayAmmoOfWeapon)},
	{"displayunavailableammoofweapon", V_BOOL, offsetof(menuNode_t, u.container.displayUnavailableAmmoOfWeapon),  MEMBER_SIZEOF(menuNode_t, u.container.displayUnavailableAmmoOfWeapon)},
	{"columns", V_INT, offsetof(menuNode_t, u.container.columns),  MEMBER_SIZEOF(menuNode_t, u.container.columns)},
	{"filter", V_INT, offsetof(menuNode_t, u.container.filterEquipType),  MEMBER_SIZEOF(menuNode_t, u.container.filterEquipType)},

	{"lastselectedid", V_INT, offsetof(menuNode_t, u.container.lastSelectedId),  MEMBER_SIZEOF(menuNode_t, u.container.lastSelectedId)},
	{"select", V_SPECIAL_ACTION, offsetof(menuNode_t, u.container.onSelect),  MEMBER_SIZEOF(menuNode_t, u.container.onSelect)},

	{"scrollpos", V_INT, offsetof(menuNode_t, u.container.scrollCur),  MEMBER_SIZEOF(menuNode_t, u.container.scrollCur)},
	{"viewsize", V_INT, offsetof(menuNode_t, u.container.scrollNum),  MEMBER_SIZEOF(menuNode_t, u.container.scrollNum)},
	{"fullsize", V_INT, offsetof(menuNode_t, u.container.scrollTotalNum),  MEMBER_SIZEOF(menuNode_t, u.container.scrollTotalNum)},
	{"viewchange", V_SPECIAL_ACTION, offsetof(menuNode_t, u.container.onViewChange), MEMBER_SIZEOF(menuNode_t, u.container.onViewChange)},
	{NULL, V_NULL, 0, 0}
};

void MN_RegisterContainerNode (nodeBehaviour_t* behaviour)
{
	behaviour->name = "container";
	behaviour->draw = MN_ContainerNodeDraw;
	behaviour->drawTooltip = MN_ContainerNodeDrawTooltip;
	behaviour->mouseDown = MN_ContainerNodeMouseDown;
	behaviour->mouseUp = MN_ContainerNodeMouseUp;
	behaviour->capturedMouseMove = MN_ContainerNodeCapturedMouseMove;
	behaviour->loading = MN_ContainerNodeLoading;
	behaviour->loaded = MN_ContainerNodeLoaded;
	behaviour->dndEnter = MN_ContainerNodeDNDEnter;
	behaviour->dndFinished = MN_ContainerNodeDNDFinished;
	behaviour->dndMove = MN_ContainerNodeDNDMove;
	behaviour->dndLeave = MN_ContainerNodeDNDLeave;
	behaviour->mouseWheel = MN_ContainerNodeWheel;
	behaviour->properties = properties;

#if 0
	Cmd_AddCommand("scrollcont_next", MN_ScrollContainerNext_f, "Scrolls the current container (forward).");
	Cmd_AddCommand("scrollcont_prev", MN_ScrollContainerPrev_f, "Scrolls the current container (backward).");
	Cmd_AddCommand("scrollcont_scroll", MN_ScrollContainerScroll_f, "Scrolls the current container.");
#endif
}
