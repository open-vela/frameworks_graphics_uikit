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
#include "lvx_draw_demos.h"
#include "lvgl.h"

#if LV_USE_VECTOR_GRAPHIC
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

/*********************
 *      DEFINES
 *********************/
#define MAX_FLOWERS 200

#define PI 3.14159265358979323846f

#define CLOVER 0

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    int used;
    float x, y;
    float dx, dy;
    float rot;
    float rotf;
    float scl;
    float cr, cg, cb, ca;
} particle;

typedef struct {
    lv_obj_t obj;
    particle flowers[MAX_FLOWERS];
    lv_vector_path_t * path;
    lv_timer_t * anim_timer;
    int check;
} lv_hearts_t;

#define MY_CLASS &lv_hearts_class

static void lv_hearts_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_hearts_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_hearts_event(const lv_obj_class_t * class_p, lv_event_t * e);
static void lv_hearts_draw(lv_obj_t * obj, lv_event_t * e);

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_obj_t * lv_hearts_create(lv_obj_t * parent);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

static void draw_animation(lv_obj_t * obj, lv_layer_t * layer)
{
    int32_t w = lv_obj_get_width(obj);
    int32_t h = lv_obj_get_height(obj);
    lv_hearts_t * heart = (lv_hearts_t *)obj;
    lv_vector_dsc_t * ctx = lv_vector_dsc_create(layer);

    lv_area_t rect = {0, 0, w, h};
    lv_vector_dsc_set_fill_color(ctx, lv_color_black());
    lv_vector_clear_area(ctx, &rect); // clear screen

    lv_color_t fcolor;

    lv_vector_dsc_set_blend_mode(ctx, LV_VECTOR_BLEND_ADDITIVE);

    for(int i = 0; i < MAX_FLOWERS; i++) {
        if(!heart->flowers[i].used)
            continue;
        heart->flowers[i].rot += heart->flowers[i].rotf;
        heart->flowers[i].x += heart->flowers[i].dx;
        heart->flowers[i].y -= heart->flowers[i].dy;

        lv_vector_dsc_identity(ctx);

        lv_vector_dsc_translate(ctx, heart->flowers[i].x, heart->flowers[i].y);
        lv_vector_dsc_rotate(ctx, heart->flowers[i].rot);
        lv_vector_dsc_scale(ctx, heart->flowers[i].scl, heart->flowers[i].scl);

        float fr = heart->flowers[i].cr;
        float fg = heart->flowers[i].cg;
        float fb = heart->flowers[i].cb;
        float fa = heart->flowers[i].ca;

        fcolor = lv_color_make((uint8_t)(fr * 255.0f), (uint8_t)(fg * 255.0f), (uint8_t)(fb * 255.0f));
        lv_vector_dsc_set_fill_color(ctx, fcolor);
        lv_vector_dsc_set_fill_opa(ctx, (uint8_t)(fa * 255.0f));
        lv_vector_dsc_add_path(ctx, heart->path);
    }

    lv_draw_vector(ctx); // submit draw
    lv_vector_dsc_delete(ctx);
}

static void create_particles(lv_hearts_t * heart)
{
    srand((unsigned)time(NULL));

    /*init flowers*/
    for(int i = 0; i < MAX_FLOWERS; i++) {
        heart->flowers[i].used = 0;
        heart->flowers[i].x = 0;
        heart->flowers[i].y = 0;
        heart->flowers[i].dx = 0;
        heart->flowers[i].dx = 0;
        heart->flowers[i].rot = 0;
        heart->flowers[i].rotf = 0;
        heart->flowers[i].scl = 0;
        heart->flowers[i].cr = 0;
        heart->flowers[i].cg = 0;
        heart->flowers[i].cb = 0;
        heart->flowers[i].ca = 0;
    }

    /*init path*/
#if CLOVER
    lvx_vector_shapes_factory(LVX_VECTOR_SHAPE_CLOVER, heart->path);
#else
    lvx_vector_shapes_factory(LVX_VECTOR_SHAPE_HEART, heart->path);
#endif
}

