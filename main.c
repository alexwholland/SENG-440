#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint8_t signature[2];   // Signature, should be 'B' and 'M'
    uint32_t fileSize;      // Size of the BMP file
    uint32_t reserved;      // Reserved, should be 0
    uint32_t dataOffset;    // Offset to the pixel data
    uint32_t headerSize;    // Size of the header
    int32_t width;          // Width of the image in pixels
    int32_t height;         // Height of the image in pixels
    uint16_t planes;        // Number of color planes, should be 1
    uint16_t bitsPerPixel;  // Number of bits per pixel
} BMPHeader;

int readBMPHeader(FILE *file, BMPHeader *header) {
    fread(header->signature, sizeof(uint8_t), 2, file);
    fread(&header->fileSize, sizeof(uint32_t), 1, file);
    fread(&header->reserved, sizeof(uint32_t), 1, file);
    fread(&header->dataOffset, sizeof(uint32_t), 1, file);
    fread(&header->headerSize, sizeof(uint32_t), 1, file);
    fread(&header->width, sizeof(int32_t), 1, file);
    fread(&header->height, sizeof(int32_t), 1, file);
    fread(&header->planes, sizeof(uint16_t), 1, file);
    fread(&header->bitsPerPixel, sizeof(uint16_t), 1, file);

    if (header->signature[0] != 'B' || header->signature[1] != 'M') {
        printf("The file is not a valid BMP image.\n");
        return 0;
    }
    return 1;
}

int main() {
    FILE *file = fopen("dog100.bmp", "rb"); // Open the BMP file in binary mode
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return 1;
    }

    BMPHeader header;
    if (!readBMPHeader(file, &header)) {
        fclose(file);
        return 1;
    }

    printf("Image size: %dx%d pixels\n", header.width, header.height);

    fclose(file);
    return 0;
}
