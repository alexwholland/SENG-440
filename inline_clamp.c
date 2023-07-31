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

static inline int32_t clamp(int32_t x) {
    uint32_t y = x >> 8;
    if (y)
        x = ~y >> 24;
    
    return x;
}

rgb_data* ycc_to_rgb(ycc_data* input, int height, int width) {
    int image_size = height * width;

    register rgb_data* to_rgb;
    to_rgb = malloc(sizeof(rgb_data));
    to_rgb->data = malloc(sizeof(rgb_pixel) * image_size);

    register int i;
    for (i = image_size - 1; i != -1; i -= 4) {
        int Y0 = input->data[i].Y - 16;
        int Y1 = input->data[i - 1].Y - 16;
        int Y2 = input->data[i - 2].Y - 16;
        int Y3 = input->data[i - 3].Y - 16;

        int Cb0 = input->data[i].Cb - 128;
        int Cb1 = input->data[i - 1].Cb - 128;
        int Cb2 = input->data[i - 2].Cb - 128;
        int Cb3 = input->data[i - 3].Cb - 128;
        
        int Cr0 = input->data[i].Cr - 128;
        int Cr1 = input->data[i - 1].Cr - 128;
        int Cr2 = input->data[i - 2].Cr - 128;
        int Cr3 = input->data[i - 3].Cr - 128;

        to_rgb->data[i].R = clamp((4882170 * Y0 + 6694109 * Cr0) >> 22);
        to_rgb->data[i].G = clamp((4882170 * Y0 - 3409969 * Cr0 - 1639973 * Cb0) >> 22);
        to_rgb->data[i].B = clamp((4882170 * Y0 + 8464105 * Cb0) >> 22);

        to_rgb->data[i - 1].R = clamp((4882170 * Y1 + 6694109 * Cr1) >> 22);
        to_rgb->data[i - 1].G = clamp((4882170 * Y1 - 3409969 * Cr1 - 1639973 * Cb1) >> 22);
        to_rgb->data[i - 1].B = clamp((4882170 * Y1 + 8464105 * Cb1) >> 22);

        to_rgb->data[i - 2].R = clamp((4882170 * Y2 + 6694109 * Cr2) >> 22);
        to_rgb->data[i - 2].G = clamp((4882170 * Y2 - 3409969 * Cr2 - 1639973 * Cb2) >> 22);
        to_rgb->data[i - 2].B = clamp((4882170 * Y2 + 8464105 * Cb2) >> 22);

        to_rgb->data[i - 3].R = clamp((4882170 * Y3 + 6694109 * Cr3) >> 22);
        to_rgb->data[i - 3].G = clamp((4882170 * Y3 - 3409969 * Cr3 - 1639973 * Cb3) >> 22);
        to_rgb->data[i - 3].B = clamp((4882170 * Y3 + 8464105 * Cb3) >> 22);
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