/**
 * @file hearts.c
 *
 */

/*
 * Conversion from the original AmanithVG's example, by Mazatech
 * Srl - http://www.amanithvg.com
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "lvx_draw_demos.h"

#if LV_USE_VECTOR_GRAPHIC
#include "sys/time.h"
#include "time.h"
#include "unistd.h"
#include <math.h>

/*********************
 *      DEFINES
 *********************/

#define PI 3.14159265358979323846f

#define CLOVER 0

/**********************
 *      TYPEDEFS
 **********************/

#define MAX_CIRCLE_NUM 4

typedef struct {
    float angle;
    float r;
} lv_anim_circle_t;

typedef struct {
    lv_area_t rect;
} lv_anim_light_t;

typedef struct {
    lv_obj_t obj;
    lv_anim_circle_t layer1; //
    lv_anim_circle_t layer2; //
    lv_anim_circle_t layer3; //
    lv_anim_circle_t layer4; //
    lv_vector_path_t* circle_path;
    lv_timer_t* anim_timer;
    lv_color_t color;
    float move_r; // center point from 0 to move_r
    float move_r2;

    float anim_start_r;
    float anim_start_r2;

    float anim_cur_r;
    float anim_cur_r2;

    lv_anim_light_t light[5];
    lv_vector_path_t* light_path;
    lv_color_t light_color;

} lv_breathe_t;

#define MY_CLASS &lv_breathe_class

static void lv_breathe_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lv_breathe_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lv_breathe_event(const lv_obj_class_t* class_p, lv_event_t* e);
static void lv_breathe_draw(lv_obj_t* obj, lv_event_t* e);

const lv_obj_class_t lv_breathe_class = {
    .constructor_cb = lv_breathe_constructor,
    .destructor_cb = lv_breathe_destructor,
    .event_cb = lv_breathe_event,
    .instance_size = sizeof(lv_breathe_t),
    .base_class = &lv_obj_class,
    .name = "breathe",
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

static lv_obj_t* lv_breathe_create(lv_obj_t* parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lvx_draw_demo_breathe(char* info[], int size, void* param)
{
    lv_obj_t* breathe_widget = lv_breathe_create(lv_scr_act());
    lv_obj_set_size(breathe_widget, (466), (466));
}

static void create_circle(lv_breathe_t* heart)
{
    srand((unsigned)time(NULL));

    heart->layer1.angle = 0;
    heart->layer1.r = 108.0f / 2;

    heart->layer2.angle = 0;
    heart->layer2.r = 108.0f / 2;

    heart->layer3.angle = 45.0f;
    heart->layer3.r = 78.0f / 2;

    heart->layer4.angle = 0;
    heart->layer4.r = 155.0f / 2;

    /*init path*/
}

static void widget_draw(lv_breathe_t* heart)
{
    lv_obj_t* obj = (lv_obj_t*)heart;
    lv_obj_invalidate(obj);
}

static void anim_timer_cb(lv_timer_t* param)
{
    lv_breathe_t* heart = (lv_breathe_t*)param->user_data;
    widget_draw(heart);
}

static void lv_breathe_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    lv_breathe_t* heart = (lv_breathe_t*)obj;
    heart->circle_path = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM);
    heart->light_path = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM);

    heart->color = lv_color_make(0x00, 0xF5, 0xFF);
    heart->light_color = lv_color_make(0x00, 0xF5, 0xFF);
    heart->move_r = 155.0f / 2.0f + 107.0f / 2.0f - (155 + 107 - 238);
    heart->move_r2 = 155.0f / 2.0f + 78.0f / 2.0f - (155 + 78 - 218);

    heart->anim_start_r = 0;
    heart->anim_start_r2 = 0;

    heart->anim_cur_r = 0;
    heart->anim_cur_r2 = 0;

    create_circle(heart);
    heart->anim_timer = lv_timer_create(anim_timer_cb, 16, heart);
}

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

static void lvx_draw_circle(lv_obj_t* obj, lv_vector_dsc_t* ctx, float radius, float pre_step, lv_vector_path_t* circle_path, lv_anim_circle_t* circle, lv_color_t color)
{
    int32_t w = lv_obj_get_width(obj);
    int32_t h = lv_obj_get_height(obj);

    lv_fpoint_t lv_center;
    for (int i = 0; i < 4; i++) {

        float angle = circle->angle + 90 * i + pre_step * 360.0f;

        while (angle > 360.0f) {
            angle -= 360;
        }

        angle = angle / 360.0f * 2 * PI;

        lv_center.x = radius * cos(angle);
        lv_center.y = radius * sin(angle);

        lv_vector_dsc_identity(ctx);
        lv_vector_dsc_translate(ctx, w / 2, h / 2);

        lv_vector_dsc_set_fill_color(ctx, color);
        lv_vector_dsc_set_fill_opa(ctx, (uint8_t)(0.3 * 255.0f));
        lv_vector_path_append_circle(circle_path, &lv_center, circle->r, circle->r);
        lv_vector_dsc_add_path(ctx, circle_path);
        lv_vector_path_clear(circle_path);
    }
}

