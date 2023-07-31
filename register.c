#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

typedef struct rgb_pixel {
    uint8_t R;
    uint8_t G;
    uint8_t B;
} rgb_pixel;

typedef struct ycc_pixel {
    uint8_t Y;
    uint8_t Cb;
    uint8_t Cr;
} ycc_pixel;

typedef struct sample {
    uint8_t Y1;
    uint8_t Y2;
    uint8_t Y3;
    uint8_t Y4;
    uint8_t Cb;
    uint8_t Cr;
} sample;

typedef struct rgb_data {
    rgb_pixel* data;
} rgb_data;

typedef struct ycc_data {
    ycc_pixel* data;
} ycc_data;

typedef struct sampling_data {
    sample* data;
} sampling_data;

/*
Purpose: Convert an array of RGB pixels to an array of YCC pixels.
Parameters:
    - input: A pointer to an rgb_data structure containing the RGB pixel array.
    - height: The height of the image.
    - width: The width of the image.
Returns: A pointer to a ycc_data structure containing the converted YCC pixel array.
*/
ycc_data* rgb_to_ycc(rgb_data* input, int height, int width){
    int image_size = height * width;

    register ycc_data* to_ycc;
    to_ycc = malloc(sizeof(ycc_data));
    to_ycc->data = malloc(sizeof(ycc_pixel) * image_size);

    register int i;
    for (i = 0; i < image_size; i += 4) {
        to_ycc->data[i].Y  = 16 + ((16763 * input->data[i].R + 32909 * input->data[i].G + 6391 * input->data[i].B) >> 16);
        to_ycc->data[i].Cb = 128 + ((-9676 * input->data[i].R + -18996 * input->data[i].G + 28672 * input->data[i].B) >> 16);
        to_ycc->data[i].Cr = 128 + ((28672 * input->data[i].R + -24009 * input->data[i].G + -4662 * input->data[i].B) >> 16);

        to_ycc->data[i + 1].Y  = 16 + ((16763 * input->data[i + 1].R + 32909 * input->data[i + 1].G + 6391 * input->data[i + 1].B) >> 16);
        to_ycc->data[i + 1].Cb = 128 + ((-9676 * input->data[i + 1].R + -18996 * input->data[i + 1].G + 28672 * input->data[i + 1].B) >> 16);
        to_ycc->data[i + 1].Cr = 128 + ((28672 * input->data[i + 1].R + -24009 * input->data[i + 1].G + -4662 * input->data[i + 1].B) >> 16);

        to_ycc->data[i + 2].Y  = 16 + ((16763 * input->data[i + 2].R + 32909 * input->data[i + 2].G + 6391 * input->data[i + 2].B) >> 16);
        to_ycc->data[i + 2].Cb = 128 + ((-9676 * input->data[i + 2].R + -18996 * input->data[i + 2].G + 28672 * input->data[i + 2].B) >> 16);
        to_ycc->data[i + 2].Cr = 128 + ((28672 * input->data[i + 2].R + -24009 * input->data[i + 2].G + -4662 * input->data[i + 2].B) >> 16);

        to_ycc->data[i + 3].Y  = 16 + ((16763 * input->data[i + 3].R + 32909 * input->data[i + 3].G + 6391 * input->data[i + 3].B) >> 16);
        to_ycc->data[i + 3].Cb = 128 + ((-9676 * input->data[i + 3].R + -18996 * input->data[i + 3].G + 28672 * input->data[i + 3].B) >> 16);
        to_ycc->data[i + 3].Cr = 128 + ((28672 * input->data[i + 3].R + -24009 * input->data[i + 3].G + -4662 * input->data[i + 3].B) >> 16);
    }
    
    return to_ycc;
}

