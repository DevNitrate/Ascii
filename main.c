#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stbi/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stbi/stb_image_write.h"
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <windows.h>

typedef struct {
    short r;
    short g;
    short b;
} Pixel;

Pixel** imgToPixels(const char* filename, int* x, int* y);
void freePixels(Pixel** pixs, int x);
unsigned char* pixelToBuf(Pixel** pixs, int x, int y);

float** toGrayscale(Pixel** pixs, int x, int y);
char** toAscii(float** grey, int x, int y);
char* asciiToBuf(char** ascii, int x, int y);
void outputFile(char* buf, const char* filename, int use);
void copyBuf(char* str);

void printPixel(Pixel pix) {
    printf("r: %i; g: %i; b: %i;\n", pix.r, pix.g, pix.b);
}

Pixel** downscaleImg(Pixel** pixs, int x, int y, int factor);

int main(int argc, char** argv) {
    if (argc != 4 && argc != 5 && argc != 6) {
        printf("Incorrect command use: 'ascii <input.png> <output.txt> <scale factor>'\noptions:\n'-c': copy output to the clipboard\n'-r': the scale factor becomes the required width of the ascii art\n\nnote: if the output file is set to 'none' the result won't be output to a file. always put the options at the end of the command");
        exit(1);
    }
    int downf = 15;
    char* pngFile = 0;
    char* txtFile = 0;
    int useTxt = 1;
    int req = 0;
    int copy = 0;

    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];

        switch (i) {
            case 1:
                pngFile = (char*)realloc(pngFile, sizeof(char) * strlen(arg));
                strcpy(pngFile, arg);
                break;

            case 2:
                txtFile = (char*)realloc(txtFile, sizeof(char) * strlen(arg));
                strcpy(txtFile, arg);
                if (strcmp(txtFile, "none") == 0) {
                    useTxt = 0;
                }
                break;

            case 3:
                downf = atoi(arg);

                if (downf == 0) {
                    printf("scale factor isn't an integer or is equal to 0");
                    exit(1);
                }
                break;

            case 4:
                if (strcmp(arg, "-c") == 0) {
                    copy = 1;
                } else if (strcmp(arg, "-r") == 0) {
                    req = 1;
                } else {
                    printf("unknown arg: %s", arg);
                    exit(1);
                }
                break;

            case 5:
                if (strcmp(arg, "-c") == 0) {
                    copy = 1;
                } else if (strcmp(arg, "-r") == 0) {
                    req = 1;
                } else {
                    printf("unknown arg: %s", arg);
                    exit(1);
                }
                break;

            default:
                break;
        }
    }

    int x, y;
    Pixel** pixels = imgToPixels(pngFile, &x, &y);

    if (req) {
        int div = floorf((float)x / (float)downf);
        downf = (int)((float)div*2.f);
    }

    Pixel** down = downscaleImg(pixels, x, y, downf);

    float** g = toGrayscale(down, x/downf, y/downf);
    char** ascii = toAscii(g, x/downf, y/downf);
    char* buf = asciiToBuf(ascii, x/downf, y/downf);

    outputFile(buf, txtFile, useTxt);

    if (copy) {
        copyBuf(buf);
    }

    freePixels(pixels, x);
    freePixels(down, x/downf);

    return 0;
}

Pixel** imgToPixels(const char* filename, int* x, int* y) {
    int width, height, comp;
    unsigned char* data = stbi_load(filename, &width, &height, &comp, 3);

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
            if (step == 10) step = 9;
            ascii[i][j] = steps[step];
        }
    }

    return ascii;
}

char* asciiToBuf(char** ascii, int x, int y) {
    char* res = (char*)malloc((x+1)*y*2+1);

    int count = 0;

    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            char c = ascii[i][j];

            res[count] = c;
            res[count+1] = c;

            /*printf("%c", c);
            printf("%c", c);*/

            count += 2;
        }
        res[count] = '\n';
        count++;
        //printf("\n");
    }

    return res;
}

void outputFile(char* buf, const char* filename, int use) {
    if (use) {
        FILE* out = fopen(filename, "w");
        fprintf(out, buf);
    } else {
        printf("note: no output file was selected");
    }
}

void copyBuf(char* str) {
    // Calculate the length of the string including the null terminator
    size_t len = strlen(str) + 1;

    // Allocate global memory for the string
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (hMem == NULL) {
        fprintf(stderr, "GlobalAlloc failed\n");
        return;
    }

    // Copy the string to the allocated memory
    memcpy(GlobalLock(hMem), str, len);
    GlobalUnlock(hMem);

    // Open the clipboard
    if (!OpenClipboard(NULL)) {
        fprintf(stderr, "OpenClipboard failed\n");
        GlobalFree(hMem);
        return;
    }

    // Empty the clipboard
    EmptyClipboard();

    // Set the clipboard data
    if (SetClipboardData(CF_TEXT, hMem) == NULL) {
        fprintf(stderr, "SetClipboardData failed\n");
        GlobalFree(hMem);
        CloseClipboard();
        return;
    }

    // Close the clipboard
    CloseClipboard();
}