static void lv_heart_event(lv_obj_t * obj, int32_t x, int32_t y)
{
    lv_hearts_t * heart = (lv_hearts_t *)obj;
    for(int i = 0; i < MAX_FLOWERS; i++) {
        if(!heart->flowers[i].used) {
            heart->flowers[i].x = (float)x;
            heart->flowers[i].y = (float)y;
            heart->flowers[i].dx = 1.5f * ((float)rand() / RAND_MAX) - 1.6f;
            heart->flowers[i].dy = 1.5f * ((float)rand() / RAND_MAX) - 1.6f;
            heart->flowers[i].scl = 0.3f * ((float)rand() / RAND_MAX);
            heart->flowers[i].rot = ((float)rand() / RAND_MAX) * 180.0f;
            heart->flowers[i].rotf = (0.2f * ((float)rand() / RAND_MAX) - 0.1f) * 45.0f;
            heart->flowers[i].cr = (float)rand() / RAND_MAX + 0.1f;
            heart->flowers[i].cg = (float)rand() / RAND_MAX + 0.1f;
            heart->flowers[i].cb = (float)rand() / RAND_MAX + 0.1f;
            heart->flowers[i].ca = (float)rand() / RAND_MAX + 0.3f;
            heart->flowers[i].used = 1;
            break;
        }
    }
}

static void widget_draw(lv_hearts_t * heart)
{
    lv_obj_t * obj = (lv_obj_t *)heart;
    lv_obj_invalidate(obj);
}

static void anim_timer_cb(lv_timer_t * param)
{
    lv_hearts_t * heart = (lv_hearts_t *)param->user_data;
    heart->check++;
    if(heart->check > 100) {
        for(int i = 0; i < MAX_FLOWERS; i++) {
            if(heart->flowers[i].x < -100 * heart->flowers[i].scl)
                heart->flowers[i].used = 0;
        }
    }
    widget_draw(heart);
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lvx_draw_demo_hearts(char * info[], int size, void * param)
{
    lv_obj_t * hearts_widget = lv_hearts_create(lv_scr_act());
    lv_obj_set_size(hearts_widget, LV_PCT(100), LV_PCT(100));
}

const lv_obj_class_t lv_hearts_class  = {
    .constructor_cb = lv_hearts_constructor,
    .destructor_cb = lv_hearts_destructor,
    .event_cb = lv_hearts_event,
    .instance_size = sizeof(lv_hearts_t),
    .base_class = &lv_obj_class,
    .name = "hearts",
};

static lv_obj_t * lv_hearts_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

static void lv_hearts_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_hearts_t * heart = (lv_hearts_t *)obj;
    heart->path = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM);
    create_particles(heart);
    heart->anim_timer = lv_timer_create(anim_timer_cb, 16, heart);
}

static void lv_hearts_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_hearts_t * heart = (lv_hearts_t *)obj;
    lv_timer_delete(heart->anim_timer);
    lv_vector_path_delete(heart->path);
}

static void lv_hearts_draw(lv_obj_t * obj, lv_event_t * e)
{
    lv_layer_t * layer = lv_event_get_layer(e);
    draw_animation(obj, layer);
}

static void lv_hearts_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);

    lv_result_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);
    if(code == LV_EVENT_DRAW_MAIN) {
        lv_hearts_draw(obj, e);
    }
    else if(code == LV_EVENT_CLICKED) {
        lv_point_t point;
        lv_indev_get_point(lv_indev_active(), &point);
        lv_heart_event(obj, point.x, point.y);
    }
    else if(code == LV_EVENT_PRESSING) {
        lv_point_t point;
        lv_indev_get_point(lv_indev_active(), &point);
        lv_heart_event(obj, point.x, point.y);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
#else

void lvx_draw_demo_hearts(char * info[], int size, void * param)
{
    /*fallback for online examples*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Vector graphics is not enabled");
    lv_obj_center(label);
}

#endif
