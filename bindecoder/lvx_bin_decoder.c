/**
 * @file lvx_bin_decoder.c
 * Decode bin file directly to RAM.
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdlib.h>
#include "lvx_bin_decoder.h"

#if LVX_USE_BIN_DECODER

/*********************
 *      DEFINES
 *********************/

#define BIN_DECODER_PERF        0 /* output file load decoder performance */

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_img_decoder_dsc_t decoder_dsc;
    lv_img_dsc_t img_dsc;
    const void* file_cache;
} lvx_bin_decoder_data_t;


/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t decoder_info(struct _lv_img_decoder_t * decoder,
                             const void * src, lv_img_header_t * header);
static lv_res_t decoder_open(lv_img_decoder_t * dec,
                             lv_img_decoder_dsc_t * dsc);
static lv_res_t decoder_read_line(lv_img_decoder_t * decoder,
                                  lv_img_decoder_dsc_t * dsc, lv_coord_t x,
                                  lv_coord_t y, lv_coord_t len, uint8_t * buf);
static void decoder_close(lv_img_decoder_t * dec, lv_img_decoder_dsc_t * dsc);

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
    lv_img_decoder_t * dec = lv_img_decoder_create();
    lv_img_decoder_set_info_cb(dec, decoder_info);
    lv_img_decoder_set_open_cb(dec, decoder_open);
    lv_img_decoder_set_read_line_cb(dec, decoder_read_line);
    lv_img_decoder_set_close_cb(dec, decoder_close);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


static lv_res_t decoder_info(struct _lv_img_decoder_t * decoder,
                             const void * src, lv_img_header_t * header)
{
    (void) decoder; /*Unused*/
    lv_img_src_t src_type = lv_img_src_get_type(src);   /*Get the source type*/

    if (src_type == LV_IMG_SRC_FILE) {
        lv_fs_res_t res;
        uint32_t rn;
        lv_fs_file_t f;
        lv_img_header_t img_header = { 0 };
        /*Support only "*.bin" files*/
        if (strcmp(lv_fs_get_ext(src), "bin") != 0)
            return LV_RES_INV;

        res = lv_fs_open(&f, src, LV_FS_MODE_RD);
        if (res != LV_FS_RES_OK)
            return LV_RES_INV;

        res = lv_fs_read(&f, &img_header, sizeof(lv_img_header_t), &rn);
        lv_fs_close(&f);

        if ((res != LV_FS_RES_OK) || (rn != sizeof(lv_img_header_t))) {
            LV_LOG_WARN("Image get info get read file header");
            return LV_RES_INV;
        }

        *header = img_header;

        return LV_RES_OK;
     } else {
        const lv_img_dsc_t * img_dsc = src;

        header->always_zero = 0;
        header->cf = img_dsc->header.cf;
        header->w = img_dsc->header.w;
        header->h = img_dsc->header.h;

        return LV_RES_OK;
     }

     return LV_RES_INV;
}

static inline lv_res_t decode_from_file(lv_img_decoder_t * decoder,
                                        lv_img_decoder_dsc_t * dsc,
                                        lv_img_header_t * header,
                                        const void ** img_data)
{
    lv_fs_res_t res;
    lv_fs_file_t f;
    uint32_t rd_cnt;
    void * img_buf;
    uint32_t buf_len;
    const char * fn = dsc->src;

#if BIN_DECODER_PERF
    uint32_t start = lv_tick_get();
#endif

    res = lv_fs_open(&f, fn, LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK) {
        LV_LOG_WARN("BIN image decoder can't open the file");
        return LV_RES_INV;
    }

    res = lv_fs_read(&f, header, sizeof(lv_img_header_t), &rd_cnt);

    if ((res != LV_FS_RES_OK) || (rd_cnt != sizeof(lv_img_header_t))) {
        LV_LOG_WARN("BIN image decoder read header failed.");
        lv_fs_close(&f);
        return LV_RES_INV;
    }

    if (lv_fs_seek(&f, 0, LV_FS_SEEK_END) != 0) {
        lv_fs_close(&f);
        return LV_RES_INV;
    }

    lv_fs_tell(&f, &buf_len);
    buf_len -= sizeof(lv_img_header_t);

    if (lv_fs_seek(&f, sizeof(lv_img_header_t), LV_FS_SEEK_SET) != 0) {
        lv_fs_close(&f);
        return LV_RES_INV;
    }

    if (buf_len == 0) {
        LV_LOG_WARN("Invalid bin file");
        lv_fs_close(&f);
        return LV_RES_INV;
    }

    img_buf = lv_mem_alloc(buf_len);
    if (img_buf == NULL) {
        lv_fs_close(&f);
        return LV_RES_INV;
    }

    /**
     * Read and decompress data.
     */

    uint32_t rd;
    res = lv_fs_read(&f, img_buf, buf_len, &rd);
    lv_fs_close(&f);

    if (res != LV_FS_RES_OK || rd != buf_len) {
        lv_mem_free(img_buf);
        return LV_RES_INV;
    }

#if BIN_DECODER_PERF
    uint32_t elaps = lv_tick_elaps(start);
    float speed = buf_len/1024.f;
    speed /= elaps;
    speed *= 1000;
    printf("bin-load: %"PRIu32"ms, file size: %"PRIu32"kB, speed: %fkB/s\n",
            elaps, buf_len/1024, speed);
#endif

    *img_data = img_buf;
    return LV_RES_OK;
}

