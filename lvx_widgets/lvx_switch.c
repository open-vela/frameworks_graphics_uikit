/*********************
 *      INCLUDES
 *********************/
#include "lvx_switch.h"

#if (LVX_USE_SWITCH != 0 && LV_USE_SWITCH != 0)

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_switch_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_switch_class = {
    .instance_size = sizeof(lvx_switch_t),
    .width_def = SWITCH_DEFAULT_WIDTH,
    .height_def = SWITCH_DEFAULT_HEIGHT,
    .base_class = &lv_switch_class,
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * Create a switch object
 * @param parent pointer to an object, it will be the parent of the new obj.
 * @return pointer to the created obj
 */
lv_obj_t* lvx_switch_create(lv_obj_t* parent)
{
    LVX_WIDGET_CREATE(MY_CLASS, parent);
}

/*======================
 * Add/remove functions
 *=====================*/

/*
 * New object specific "add" or "remove" functions come here
 */

/*=====================
 * Setter functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
