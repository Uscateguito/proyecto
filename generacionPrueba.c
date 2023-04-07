#include <stdio.h>
#include <stdint.h>

#pragma pack(2)
typedef struct
{
    uint16_t signature; // BM
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t pixelArrayOffset;
} BMPFileHeader;

#pragma pack(2)
typedef struct
{
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t importantColors;
} BMPInfoHeader;

int main()
{
    int width = 500;  // Image width
    int height = 500; // Image height

    // Create a blank bitmap image with a size of 256x256 pixels
    BMPFileHeader fileHeader = {'B' + ('M' << 8), sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + width * height * 3, 0, 0, sizeof(BMPFileHeader) + sizeof(BMPInfoHeader)};
    BMPInfoHeader infoHeader = {sizeof(BMPInfoHeader), width, height, 1, 24, 0, width * height * 3, 0, 0, 0, 0};

    // Open a binary file for writing
    FILE *file = fopen("pruebini.bmp", "wb");

    // Write the file header and info header to the file
    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, file);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, file);

    // Set the color of each pixel to red
    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < width; x++)
        {
            if (x <= width / 3)
            {
                uint8_t blue = 0;
                uint8_t green = 255;
                uint8_t red =255;
                fwrite(&blue, sizeof(uint8_t), 1, file);  // Blue
                fwrite(&green, sizeof(uint8_t), 1, file); // Green
                fwrite(&red, sizeof(uint8_t), 1, file);   // Red
            }
            else if (x >= width / 3 && x < 2 * width / 3)
            {
                uint8_t blue = 255;
                uint8_t green = 0;
                uint8_t red = 0;
                fwrite(&blue, sizeof(uint8_t), 1, file);  // Blue
                fwrite(&green, sizeof(uint8_t), 1, file); // Green
                fwrite(&red, sizeof(uint8_t), 1, file);   // Red
            }
            else if (x >= 2 * width / 3 && x <= width)
            {
                uint8_t blue = 0;
                uint8_t green = 0;
                uint8_t red = 255;
                fwrite(&blue, sizeof(uint8_t), 1, file);  // Blue
                fwrite(&green, sizeof(uint8_t), 1, file); // Green
                fwrite(&red, sizeof(uint8_t), 1, file);   // Red
            }
        }
    }

    // Close the file
    fclose(file);

    return 0;
}
