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

typedef struct ycc_meta {
    float Y1;
    float Y2;
    float Y3;
    float Y4;
    float Cb;
    float Cr;
} ycc_meta;

typedef struct ycc_array {
    struct ycc_pixel P1;
    struct ycc_pixel P2;
    struct ycc_pixel P3;
    struct ycc_pixel P4;
} ycc_array;

typedef struct rgb_array {
    struct rgb_pixel P1;
    struct rgb_pixel P2;
    struct rgb_pixel P3;
    struct rgb_pixel P4;
} rgb_array;

typedef struct rgb_data {
    rgb_pixel* data;
} rgb_data;

typedef struct ycc_data {
    ycc_pixel* data;
} ycc_data;

typedef struct ycc_meta_data {
    ycc_meta* data;
} ycc_meta_data;

/*
Purpose: Print the RGB pixel values to a file.
Parameters:
    - pixel: A pointer to an rgb_pixel structure containing the RGB values.
    - out: A pointer to the FILE where the RGB pixel values will be written.
Returns: None.
*/
void print_rgb_pixel(rgb_pixel* pixel, FILE* out) {
    fwrite(&pixel->R, sizeof(uint8_t), 1, out);
    fwrite(&pixel->G, sizeof(uint8_t), 1, out);
    fwrite(&pixel->B, sizeof(uint8_t), 1, out);
}


/*
Purpose: Convert an RGB pixel to YCbCr (YCC) color space.
Parameters:
    - in: A pointer to an rgb_pixel structure containing the RGB values.
Returns: A ycc_pixel structure containing the converted YCC values.
*/
ycc_pixel convert_to_ycc(rgb_pixel in) {
    ycc_pixel out;

    double conversion[3][3] = {
        {65.481, 128.553, 24.966},
        {-37.797, -74.203, 112.0},
        {112.0, -93.786, -18.214}
    };
    double inArray[3] = {
        ((double)in.R) / 255,
        ((double)in.G) / 255,
        ((double)in.B) / 255
    };

    out.Y = 16 + (conversion[0][0] * inArray[0] + conversion[0][1] * inArray[1] + conversion[0][2] * inArray[2]);
    out.Cb = 128 + (conversion[1][0] * inArray[0] + conversion[1][1] * inArray[1] + conversion[1][2] * inArray[2]);
    out.Cr = 128 + (conversion[2][0] * inArray[0] + conversion[2][1] * inArray[1] + conversion[2][2] * inArray[2]);

    return out;
}


/*
Purpose: Downsample four YCC pixels to create a YCC meta pixel.
Parameters:
    - in1, in2, in3, in4: Pointers to ycc_pixel structures containing YCC values for four pixels.
Returns: A ycc_meta structure containing the downsampled YCC values.
*/
ycc_meta downsample_ycc(ycc_pixel* in1, ycc_pixel* in2, ycc_pixel* in3, ycc_pixel* in4) {
    ycc_meta* out = malloc(sizeof(ycc_meta));
    out->Y1 = in1->Y;
    out->Y2 = in2->Y;
    out->Y3 = in3->Y;
    out->Y4 = in4->Y;
    out->Cb = (in1->Cb + in2->Cb + in3->Cb + in4->Cb) / 4;
    out->Cr = (in1->Cr + in2->Cr + in3->Cr + in4->Cr) / 4;

    return *out;
}

/*
Purpose: Upsample a YCC meta pixel to create four YCC pixels.
Parameters:
    - in: A pointer to a ycc_meta structure containing YCC meta values.
Returns: A ycc_array structure containing the upsampled YCC values.
*/
ycc_array upsample_ycc(ycc_meta* in) {
    ycc_array out;

    // Assign YCC values to the four upsampled pixels
    out.P1.Y = in->Y1;
    out.P2.Y = in->Y2;
    out.P3.Y = in->Y3;
    out.P4.Y = in->Y4;

    out.P1.Cb = in->Cb;
    out.P2.Cb = in->Cb;
    out.P3.Cb = in->Cb;
    out.P4.Cb = in->Cb;

    out.P1.Cr = in->Cr;
    out.P2.Cr = in->Cr;
    out.P3.Cr = in->Cr;
    out.P4.Cr = in->Cr;

    return out;
}

/*
Purpose: Truncate a float value to ensure it falls within the 0 to 255 range.
Parameters:
    - floatValue: The input float value to be clipped.
Returns: The truncated float value within the range [0, 255].
*/
float truncateFloat(float floatValue) {
    return (floatValue < 0.0f) ? 0.0f : ((floatValue > 255.0f) ? 255.0f : floatValue);
}

/*
Purpose: Convert a YCC pixel to RGB color space.
Parameters:
    - in: A pointer to a ycc_pixel structure containing the YCC values.
Returns: An rgb_pixel structure containing the converted RGB values.
*/
rgb_pixel convert_to_rgb(ycc_pixel* in) {
    rgb_pixel* out = malloc(sizeof(rgb_pixel));

    out->R = truncateFloat(1.164 * (in->Y - 16) + 1.596 * (in->Cr - 128));
    out->G = truncateFloat((1.164 * (in->Y - 16) - 0.813 * (in->Cr - 128) - 0.391 * (in->Cb - 128)));
    out->B = truncateFloat((1.164 * (in->Y - 16) + 2.018 * (in->Cb - 128)));

    return *out;
}

