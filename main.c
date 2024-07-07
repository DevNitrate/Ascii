#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stbi/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stbi/stb_image_write.h"

typedef struct {
    short r;
    short g;
    short b;
} Pixel;

Pixel** imgToPixels(int* x, int* y);
void freePixels(Pixel** pixs, int x);
unsigned char* pixelToBuf(Pixel** pixs, int x, int y);

float** toGrayscale(Pixel** pixs, int x, int y);
char** toAscii(float** grey, int x, int y);

void printPixel(Pixel pix) {
    printf("r: %i; g: %i; b: %i;\n", pix.r, pix.g, pix.b);
}

Pixel** downscaleImg(Pixel** pixs, int x, int y, int factor);

int main() {
    const int downf = 8;

    int x, y;
    Pixel** pixels = imgToPixels(&x, &y);

    Pixel** down = downscaleImg(pixels, x, y, downf);
    /*unsigned char* data = pixelToBuf(down, x/downf, y/downf);

    stbi_write_png("out.png", x/downf, y/downf, 3, data, (x/downf)*3);*/

    float** g = toGrayscale(down, x/downf, y/downf);
    char** ascii = toAscii(g, x/downf, y/downf);

    freePixels(pixels, x);
    freePixels(down, x/downf);
    //free(data);

    return 0;
}

Pixel** imgToPixels(int* x, int* y) {
    int width, height, comp;
    unsigned char* data = stbi_load("test.png", &width, &height, &comp, 3);

    if (data == NULL) {
        printf("Something went wrong while loading png");
    }

    *x = width;
    *y = height;

    Pixel** pixs = (Pixel**)malloc(width * sizeof(Pixel*));
    for (int i = 0; i < width; i++) {
        pixs[i] = (Pixel*)malloc(sizeof(Pixel)*height);
    }

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int index = (j * width + i) * 3;
            pixs[i][j].r = data[index];
            pixs[i][j].g = data[index + 1];
            pixs[i][j].b = data[index + 2];
        }
    }
    
    stbi_image_free(data);
    
    return pixs;
}

void freePixels(Pixel** pixs, int x) {
    for (int i = 0; i < x; i++) {
        free(pixs[i]);
    }

    free(pixs);
}

Pixel** downscaleImg(Pixel** pixs, int x, int y, int factor) {
    int width = x / factor;
    int height = y / factor;

    Pixel** down = (Pixel**)malloc(width * sizeof(Pixel*));
    for (int i = 0; i < width; i++) {
        down[i] = (Pixel*)malloc(sizeof(Pixel)*height);
    }
    
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            down[i][j] = pixs[i*factor][j*factor];
        }
    }

    return down;
}

unsigned char* pixelToBuf(Pixel** pixs, int x, int y) {
    unsigned char* data = (unsigned char*)malloc(((x*y)*3) * sizeof(unsigned char));

    int count = 0;
    int index = 1;

    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            data[count] = pixs[j][i].r;
            data[count+1] = pixs[j][i].g;
            data[count+2] = pixs[j][i].b;
            
            count += 3;
        }
    }

    return data;
}

float** toGrayscale(Pixel** pixs, int x, int y) {
    float** grey = (float**)malloc(x * sizeof(float*));
    for (int i = 0; i < x; i++) {
        grey[i] = (float*)malloc(y * sizeof(float));
    }

    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            int r = pixs[j][i].r;
            int g = pixs[j][i].g;
            int b = pixs[j][i].b;

            float avg = (r + g + b) / 3 / 255.0f;
            grey[j][i] = avg;
        }
    }

    return grey;
}

char** toAscii(float** grey, int x, int y) {
    const char* steps = " .:coPO?#@";

    char** ascii = (char**)malloc(y * sizeof(char*));
    for (int i = 0; i < y; i++) {
        ascii[i] = (char*)malloc(x * sizeof(char));
    }

    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            int step = (int)(grey[j][i]*10);
            ascii[i][j] = steps[step];
        }
    }

    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            printf("%c", ascii[i][j]);
            printf("%c", ascii[i][j]);
        }
        printf("\n");
    }
}
