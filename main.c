#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

char* loadPng(const char* filename, int* width, int* height) {
  unsigned char* image = NULL;
  int error = lodepng_decode32_file(&image, width, height, filename);
  if (error){
    printf("error %u: %s\n", error, lodepng_error_text(error));
  }
  return (image);
}

void calculateBrightness(int w, int h, unsigned char *image, unsigned char *brightness) {
    for (int i = 0; i < w * h; i++) {
         brightness[i] = 0.3 * image[4 * i] + 0.600 * image[4 * i + 1] + 0.110 * image[4 * i + 2];
    }
    return;
}

void binarizeImage(int w, int h, unsigned char *brightness) {
    for (int i = 2; i < h - 1; i++) {
        for (int j = 2; j < w - 1; j++) {
            if (brightness[w * i + j] < 60) {
                brightness[w * i + j] = 0;
            }
            if (brightness[w * i + j] > 160) {
                brightness[w * i + j] = 255;
            }
        }
    }
    return;
}

void applyGaussianFilter(int w, int h, unsigned char *input, unsigned char *output) {
    float kernel[7][7] = {
        {0.000036, 0.000363, 0.001446, 0.002291, 0.001446, 0.000363, 0.000036},
        {0.000363, 0.003676, 0.014662, 0.023226, 0.014662, 0.003676, 0.000363},
        {0.001446, 0.014662, 0.058488, 0.092651, 0.058488, 0.014662, 0.001446},
        {0.002291, 0.023226, 0.092651, 0.146768, 0.092651, 0.023226, 0.002291},
        {0.001446, 0.014662, 0.058488, 0.092651, 0.058488, 0.014662, 0.001446},
        {0.000363, 0.003676, 0.014662, 0.023226, 0.014662, 0.003676, 0.000363},
        {0.000036, 0.000363, 0.001446, 0.002291, 0.001446, 0.000363, 0.000036}
    };

    for (int i = 3; i < h - 3; i++) {
        for (int j = 3; j < w - 3; j++) {
            float sum = 0.0;
            for (int k = -3; k <= 3; k++) {
                for (int l = -3; l <= 3; l++) {
                    sum += kernel[k + 3][l + 3] * input[w * (i + k) + (j + l)];
                }
            }
            output[w * i + j] = (unsigned char)sum;
        }
    }
}

void colorPattern(int w, int h, unsigned char *brightness, unsigned char *coloredImage) {
    for (int i = 0; i < w * h; i++) {
        if (brightness[i] < 20) {
            coloredImage[i * 4] = 0;
            coloredImage[i * 4 + 1] = 0;
            coloredImage[i * 4 + 2] = 255;
        }
        else if (brightness[i] > 160) {
            coloredImage[i * 4] = 255;
            coloredImage[i * 4 + 1] = 0;   
            coloredImage[i * 4 + 2] = 0;
        }
        else {
            coloredImage[i * 4] = 255 - brightness[i];
            coloredImage[i * 4 + 1] = brightness[i];
            coloredImage[i * 4 + 2] = 255 - brightness[i];
        }
        coloredImage[i * 4 + 3] = 200;
    }
}

void writePng(const char* filename, const unsigned char* image, unsigned width, unsigned height) {
    unsigned char* png;
    long unsigned int pngsize;

    int error = lodepng_encode32(&png, &pngsize, image, width, height);
    if (!error) {
        lodepng_save_file(png, pngsize, filename);
    }
    if (error) printf("error %u: %s\n", error, lodepng_error_text(error));
    free(png);
}

int main() {
    const char* filename = "skull.png";
    int width, height;
    unsigned char* image = loadPng(filename, &width, &height);
    if (image == NULL) {
        printf("I can not read the picture from the file %s. Error.\n", filename);
        return -1;
    }
    unsigned char* brightness = (unsigned char*)malloc(height * width * sizeof(unsigned char));
    unsigned char* binarizedImage = (unsigned char*)malloc(height * width * sizeof(unsigned char));
    unsigned char* smoothedImage1 = (unsigned char*)malloc(height * width * sizeof(unsigned char));
    unsigned char* smoothedImage2 = (unsigned char*)malloc(height * width * sizeof(unsigned char));
    unsigned char* coloredImage = (unsigned char*)malloc(height * width * 4 * sizeof(unsigned char));

    calculateBrightness(width, height, image, brightness);
    binarizeImage(width, height, brightness);
    applyGaussianFilter(width, height, brightness, smoothedImage1);
    applyGaussianFilter(width, height, smoothedImage1, smoothedImage2);
    colorPattern(width, height, smoothedImage2, coloredImage);

    const char* newImage = "skull-modified.png";
    writePng(newImage, coloredImage, width, height);

    free(image);
    free(brightness);
    free(binarizedImage);
    free(smoothedImage1);
    free(smoothedImage2);
    free(coloredImage);

    return 0;
}