/*
Purpose: Convert an array of RGB pixels to an array of YCC pixels.
Parameters:
    - inData: A pointer to an rgb_data structure containing the RGB pixel array.
    - height: The height of the image.
    - width: The width of the image.
Returns: A pointer to a ycc_data structure containing the converted YCC pixel array.
*/
ycc_data* rgb_to_ycc(rgb_data* inData, int height, int width) {
    int imageSize = height * width;

    ycc_data* yccData;
    yccData = malloc(sizeof(ycc_data));
    yccData->data = malloc(sizeof(ycc_pixel) * imageSize);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int offset = i * width;
            yccData->data[offset + j] = convert_to_ycc(inData->data[offset + j]);
        }
    }

    return yccData;
}

/*
Purpose: Convert an array of YCC pixels to an array of YCC meta pixels.
Parameters:
    - inData: A pointer to a ycc_data structure containing the YCC pixel array.
    - height: The height of the image.
    - width: The width of the image.
Returns: A pointer to a ycc_meta_data structure containing the converted YCC meta pixel array.
*/
ycc_meta_data* ycc_to_meta(ycc_data* inData, int height, int width) {
    int imageSize = height * width;

    ycc_meta_data* yccMetaData;
    yccMetaData = malloc(sizeof(ycc_meta_data));
    yccMetaData->data = malloc(sizeof(ycc_meta) * imageSize / 4);

    for (int i = 0; i < height / 2; i++) {
        for (int j = 0; j < width / 2; j++) {
            int offset = i * width / 2;
            int tracer = i * 2 * width + j * 2;
            yccMetaData->data[offset + j] = downsample_ycc(&inData->data[tracer], 
                                                           &inData->data[tracer + 1],
                                                           &inData->data[tracer + width], 
                                                           &inData->data[tracer + 1 + width]);
        }
    }

    return yccMetaData;
}

/*
Purpose: Convert an array of YCC meta pixels to an array of YCC pixels.
Parameters:
    - inData: A pointer to a ycc_meta_data structure containing the YCC meta pixel array.
    - height: The height of the image.
    - width: The width of the image.
Returns: A pointer to a ycc_data structure containing the converted YCC pixel array.
*/
ycc_data* meta_to_ycc(ycc_meta_data* inData, int height, int width) {
    int imageSize = height * width;

    ycc_data* yccData;
    yccData = malloc(sizeof(ycc_data));
    yccData->data = malloc(sizeof(ycc_pixel) * imageSize);

    for (int i = 0; i < height / 2; i++) {
        for (int j = 0; j < width / 2; j++) {
            int offset = i * width / 2;
            int tracer = i * 2 * width + j * 2;
            ycc_array yccArray = upsample_ycc(&inData->data[offset + j]);
            yccData->data[tracer] = yccArray.P1;
            yccData->data[tracer + 1] = yccArray.P2;
            yccData->data[tracer + width] = yccArray.P3;
            yccData->data[tracer + 1 + width] = yccArray.P4;
        }
    }

    return yccData;
}

/*
Purpose: Convert an array of YCC pixels to an array of RGB pixels.
Parameters:
    - inData: A pointer to a ycc_data structure containing the YCC pixel array.
    - height: The height of the image.
    - width: The width of the image.
Returns: A pointer to an rgb_data structure containing the converted RGB pixel array.
*/
rgb_data* ycc_to_rgb(ycc_data* inData, int height, int width) {
    int imageSize = height * width;

    rgb_data* rgbData;
    rgbData = malloc(sizeof(rgb_data));
    rgbData->data = malloc(sizeof(rgb_pixel) * imageSize);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int offset = i * width;
            rgbData->data[offset + j] = convert_to_rgb(&inData->data[offset + j]);
        }
    }
    return rgbData;
}

/*
Purpose: Convert an array of RGB pixels to YCC, then back to RGB.
Parameters:
    - inData: A pointer to an rgb_data structure containing the RGB pixel array.
    - height: The height of the image.
    - width: The width of the image.
Returns: A pointer to an rgb_data structure containing the converted RGB pixel array.
*/
rgb_data* rgb_to_ycc_to_rgb(rgb_data* inData, int height, int width) {
    ycc_data* yccIn = rgb_to_ycc(inData, height, width);
    ycc_meta_data* yccMeta = ycc_to_meta(yccIn, height, width);
    ycc_data* yccOut = meta_to_ycc(yccMeta, height, width);

    return ycc_to_rgb(yccOut, height, width);
}

int main(int argc, char* argv[]) {
    FILE* pFile;
    FILE* outFile;

    pFile = fopen("dog100.raw", "rb");
    if (pFile == NULL) {
        printf("Input File error");
        exit(1);
    }

    outFile = fopen("dog100_new.raw", "wb");
    if (outFile == NULL) {
        printf("Output File error");
        exit(1);
    }

    // Height and Width must be predefined for RAW file processing
    int width = 100;
    int height = 100;
    int imageSize = width * height;

    rgb_data* inData;
    inData = malloc(sizeof(rgb_data));
    inData->data = malloc(sizeof(rgb_pixel) * imageSize);

    fread(inData->data, sizeof(rgb_pixel), imageSize, pFile);

    rgb_data* outData;
    outData = rgb_to_ycc_to_rgb(inData, height, width);

    for (int k = 0; k < imageSize; k++) 
        print_rgb_pixel(&outData->data[k], outFile);

    fclose(outFile);
    fclose(pFile);

    return 0;
}