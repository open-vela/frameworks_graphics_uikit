/*********************
 *      INCLUDES
 *********************/
#include "lvx_radio.h"

#if (LVX_USE_RADIO != 0)

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_radio_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_radio_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_radio_event_cb(const lv_obj_class_t* class_p, lv_event_t* e);
static void lvx_radio_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_radio_class = {
    .constructor_cb = lvx_radio_constructor,
    .event_cb = lvx_radio_event_cb,
    .destructor_cb = lvx_radio_destructor,
    .width_def = RADIO_DEFAULT_WIDTH,
    .height_def = RADIO_DEFAULT_HEIGHT,
    .instance_size = sizeof(lvx_radio_t),
    .base_class = &lvx_btn_class,
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lvx_radio_group_t* lvx_radio_group_create(void)
{
    lvx_radio_group_t* group = lv_mem_alloc(sizeof(lvx_radio_group_t));
    _lv_ll_init(&group->obj_list, sizeof(lv_obj_t*));
    return group;
}

void lvx_radio_group_destroy(lvx_radio_group_t* group)
{
    if (group == NULL) {
        return;
    }

    lv_obj_t** cnt = NULL;
    _LV_LL_READ(&group->obj_list, cnt)
    {
        lv_obj_t* obj = *cnt;
        lvx_radio_t* radio = (lvx_radio_t*)obj;
        radio->group = NULL;
    }

    _lv_ll_clear(&group->obj_list);
}

void lvx_radio_group_add_radio(lvx_radio_group_t* group, lv_obj_t* obj)
{
    lv_obj_t** chlid = NULL;
    _LV_LL_READ(&group->obj_list, chlid)
    {
        if (*chlid == obj) {
            return;
        }
    }

    lv_obj_t* new = _lv_ll_ins_tail(&group->obj_list);
    lvx_radio_t* radio = (lvx_radio_t*)obj;
    radio->group = group;
    *(lv_obj_t**)new = obj;
}

void lvx_radio_group_remove_radio(lvx_radio_group_t* group, lv_obj_t* obj)
{
    lv_obj_t** new = NULL;
    _LV_LL_READ(&group->obj_list, new)
    {
        if (*new == obj) {
            _lv_ll_remove(&group->obj_list, new);
            break;
        }
    }
}

/**
 * Create a btn object, which
 * @param parent pointer to an object, it will be the parent of the new arc
 * @return pointer to the created arc
 */
lv_obj_t* lvx_radio_create(lv_obj_t* parent)
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

void lvx_radio_set_img_src(lv_obj_t* obj, const void* src)
{
    lv_obj_set_style_bg_img_src(obj, src, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(obj, src, LV_STATE_PRESSED);
    lv_obj_set_style_bg_img_src(obj, src, LV_STATE_DISABLED);
}

void lvx_radio_set_style_bg_color(lv_obj_t* obj, lv_color_t color)
{
    lvx_btn_set_style_bg_color(obj, color);
}

void lvx_radio_set_state(lv_obj_t* obj, lv_state_t state)
{
    lvx_radio_t* radio = (lvx_radio_t*)obj;
    lv_obj_t** new = NULL;

    if (state != LV_STATE_DEFAULT) {
        _LV_LL_READ(&radio->group->obj_list, new)
        {
            if (*new == obj) {
                lv_obj_clear_state(*new, LV_STATE_DEFAULT);
                lv_obj_add_state(*new, LV_STATE_CHECKED);
            } else {
                lv_obj_clear_state(*new, LV_STATE_CHECKED);
                lv_obj_add_state(*new, LV_STATE_DEFAULT);
            }
        }
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_radio_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
}

static void lvx_radio_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    lvx_radio_t* radio = (lvx_radio_t*)obj;
    if (radio->group != NULL) {
        lvx_radio_group_remove_radio(radio->group, obj);
    }
    radio->group = NULL;
}

static void lvx_radio_event_cb(const lv_obj_class_t* class_p, lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_current_target(e);
    lv_obj_t** new = NULL;
    lvx_radio_t* radio = (lvx_radio_t*)obj;

    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK)
        return;
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED)
        return;

    lv_state_t state = lv_obj_get_state(obj);
    if (state != LV_STATE_DEFAULT) {
        if (radio->group == NULL) {
            if (lv_obj_has_flag(obj, LV_STATE_CHECKED)) {
                lv_obj_clear_state(obj, LV_STATE_CHECKED);
                lv_obj_add_state(obj, LV_STATE_DEFAULT);
            } else {
                lv_obj_clear_state(obj, LV_STATE_DEFAULT);
                lv_obj_add_state(obj, LV_STATE_CHECKED);
            }
        } else {
            _LV_LL_READ(&radio->group->obj_list, new)
            {
                if (*new == obj) {
                    lv_obj_clear_state(*new, LV_STATE_DEFAULT);
                    lv_obj_add_state(*new, LV_STATE_CHECKED);
                } else {
                    lv_obj_clear_state(*new, LV_STATE_CHECKED);
                    lv_obj_add_state(*new, LV_STATE_DEFAULT);
                }
            }
        }
    }
}

#endif