static void draw_animation(lv_obj_t* obj, lv_layer_t* layer)
{

    int32_t w = lv_obj_get_width(obj);
    int32_t h = lv_obj_get_height(obj);
    lv_breathe_t* heart = (lv_breathe_t*)obj;
    lv_vector_dsc_t* ctx = lv_vector_dsc_create(layer);

    lv_area_t rect = { 0, 0, w, h };
    lv_vector_dsc_set_fill_color(ctx, lv_color_black());
    lv_vector_clear_area(ctx, &rect); // clear screen

    lv_color_t fcolor;

    // lv_vector_dsc_set_blend_mode(ctx, LV_VECTOR_BLEND_ADDITIVE);
    static int g_angle_index = 0;

    ++g_angle_index;

    static bool revert = false;
    if (g_angle_index > 240) {

        g_angle_index = g_angle_index % 240;
        revert = !revert;
    }

    float pre_step = g_angle_index / 240.0f;

    if (revert) {
        heart->anim_cur_r = heart->move_r - (heart->move_r - heart->anim_start_r) * pre_step;
        heart->anim_cur_r2 = heart->move_r2 - (heart->move_r2 - heart->anim_start_r2) * pre_step;
    } else {
        heart->anim_cur_r = (heart->move_r - heart->anim_start_r) * pre_step;
        heart->anim_cur_r2 = (heart->move_r2 - heart->anim_start_r2) * pre_step;
    }

    // draw light
    heart->light[0].rect.x1 = 0;
    heart->light[0].rect.y1 = 0;
    heart->light[0].rect.y2 = 0;
    heart->light[0].rect.y2 = 0;

    // draw circle
    lvx_draw_circle(obj, ctx, heart->anim_cur_r, pre_step, heart->circle_path, &(heart->layer1), heart->color);
    lvx_draw_circle(obj, ctx, heart->anim_cur_r, pre_step, heart->circle_path, &(heart->layer2), heart->color);
    lvx_draw_circle(obj, ctx, heart->anim_cur_r2, pre_step, heart->circle_path, &(heart->layer3), heart->color);

    lv_fpoint_t lv_center;
    lv_center.x = 0;
    lv_center.y = 0;

    lv_vector_dsc_identity(ctx);
    lv_vector_dsc_translate(ctx, w / 2, h / 2);
    lv_vector_dsc_set_fill_color(ctx, heart->color);
    lv_vector_dsc_set_fill_opa(ctx, (uint8_t)(0.5 * 255.0f));

    lv_vector_path_append_circle(heart->circle_path, &lv_center, heart->layer4.r, heart->layer4.r);

    lv_vector_dsc_add_path(ctx, heart->circle_path);
    lv_vector_path_clear(heart->circle_path);

    lv_draw_vector(ctx); // submit draw
    lv_vector_dsc_delete(ctx);

    float radius = 0;
    for (int i = 0; i < MAX_CIRCLE_NUM - 1; i++) {

        float angle = (heart->circle[i].angle + pre_step * 360.0f);

        while (angle > 360.0f) {
            angle -= 360;
        }

        angle = angle / 360.0f * 2 * PI;

        if (i / 4 == 2) {
            radius = heart->anim_cur_r2;
        } else {
            radius = heart->anim_cur_r;
        }
        lv_center.x = radius * cos(angle);
        lv_center.y = radius * sin(angle);

        lv_vector_dsc_identity(ctx);
        lv_vector_dsc_translate(ctx, w / 2, h / 2);

        lv_vector_dsc_set_fill_color(ctx, heart->color);
        lv_vector_dsc_set_fill_opa(ctx, (uint8_t)(0.3 * 255.0f));

        lv_vector_path_append_circle(heart->circle_path, &lv_center, heart->circle[i].r, heart->circle[i].r);

        lv_vector_dsc_add_path(ctx, heart->circle_path);
        lv_vector_path_clear(heart->circle_path);
    }

    lv_center.x = 0;
    lv_center.y = 0;

    lv_vector_dsc_identity(ctx);
    lv_vector_dsc_translate(ctx, w / 2, h / 2);
    lv_vector_dsc_set_fill_color(ctx, heart->color);
    lv_vector_dsc_set_fill_opa(ctx, (uint8_t)(0.5 * 255.0f));

    lv_vector_path_append_circle(heart->circle_path, &lv_center, heart->circle[12].r, heart->circle[12].r);

    lv_vector_dsc_add_path(ctx, heart->circle_path);
    lv_vector_path_clear(heart->circle_path);

    lv_draw_vector(ctx); // submit draw
    lv_vector_dsc_delete(ctx);
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static void lv_breathe_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    lv_breathe_t* heart = (lv_breathe_t*)obj;
    lv_timer_delete(heart->anim_timer);
    lv_vector_path_delete(heart->circle_path);
}

static void lv_breathe_draw(lv_obj_t* obj, lv_event_t* e)
{
    lv_layer_t* layer = lv_event_get_layer(e);
    draw_animation(obj, layer);
}

static void lv_breathe_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
    LV_UNUSED(class_p);

    lv_result_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RESULT_OK)
        return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_current_target(e);
    if (code == LV_EVENT_DRAW_MAIN) {
        lv_breathe_draw(obj, e);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
#else

void lvx_draw_demo_breathe(char* info[], int size, void* param)
{
    /*fallback for online examples*/
    lv_obj_t* label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Vector graphics is not enabled");
    lv_obj_center(label);
}

#endif