/*
Purpose: Convert an array of YCC pixels to an array of YCC meta pixels.
Parameters:
    - input: A pointer to a ycc_data structure containing the YCC pixel array.
    - height: The height of the image.
    - width: The width of the image.
Returns: A pointer to a sampling_data structure containing the converted YCC meta pixel array.
*/
sampling_data* downsample_ycc(ycc_data* input, int height, int width) {
    int image_size = height * width;

    register sampling_data* to_downsample;
    to_downsample = malloc(sizeof(sampling_data));
    to_downsample->data = malloc(sizeof(sample) * (image_size >> 2));

    register int i, j, offset, outline;
    for (i = (height >> 1) - 1; i != -1; i--) {
        offset = i * width >> 1;
        for (j = (width >> 1) - 1; j != -1; j--) {
            outline = i * 2 * width + j * 2;
            to_downsample->data[offset + j].Y1 = input->data[outline].Y;
            to_downsample->data[offset + j].Y2 = input->data[outline + 1].Y;
            to_downsample->data[offset + j].Y3 = input->data[outline + width].Y;
            to_downsample->data[offset + j].Y4 = input->data[outline + 1 + width].Y;
            to_downsample->data[offset + j].Cb = (input->data[outline].Cb + input->data[outline + 1].Cb + input->data[outline + width].Cb + input->data[outline + 1 + width].Cb) >> 2;
            to_downsample->data[offset + j].Cr = (input->data[outline].Cr + input->data[outline + 1].Cr + input->data[outline + width].Cr + input->data[outline + 1 + width].Cr) >> 2;
        }
    }
    return to_downsample;
}

/*
Purpose: Convert an array of YCC meta pixels to an array of YCC pixels.
Parameters:
    - input: A pointer to a sampling_data structure containing the YCC meta pixel array.
    - height: The height of the image.
    - width: The width of the image.
Returns: A pointer to a ycc_data structure containing the converted YCC pixel array.
*/
ycc_data* upsample_ycc(sampling_data* input, int height, int width) {
    int image_size = height * width;

    register ycc_data* to_upsample;
    to_upsample = malloc(sizeof(ycc_data));
    to_upsample->data = malloc(sizeof(ycc_pixel) * image_size);
    
    register int i, j, offset, outline;
    for (i = (height >> 1) - 1; i != -1; i--) {
        offset = i * (width >> 1);
        for (j = (width >> 1) - 1; j != -1; j--) {
            outline = i * 2 * width + j * 2;
            to_upsample->data[outline].Y = input->data[offset + j].Y1;
            to_upsample->data[outline + 1].Y = input->data[offset + j].Y2;
            to_upsample->data[outline + width].Y = input->data[offset + j].Y3;
            to_upsample->data[outline + 1 + width].Y = input->data[offset + j].Y4;

            to_upsample->data[outline].Cb = input->data[offset + j].Cb;
            to_upsample->data[outline + 1].Cb = input->data[offset + j].Cb;
            to_upsample->data[outline + width].Cb = input->data[offset + j].Cb;
            to_upsample->data[outline + 1 + width].Cb = input->data[offset + j].Cb;

            to_upsample->data[outline].Cr = input->data[offset + j].Cr;
            to_upsample->data[outline + 1].Cr = input->data[offset + j].Cr;
            to_upsample->data[outline + width].Cr = input->data[offset + j].Cr;
            to_upsample->data[outline + 1 + width].Cr = input->data[offset + j].Cr;
        }
    }
    
    return to_upsample;
}

