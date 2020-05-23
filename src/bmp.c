#include "bmp.h"
#include "ff.h"

typedef struct
{
    uint8_t b;
    uint8_t m;
    uint32_t bitmap_file_size;
    uint32_t reserved;
    uint32_t bitmap_data_offset;
} __attribute__((packed)) BMP_FileHeader_t;

typedef struct
{
    uint32_t header_size;
    uint16_t bitmap_width;
    uint16_t bitmap_height;
    uint16_t colour_planes;
    uint16_t bits_per_pixel;
} __attribute((packed)) BMP_CoreHeader_t;

void BMP_Save(uint8_t *data, int width, int height)
{
    FATFS fs;
    FIL fp;
    FRESULT rc;
    unsigned int bw;

    rc = f_mount(&fs, "", 0);
    if(rc)
    {
        return;
    }

    rc = f_open(&fp, "IMAGE.BMP", FA_WRITE | FA_CREATE_ALWAYS);
    if(rc)
    {
        return;
    }

    // Rows must be padded to a multiple if 4 bytes
    int row_size = width / 8;
    int padding_length = (4 - (row_size % 4)) % 4;
    uint8_t padding_bytes[4] = {0};
    row_size += padding_length;

    BMP_FileHeader_t file_header =
    {
        .b = 'B',
        .m = 'M',
        .bitmap_file_size = 14 + 12 + row_size * height,
        .bitmap_data_offset = 14 + 12 + 6
    };

    // Use the simplest possible header
    BMP_CoreHeader_t core_header =
    {
        .header_size = 12,
        .bitmap_width = width,
        .bitmap_height = height,
        .colour_planes = 1,
        .bits_per_pixel = 1
    };

    // At one bit per pixel, we need a colour table
    uint8_t colour_table[6] =
    {
        0xff, 0xff, 0xff,
        0x00, 0x00, 0x00
    };

    if(f_write(&fp, &file_header, sizeof(file_header), &bw) != FR_OK)
        return;
    if(f_write(&fp, &core_header, sizeof(core_header), &bw) != FR_OK)
        return;
    if(f_write(&fp, &colour_table, sizeof(colour_table), &bw) != FR_OK)
        return;

    // BMPs are stored with their rows from the bottom upwards
    for(int y = height - 1; y >= 0; y--)
    {
        uint8_t *row = data + y * (width / 8);
        if(f_write(&fp, row, width / 8, &bw) != FR_OK)
            return;
        if(padding_length != 0)
        {
            if(f_write(&fp, padding_bytes, padding_length, &bw) != FR_OK)
                return;
        }
    }

    f_close(&fp);
}
