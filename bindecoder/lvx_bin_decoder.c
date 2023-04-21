/**
 * @file lvx_bin_decoder.c
 * Decode bin file directly to RAM.
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_bin_decoder.h"
#include <stdlib.h>

#if LVX_USE_BIN_DECODER

/*********************
 *      DEFINES
 *********************/

#define BIN_DECODER_PERF 0 /* output file load decoder performance */

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_img_dsc_t* img;
} lvx_bin_decoder_data_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t decoder_info(struct _lv_img_decoder_t* decoder, const void* src,
                             lv_img_header_t* header);
static lv_res_t decoder_open(lv_img_decoder_t* dec, lv_img_decoder_dsc_t* dsc);
static void decoder_close(lv_img_decoder_t* dec, lv_img_decoder_dsc_t* dsc);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lvx_bin_decoder_init(void)
{
    lv_img_decoder_t* dec = lv_img_decoder_create();
    lv_img_decoder_set_info_cb(dec, decoder_info);
    lv_img_decoder_set_open_cb(dec, decoder_open);
    lv_img_decoder_set_close_cb(dec, decoder_close);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_img_dsc_t* decode_image(lv_img_decoder_t* decoder, lv_img_decoder_dsc_t* dsc)
{
    lv_res_t res;
    lv_img_header_t header = dsc->header;

    lv_img_dsc_t* img_dsc
        = lv_img_buf_alloc(header.w, header.h, LV_IMG_CF_TRUE_COLOR_ALPHA);

    if (!img_dsc) {
        return NULL;
    }

    if (dsc->img_data) {
        /* The file has been read in decoder_open. */
        memcpy((void*)img_dsc->data, dsc->img_data, img_dsc->data_size);
    } else {
        /* Read file line by line. */
        uint32_t px_size = lv_img_cf_get_px_size(LV_IMG_CF_TRUE_COLOR_ALPHA);
        uint32_t stride = header.w * (px_size >> 3);
        for (lv_coord_t row = 0; row < header.h; row++) {
            res = lv_img_decoder_built_in_read_line(
                decoder, dsc, 0, row, header.w,
                (uint8_t*)(img_dsc->data + row * stride));
            if (res != LV_RES_OK) {
                lv_img_buf_free(img_dsc);
                return NULL;
            }
        }
    }

    return img_dsc;
}

static lv_res_t decoder_info(lv_img_decoder_t* decoder, const void* src,
                             lv_img_header_t* header)
{
    lv_res_t res = lv_img_decoder_built_in_info(decoder, src, header);
    if (res != LV_RES_OK)
        return res;

    lv_img_src_t src_type = lv_img_src_get_type(src);
    if(src_type == LV_IMG_SRC_VARIABLE){
        if (header->cf == LV_IMG_CF_TRUE_COLOR ||
            header->cf == LV_IMG_CF_TRUE_COLOR_ALPHA)
            return LV_RES_INV;
    }

    /* we insist this is a truecolor-alpha image. */
    header->cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    return LV_RES_OK;
}

static lv_res_t decoder_open(lv_img_decoder_t* decoder,
                             lv_img_decoder_dsc_t* dsc)
{
    lvx_bin_decoder_data_t* data;
    lv_img_header_t header;

    lv_res_t res = lv_img_decoder_built_in_info(decoder, dsc->src, &header);
    if (res != LV_RES_OK)
        return res;

    /* use the built-in decoder to read image data, need to set correct cf */
    dsc->header.cf = header.cf;

    res = lv_img_decoder_built_in_open(decoder, dsc);
    if (res != LV_RES_OK) {
        return LV_RES_INV;
    }

    lv_img_dsc_t* img = decode_image(decoder, dsc);
    lv_img_decoder_built_in_close(decoder, dsc);
    if (img == NULL) {
        return LV_RES_INV;
    }

    /* change back to truecolor-alpha format once we got img map */
    dsc->header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;

    /*Decoder private data.*/
    data = lv_mem_alloc(sizeof(*data));
    if (data == NULL) {
        return LV_RES_INV;
    }
    lv_memset_00(data, sizeof(*data));

    data->img = img;
    dsc->img_data = img->data;
    dsc->user_data = data;
    return LV_RES_OK;
}

static void decoder_close(lv_img_decoder_t* decoder, lv_img_decoder_dsc_t* dsc)
{
    if (!dsc->user_data)
        return;

    lvx_bin_decoder_data_t* data = dsc->user_data;
    if (data->img)
        lv_img_buf_free(data->img);

    lv_mem_free(dsc->user_data);
    dsc->user_data = NULL;
}

#endif /*LVX_USE_BIN_DECODER*/
