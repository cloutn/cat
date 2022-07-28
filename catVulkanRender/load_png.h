#pragma once

#include <stdio.h>

unsigned char* load_png_data_to_memory(FILE* fp, unsigned char* rgba, int* out_width, int* out_height, int* pitch, int* pixel);