/*
Purpose: Convert an array of YCC pixels to an array of RGB pixels.
Parameters:
    - input: A pointer to a ycc_data structure containing the YCC pixel array.
    - height: The height of the image.
    - width: The width of the image.
Returns: A pointer to an rgb_data structure containing the converted RGB pixel array.
*/
rgb_data* ycc_to_rgb(ycc_data* input, int height, int width) {
    int image_size = height * width;

    register rgb_data* to_rgb;
    to_rgb = malloc(sizeof(rgb_data));
    to_rgb->data = malloc(sizeof(rgb_pixel) * image_size);

    register int i;
    for (i = image_size - 1; i != -1; i -= 4) {
        int y1 = 4882170 * (input->data[i].Y - 16);
        int r1 = (y1 + 6694109 * (input->data[i].Cr - 128)) >> 22;
        int g1 = ((y1 - 3409969 * (input->data[i].Cr - 128) - 1639973 * (input->data[i].Cb - 128))) >> 22;
        int b1 = (y1 + 8464105 * (input->data[i].Cb - 128)) >> 22;

        to_rgb->data[i].R = r1 > 255 ? 255 : (r1 < 0 ? 0 : r1);
        to_rgb->data[i].G = g1 > 255 ? 255 : (g1 < 0 ? 0 : g1);
        to_rgb->data[i].B = b1 > 255 ? 255 : (b1 < 0 ? 0 : b1);

        int y2 = 4882170 * (input->data[i - 1].Y - 16);
        int r2 = (y2 + 6694109 * (input->data[i - 1].Cr - 128)) >> 22;
        int g2 = ((y2 - 3409969 * (input->data[i - 1].Cr - 128) - 1639973 * (input->data[i - 1].Cb - 128))) >> 22;
        int b2 = (y2 + 8464105 * (input->data[i - 1].Cb - 128)) >> 22;

        to_rgb->data[i - 1].R = r2 > 255 ? 255 : (r2 < 0 ? 0 : r2);
        to_rgb->data[i - 1].G = g2 > 255 ? 255 : (g2 < 0 ? 0 : g2);
        to_rgb->data[i - 1].B = b2 > 255 ? 255 : (b2 < 0 ? 0 : b2);

        int y3 = 4882170 * (input->data[i - 2].Y - 16);
        int r3 = (y3 + 6694109 * (input->data[i - 2].Cr - 128)) >> 22;
        int g3 = ((y3 - 3409969 * (input->data[i - 2].Cr - 128) - 1639973 * (input->data[i - 2].Cb - 128))) >> 22;
        int b3 = (y3 + 8464105 * (input->data[i - 2].Cb - 128)) >> 22;

        to_rgb->data[i - 2].R = r3 > 255 ? 255 : (r3 < 0 ? 0 : r3);
        to_rgb->data[i - 2].G = g3 > 255 ? 255 : (g3 < 0 ? 0 : g3);
        to_rgb->data[i - 2].B = b3 > 255 ? 255 : (b3 < 0 ? 0 : b3);

        int y4 = 4882170 * (input->data[i - 3].Y - 16);
        int r4 = (y4 + 6694109 * (input->data[i - 3].Cr - 128)) >> 22;
        int g4 = ((y4 - 3409969 * (input->data[i - 3].Cr - 128) - 1639973 * (input->data[i - 3].Cb - 128))) >> 22;
        int b4 = (y4 + 8464105 * (input->data[i - 3].Cb - 128)) >> 22;

        to_rgb->data[i - 3].R = r4 > 255 ? 255 : (r4 < 0 ? 0 : r4);
        to_rgb->data[i - 3].G = g4 > 255 ? 255 : (g4 < 0 ? 0 : g4);
        to_rgb->data[i - 3].B = b4 > 255 ? 255 : (b4 < 0 ? 0 : b4);
    }

    return to_rgb;
}

int main(int argc, char* argv[]) {
    FILE* fptr;
    FILE* fout;

    fptr = fopen("dog100.raw", "rb");
    if (fptr == NULL) {
        printf("Input File error");
        exit(1);
    }

    fout = fopen("dog100_new.raw", "wb");
    if (fout == NULL) {
        printf("Output File error");
        exit(1);
    }

    // Height and Width must be predefined for RAW file processing
    int width = 100;
    int height = 100;
    int image_size = width * height;

    rgb_data* input;
    input = malloc(sizeof(rgb_data));
    input->data = malloc(sizeof(rgb_pixel) * image_size);

    fread(input->data, sizeof(rgb_pixel), image_size, fptr);

    ycc_data* ycc_input = rgb_to_ycc(input, height, width);
    sampling_data* ycc_downsample = downsample_ycc(ycc_input, height, width);
    ycc_data* ycc_upsample = upsample_ycc(ycc_downsample, height, width);
    rgb_data* output = ycc_to_rgb(ycc_upsample, height, width);

    register int i;
    for (i = 0; i < image_size; i++) {
        fwrite(&(output->data[i].R), sizeof(uint8_t), 1, fout);
        fwrite(&(output->data[i].G), sizeof(uint8_t), 1, fout);
        fwrite(&(output->data[i].B), sizeof(uint8_t), 1, fout);
    }

    fclose(fout);
    fclose(fptr);

    return 0;
}