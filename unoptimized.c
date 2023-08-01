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
    float Y;
    float Cb;
    float Cr;
} ycc_pixel;

typedef struct sample {
    float Y1;
    float Y2;
    float Y3;
    float Y4;
    float Cb;
    float Cr;
} sample;

typedef struct ycc_intermediary {
    struct ycc_pixel P1;
    struct ycc_pixel P2;
    struct ycc_pixel P3;
    struct ycc_pixel P4;
} ycc_intermediary;

typedef struct rgb_data {
    rgb_pixel* data;
} rgb_data;

typedef struct ycc_data {
    ycc_pixel* data;
} ycc_data;

typedef struct sampling_data {
    sample* data;
} sampling_data;

float clamp(float x) {
    return ((x < 0.0f) ? 0.0f : ((x > 255.0f) ? 255.0f : x));
}

ycc_pixel convert_to_ycc(rgb_pixel input) {
    ycc_pixel output;

    // Convert RGB values to the range [0, 1]
    float R = ((float)input.R) / 255.0f;
    float G = ((float)input.G) / 255.0f;
    float B = ((float)input.B) / 255.0f;

    // Convert to YCC color space without scaling
    output.Y = (0.299f * R) + (0.587f * G) + (0.114f * B);
    output.Cb = (-0.168736f * R) - (0.331264f * G) + (0.5f * B);
    output.Cr = (0.5f * R) - (0.418688f * G) - (0.081312f * B);

    // Scale YCC values to the range [0, 255]
    output.Y = clamp(16 + (output.Y * 219));
    output.Cb = clamp(128 + (output.Cb * 224));
    output.Cr = clamp(128 + (output.Cr * 224));

    return output;
}

sample downsample_ycc(ycc_pixel* input_1, ycc_pixel* input_2, ycc_pixel* input_3, ycc_pixel* input_4) {
    sample* output = malloc(sizeof(sample));
    output->Y1 = input_1->Y;
    output->Y2 = input_2->Y;
    output->Y3 = input_3->Y;
    output->Y4 = input_4->Y;
    output->Cb = (input_1->Cb + input_2->Cb + input_3->Cb + input_4->Cb) / 4;
    output->Cr = (input_1->Cr + input_2->Cr + input_3->Cr + input_4->Cr) / 4;

    return *output;
}

ycc_intermediary upsample_ycc(sample* input) {
    ycc_intermediary output;

    // Assign YCC values to the four upsampled pixels
    output.P1.Y = input->Y1;
    output.P2.Y = input->Y2;
    output.P3.Y = input->Y3;
    output.P4.Y = input->Y4;

    output.P1.Cb = input->Cb;
    output.P2.Cb = input->Cb;
    output.P3.Cb = input->Cb;
    output.P4.Cb = input->Cb;

    output.P1.Cr = input->Cr;
    output.P2.Cr = input->Cr;
    output.P3.Cr = input->Cr;
    output.P4.Cr = input->Cr;

    return output;
}

rgb_pixel convert_to_rgb(ycc_pixel* input) {
    rgb_pixel* output = malloc(sizeof(rgb_pixel));

    output->R = clamp(1.164f * (input->Y - 16) + 1.596f * (input->Cr - 128));
    output->G = clamp(1.164f * (input->Y - 16) - 0.813f * (input->Cr - 128) - 0.391f * (input->Cb - 128));
    output->B = clamp(1.164f * (input->Y - 16) + 2.018f * (input->Cb - 128));

    return *output;
}

ycc_data* rgb_to_ycc(rgb_data* input, int height, int width) {
    int image_size = height * width;

    ycc_data* to_ycc;
    to_ycc = malloc(sizeof(ycc_data));
    to_ycc->data = malloc(sizeof(ycc_pixel) * image_size);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int offset = i * width;
            to_ycc->data[offset + j] = convert_to_ycc(input->data[offset + j]);
        }
    }

    return to_ycc;
}

sampling_data* ycc_to_intermediary(ycc_data* input, int height, int width) {
    int image_size = height * width;

    sampling_data* to_intermediary;
    to_intermediary = malloc(sizeof(sampling_data));
    to_intermediary->data = malloc(sizeof(sample) * image_size / 4);

    for (int i = 0; i < height / 2; i++) {
        for (int j = 0; j < width / 2; j++) {
            int offset = i * width / 2;
            int outline = i * 2 * width + j * 2;
            to_intermediary->data[offset + j] = downsample_ycc(&input->data[outline], 
                                                               &input->data[outline + 1],
                                                               &input->data[outline + width], 
                                                               &input->data[outline + 1 + width]);
        }
    }

    return to_intermediary;
}

ycc_data* intermediary_to_ycc(sampling_data* input, int height, int width) {
    int image_size = height * width;

    ycc_data* to_ycc;
    to_ycc = malloc(sizeof(ycc_data));
    to_ycc->data = malloc(sizeof(ycc_pixel) * image_size);

    for (int i = 0; i < height / 2; i++) {
        for (int j = 0; j < width / 2; j++) {
            int offset = i * width / 2;
            int outline = i * 2 * width + j * 2;
            ycc_intermediary to_upsample = upsample_ycc(&input->data[offset + j]);
            to_ycc->data[outline] = to_upsample.P1;
            to_ycc->data[outline + 1] = to_upsample.P2;
            to_ycc->data[outline + width] = to_upsample.P3;
            to_ycc->data[outline + 1 + width] = to_upsample.P4;
        }
    }

    return to_ycc;
}

rgb_data* ycc_to_rgb(ycc_data* input, int height, int width) {
    int image_size = height * width;

    rgb_data* to_rgb;
    to_rgb = malloc(sizeof(rgb_data));
    to_rgb->data = malloc(sizeof(rgb_pixel) * image_size);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int offset = i * width;
            to_rgb->data[offset + j] = convert_to_rgb(&input->data[offset + j]);
        }
    }
    return to_rgb;
}

int main(int argc, char* argv[]) {
    FILE* fptr;
    FILE* fout;

    fptr = fopen("input_test.raw", "rb");
    if (fptr == NULL) {
        printf("Input File error");
        exit(1);
    }

    fout = fopen("input_test_converted.raw", "wb");
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
    sampling_data* ycc_downsample = ycc_to_intermediary(ycc_input, height, width);
    ycc_data* ycc_upsample = intermediary_to_ycc(ycc_downsample, height, width);
    rgb_data* output = ycc_to_rgb(ycc_upsample, height, width);

    for (int i = 0; i < image_size; i++) {
        fwrite(&(output->data[i].R), sizeof(uint8_t), 1, fout);
        fwrite(&(output->data[i].G), sizeof(uint8_t), 1, fout);
        fwrite(&(output->data[i].B), sizeof(uint8_t), 1, fout);
    } 

    fclose(fout);
    fclose(fptr);

    return 0;
}