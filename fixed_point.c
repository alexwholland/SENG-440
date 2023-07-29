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
Purpose: Print the RGB pixel values to a file.
Parameters:
    - pixel: A pointer to an rgb_pixel structure containing the RGB values.
    - out: A pointer to the FILE where the RGB pixel values will be written.
Returns: None.
*/
void print_rgb_pixel(rgb_pixel* pixel, FILE* out) {
    fwrite(&(pixel->R), sizeof(uint8_t), 1, out);
    fwrite(&(pixel->G), sizeof(uint8_t), 1, out);
    fwrite(&(pixel->B), sizeof(uint8_t), 1, out);
}

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

  ycc_data* yccData;
  yccData = malloc(sizeof(ycc_data));
  yccData->data = malloc(sizeof(ycc_pixel) * imageSize);

  for(int i = 0; i < imageSize; i++){
    yccData->data[i].Y  = 16 +  (( 16763*inData->data[i].R +  32909*inData->data[i].G +  6391*inData->data[i].B) >> 16);
    yccData->data[i].Cb = 128 + (( -9676*inData->data[i].R + -18996*inData->data[i].G + 28672*inData->data[i].B) >> 16);
    yccData->data[i].Cr = 128 + (( 28672*inData->data[i].R + -24009*inData->data[i].G + -4662*inData->data[i].B) >> 16);
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
ycc_meta_data* ycc_to_meta(ycc_data* inData, int height, int width){
    int imageSize = height * width;
    int metaSize = imageSize >> 2;
    int halfHeight = height >> 1;
    int halfWidth = width >> 1;

    ycc_meta_data* yccMetaData;
    yccMetaData = malloc(sizeof(ycc_meta_data));
    yccMetaData->data = malloc(sizeof(ycc_meta) * metaSize);

    for(int i = 0; i < halfHeight; i++){
        int offset = i * width >> 1;
        for(int j = 0; j < halfWidth; j++){
            int tracer = i * 2 * width + j * 2;
            yccMetaData->data[offset+j].Y1  = inData->data[tracer].Y;
            yccMetaData->data[offset+j].Y2  = inData->data[tracer+1].Y;
            yccMetaData->data[offset+j].Y3  = inData->data[tracer+width].Y;
            yccMetaData->data[offset+j].Y4  = inData->data[tracer+1+width].Y;
            yccMetaData->data[offset+j].Cb = (inData->data[tracer].Cb + inData->data[tracer+1].Cb + inData->data[tracer+width].Cb + inData->data[tracer+1+width].Cb) >> 2;
            yccMetaData->data[offset+j].Cr = (inData->data[tracer].Cr + inData->data[tracer+1].Cr + inData->data[tracer+width].Cr + inData->data[tracer+1+width].Cr) >> 2;
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
ycc_data* meta_to_ycc(ycc_meta_data* inData, int height, int width){
    int imageSize = height * width;
    int metaSize = imageSize >> 2;
    int halfHeight = height >> 1;
    int halfWidth = width >> 1;

    ycc_data* yccData;
    yccData = malloc(sizeof(ycc_data));
    yccData->data = malloc(sizeof(ycc_pixel) * imageSize);

    for(int i = 0; i < halfHeight; i++){
        int offset = i * width / 2;
        for(int j = 0; j < halfWidth; j++){
        int tracer = i * 2 * width + j * 2;
        yccData->data[tracer].Y = inData->data[offset+j].Y1;
        yccData->data[tracer+1].Y = inData->data[offset+j].Y2;
        yccData->data[tracer+width].Y = inData->data[offset+j].Y3;
        yccData->data[tracer+1+width].Y = inData->data[offset+j].Y4;

        yccData->data[tracer].Cb = inData->data[offset+j].Cb;
        yccData->data[tracer+1].Cb = inData->data[offset+j].Cb;
        yccData->data[tracer+width].Cb = inData->data[offset+j].Cb;
        yccData->data[tracer+1+width].Cb = inData->data[offset+j].Cb;

        yccData->data[tracer].Cr = inData->data[offset+j].Cr;
        yccData->data[tracer+1].Cr = inData->data[offset+j].Cr;
        yccData->data[tracer+width].Cr = inData->data[offset+j].Cr;
        yccData->data[tracer+1+width].Cr = inData->data[offset+j].Cr;
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
rgb_data* ycc_to_rgb(ycc_data* inData, int height, int width){
  int imageSize = height * width;

  rgb_data* rgbData;
  rgbData = malloc(sizeof(rgb_data));
  rgbData->data = malloc(sizeof(rgb_pixel)*imageSize);
  for(int i = 0; i < imageSize ; i++){
    int y1 = 4882170*(inData->data[i].Y -16);
    int r1 = (y1 + 6694109*(inData->data[i].Cr - 128)) >> 22;
    int g1 = ((y1 - 3409969*(inData->data[i].Cr - 128) - 1639973*(inData->data[i].Cb - 128))) >> 22;
    int b1 = (y1 + 8464105*(inData->data[i].Cb - 128)) >> 22;

    rgbData->data[i].R = r1 > 255 ? 255 : (r1 < 0 ? 0 : r1);
    rgbData->data[i].G = g1 > 255 ? 255 : (g1 < 0 ? 0 : g1);
    rgbData->data[i].B = b1 > 255 ? 255 : (b1 < 0 ? 0 : b1);
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