#ifndef LV_EXTENSIONS_WIDGETS_ROLLER_H_
#define LV_EXTENSIONS_WIDGETS_ROLLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvx_widgets.h"

#if LVX_USE_PICKER != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
enum {
    LVX_ROLLER_MODE_NORMAL = LV_ROLLER_MODE_NORMAL,
    LVX_ROLLER_MODE_INFINITE = LV_ROLLER_MODE_INFINITE,
};

typedef uint8_t lvx_picker_mode_t;

typedef struct {
    lv_obj_t obj;
    lv_obj_t* rec;
    lv_obj_t** colume_array;
    lv_coord_t colume_cnt;
} lvx_picker_t;

extern const lv_obj_class_t lvx_picker_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a roller objects, which can contain more than one column
 * @param parent    pointer to an object, it will be the parent of the new roller.
 * @return          pointer to the created roller
 */
lv_obj_t* lvx_picker_create(lv_obj_t* parent);

/*=====================
 * Setter functions
 *====================*/
/**
 * Add the options on a roller
 * @param obj       pointer to lvx_picker object
 * @param options   a string with '\n' separated options. E.g. "One\nTwo\nThree"
 * @param mode      `LV_ROLLER_MODE_NORMAL` or `LV_ROLLER_MODE_INFINITE`
 */
void lvx_picker_add_options(lv_obj_t* obj, const char* options,
                            lv_roller_mode_t mode);

/**
 * Set the options on a roller
 * @param obj             pointer to roller object
 * @param options_array   a string array with '\n' separated options.
 * @param mode            `LV_ROLLER_MODE_NORMAL` or `LV_ROLLER_MODE_INFINITE`
 */
void lvx_picker_set_options(lv_obj_t* obj, const char** options_array,
                            lv_coord_t col_cnt, lv_roller_mode_t mode);
/**
 * Set the height to show the given number of rows (options)
 * @param obj       pointer to a roller object
 * @param index     which column number to set
 * @param row_cnt   number of desired visible rows
 */
void lvx_picker_set_column_visible_row_count(lv_obj_t* obj, uint8_t index,
                                             uint8_t row_cnt);

/**
 * Set the height to show the given number of rows (options)
 * @param obj       pointer to a roller object
 * @param row_cnt   number of desired visible rows
 */
void lvx_picker_set_visible_row_count(lv_obj_t * obj, uint8_t row_cnt);

/**
 * Set the height to show the given number of rows (options)
 * @param obj          pointer to a roller object
 * @param space_text   set the text between columns
 */
void lvx_picker_set_space_text(lv_obj_t * obj, const char* space_text);

/*=====================
 * Getter functions
 *====================*/
uint32_t lvx_picker_get_select_str(lv_obj_t* obj, char* buf, uint32_t len);

/**********************
 *      MACROS
 **********************/

#endif /* LVX_USE_PICKER */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_EXTENSIONS_WIDGETS_ROLLER_H_ */