static inline lv_res_t decode_from_variable(lv_img_decoder_t * decoder,
                                        lv_img_decoder_dsc_t * dsc,
                                        lv_img_header_t * header,
                                        void const * * img_data)
{
    const lv_img_dsc_t * img_dsc = dsc->src;

    *img_data = img_dsc->data;
    return LV_RES_OK;
}

static lv_res_t decoder_open(lv_img_decoder_t * decoder,
                             lv_img_decoder_dsc_t * dsc)
{
    lv_fs_res_t res;
    lv_img_header_t imgheader;
    lvx_bin_decoder_data_t * data;
    const void* img_data;
    bool from_file = false;
    if (dsc->src_type == LV_IMG_SRC_FILE) {
        res = decode_from_file(decoder, dsc, &imgheader, &img_data);
        if (res != LV_RES_OK)
            return res;
        from_file = true;
    }

    else if (dsc->src_type == LV_IMG_SRC_VARIABLE) {
        res = decode_from_variable(decoder, dsc, &imgheader, &img_data);

        if (res != LV_RES_OK)
            return res;
    }
    else {
        return LV_RES_INV;
    }

    /*Decoder private data.*/
    data = lv_mem_alloc(sizeof(*data));
    if (data == NULL) {
        return LV_RES_INV;
    }
    lv_memset_00(data, sizeof(*data));

    if (from_file) {
        data->file_cache = img_data;
    }
    lv_memcpy(&data->decoder_dsc, dsc, sizeof(*dsc));

    data->img_dsc.header = imgheader;
    data->img_dsc.data = img_data;
    data->img_dsc.data_size = 0;

    data->decoder_dsc.img_data = NULL;
    data->decoder_dsc.src_type = LV_IMG_SRC_VARIABLE;
    data->decoder_dsc.src = &data->img_dsc;
    data->decoder_dsc.user_data = NULL;

    res = lv_img_decoder_built_in_open(decoder, &data->decoder_dsc);
    if (res != LV_RES_OK) {
        lv_mem_free((void*)img_data);
        lv_mem_free(data);
        return LV_RES_INV;
    }

    dsc->img_data = data->decoder_dsc.img_data;
    dsc->user_data = data;
    return LV_RES_OK;
}

static lv_res_t decoder_read_line(lv_img_decoder_t * decoder,
                                  lv_img_decoder_dsc_t * dsc, lv_coord_t x,
                                  lv_coord_t y, lv_coord_t len, uint8_t * buf)
{
    lvx_bin_decoder_data_t* data = dsc->user_data;
    if (data == NULL)
        return LV_RES_INV;

    return lv_img_decoder_built_in_read_line(decoder, &data->decoder_dsc, x, y,
                                             len, buf);
}

static void decoder_close(lv_img_decoder_t *decoder, lv_img_decoder_dsc_t *dsc)
{
    if (!dsc->user_data)
        return;

    lvx_bin_decoder_data_t* decoder_data = dsc->user_data;
    lv_img_decoder_built_in_close(decoder, &decoder_data->decoder_dsc);
    if (decoder_data->file_cache)
        lv_mem_free((uint8_t *)decoder_data->file_cache);
    lv_mem_free(dsc->user_data);
    dsc->user_data = NULL;
}

#endif /*LVX_USE_BIN_DECODER*/
