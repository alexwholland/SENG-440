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

typedef struct ycc_meta {
    uint8_t Y1;
    uint8_t Y2;
    uint8_t Y3;
    uint8_t Y4;
    uint8_t Cb;
    uint8_t Cr;
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
Purpose: Convert an array of RGB pixels to an array of YCC pixels.
Parameters:
    - inData: A pointer to an rgb_data structure containing the RGB pixel array.
    - height: The height of the image.
    - width: The width of the image.
Returns: A pointer to a ycc_data structure containing the converted YCC pixel array.
*/
ycc_data* rgb_to_ycc(rgb_data* inData, int height, int width){
    int imageSize = height * width;

    register ycc_data* yccData;
    yccData = malloc(sizeof(ycc_data));
    yccData->data = malloc(sizeof(ycc_pixel) * imageSize);

    register int i;
    for (i = 0; i < imageSize; i += 4) {
        yccData->data[i].Y  = 16 + ((16763 * inData->data[i].R + 32909 * inData->data[i].G + 6391 * inData->data[i].B) >> 16);
        yccData->data[i].Cb = 128 + ((-9676 * inData->data[i].R + -18996 * inData->data[i].G + 28672 * inData->data[i].B) >> 16);
        yccData->data[i].Cr = 128 + ((28672 * inData->data[i].R + -24009 * inData->data[i].G + -4662 * inData->data[i].B) >> 16);

        yccData->data[i + 1].Y  = 16 + ((16763 * inData->data[i + 1].R + 32909 * inData->data[i + 1].G + 6391 * inData->data[i + 1].B) >> 16);
        yccData->data[i + 1].Cb = 128 + ((-9676 * inData->data[i + 1].R + -18996 * inData->data[i + 1].G + 28672 * inData->data[i + 1].B) >> 16);
        yccData->data[i + 1].Cr = 128 + ((28672 * inData->data[i + 1].R + -24009 * inData->data[i + 1].G + -4662 * inData->data[i + 1].B) >> 16);

        yccData->data[i + 2].Y  = 16 + ((16763 * inData->data[i + 2].R + 32909 * inData->data[i + 2].G + 6391 * inData->data[i + 2].B) >> 16);
        yccData->data[i + 2].Cb = 128 + ((-9676 * inData->data[i + 2].R + -18996 * inData->data[i + 2].G + 28672 * inData->data[i + 2].B) >> 16);
        yccData->data[i + 2].Cr = 128 + ((28672 * inData->data[i + 2].R + -24009 * inData->data[i + 2].G + -4662 * inData->data[i + 2].B) >> 16);

        yccData->data[i + 3].Y  = 16 + ((16763 * inData->data[i + 3].R + 32909 * inData->data[i + 3].G + 6391 * inData->data[i + 3].B) >> 16);
        yccData->data[i + 3].Cb = 128 + ((-9676 * inData->data[i + 3].R + -18996 * inData->data[i + 3].G + 28672 * inData->data[i + 3].B) >> 16);
        yccData->data[i + 3].Cr = 128 + ((28672 * inData->data[i + 3].R + -24009 * inData->data[i + 3].G + -4662 * inData->data[i + 3].B) >> 16);
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

    register ycc_meta_data* yccMetaData;
    yccMetaData = malloc(sizeof(ycc_meta_data));
    yccMetaData->data = malloc(sizeof(ycc_meta) * imageSize / 4);

    register int i, j, offset, tracer;
    for (i = (height >> 1) - 1; i != -1; i--) {
        offset = i * width >> 1;
        for (j = (width >> 1) - 1; j != -1; j--) {
            tracer = i * 2 * width + j * 2;
            yccMetaData->data[offset + j].Y1 = inData->data[tracer].Y;
            yccMetaData->data[offset + j].Y2 = inData->data[tracer + 1].Y;
            yccMetaData->data[offset + j].Y3 = inData->data[tracer + width].Y;
            yccMetaData->data[offset + j].Y4 = inData->data[tracer + 1 + width].Y;
            yccMetaData->data[offset + j].Cb = (inData->data[tracer].Cb + inData->data[tracer + 1].Cb + inData->data[tracer + width].Cb + inData->data[tracer + 1 + width].Cb) >> 2;
            yccMetaData->data[offset + j].Cr = (inData->data[tracer].Cr + inData->data[tracer + 1].Cr + inData->data[tracer + width].Cr + inData->data[tracer + 1 + width].Cr) >> 2;
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

    register ycc_data* yccData;
    yccData = malloc(sizeof(ycc_data));
    yccData->data = malloc(sizeof(ycc_pixel) * imageSize);
    
    register int i, j, offset, tracer;
    for (i = (height >> 1) - 1; i != -1; i--) {
        offset = i * width / 2;
        for (j = (width >> 1) - 1; j != -1; j--) {
            tracer = i * 2 * width + j * 2;
            yccData->data[tracer].Y = inData->data[offset + j].Y1;
            yccData->data[tracer + 1].Y = inData->data[offset + j].Y2;
            yccData->data[tracer + width].Y = inData->data[offset + j].Y3;
            yccData->data[tracer + 1 + width].Y = inData->data[offset + j].Y4;

            yccData->data[tracer].Cb = inData->data[offset + j].Cb;
            yccData->data[tracer + 1].Cb = inData->data[offset + j].Cb;
            yccData->data[tracer + width].Cb = inData->data[offset + j].Cb;
            yccData->data[tracer + 1 + width].Cb = inData->data[offset + j].Cb;

            yccData->data[tracer].Cr = inData->data[offset + j].Cr;
            yccData->data[tracer + 1].Cr = inData->data[offset + j].Cr;
            yccData->data[tracer + width].Cr = inData->data[offset + j].Cr;
            yccData->data[tracer + 1 + width].Cr = inData->data[offset + j].Cr;
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

    register rgb_data* rgbData;
    rgbData = malloc(sizeof(rgb_data));
    rgbData->data = malloc(sizeof(rgb_pixel) * imageSize);

    register int i;
    for (i = imageSize - 1; i != -1; i -= 4) {
        int y1 = 4882170 * (inData->data[i].Y - 16);
        int r1 = (y1 + 6694109 * (inData->data[i].Cr - 128)) >> 22;
        int g1 = ((y1 - 3409969 * (inData->data[i].Cr - 128) - 1639973 * (inData->data[i].Cb - 128))) >> 22;
        int b1 = (y1 + 8464105 * (inData->data[i].Cb - 128)) >> 22;

        rgbData->data[i].R = r1 > 255 ? 255 : (r1 < 0 ? 0 : r1);
        rgbData->data[i].G = g1 > 255 ? 255 : (g1 < 0 ? 0 : g1);
        rgbData->data[i].B = b1 > 255 ? 255 : (b1 < 0 ? 0 : b1);

        int y2 = 4882170 * (inData->data[i - 1].Y - 16);
        int r2 = (y2 + 6694109 * (inData->data[i - 1].Cr - 128)) >> 22;
        int g2 = ((y2 - 3409969 * (inData->data[i - 1].Cr - 128) - 1639973 * (inData->data[i - 1].Cb - 128))) >> 22;
        int b2 = (y2 + 8464105 * (inData->data[i - 1].Cb - 128)) >> 22;

        rgbData->data[i - 1].R = r2 > 255 ? 255 : (r2 < 0 ? 0 : r2);
        rgbData->data[i - 1].G = g2 > 255 ? 255 : (g2 < 0 ? 0 : g2);
        rgbData->data[i - 1].B = b2 > 255 ? 255 : (b2 < 0 ? 0 : b2);

        int y3 = 4882170 * (inData->data[i - 2].Y - 16);
        int r3 = (y3 + 6694109 * (inData->data[i - 2].Cr - 128)) >> 22;
        int g3 = ((y3 - 3409969 * (inData->data[i - 2].Cr - 128) - 1639973 * (inData->data[i - 2].Cb - 128))) >> 22;
        int b3 = (y3 + 8464105 * (inData->data[i - 2].Cb - 128)) >> 22;

        rgbData->data[i - 2].R = r3 > 255 ? 255 : (r3 < 0 ? 0 : r3);
        rgbData->data[i - 2].G = g3 > 255 ? 255 : (g3 < 0 ? 0 : g3);
        rgbData->data[i - 2].B = b3 > 255 ? 255 : (b3 < 0 ? 0 : b3);

        int y4 = 4882170 * (inData->data[i - 3].Y - 16);
        int r4 = (y4 + 6694109 * (inData->data[i - 3].Cr - 128)) >> 22;
        int g4 = ((y4 - 3409969 * (inData->data[i - 3].Cr - 128) - 1639973 * (inData->data[i - 3].Cb - 128))) >> 22;
        int b4 = (y4 + 8464105 * (inData->data[i - 3].Cb - 128)) >> 22;

        rgbData->data[i - 3].R = r4 > 255 ? 255 : (r4 < 0 ? 0 : r4);
        rgbData->data[i - 3].G = g4 > 255 ? 255 : (g4 < 0 ? 0 : g4);
        rgbData->data[i - 3].B = b4 > 255 ? 255 : (b4 < 0 ? 0 : b4);
    }

    return rgbData;
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

    ycc_data* yccIn = rgb_to_ycc(inData, height, width);
    ycc_meta_data* yccMeta = ycc_to_meta(yccIn, height, width);
    ycc_data* yccOut = meta_to_ycc(yccMeta, height, width);
    rgb_data* outData = ycc_to_rgb(yccOut, height, width);

    register int i;
    for (i = 0; i < imageSize; i++) {
        fwrite(&(outData->data[i].R), sizeof(uint8_t), 1, outFile);
        fwrite(&(outData->data[i].G), sizeof(uint8_t), 1, outFile);
        fwrite(&(outData->data[i].B), sizeof(uint8_t), 1, outFile);
    }

    fclose(outFile);
    fclose(pFile);

    return 0;
}