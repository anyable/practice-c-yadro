#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint32_t H, W;
  uint8_t * A;
  uint8_t * B;
  uint8_t * C;
  uint16_t DH, DW;
  int8_t * D;
} Data;


void freeData(Data * data) {
  if (data == NULL) {
    return;
  }
  if (data->A) {
    free(data->A);
    data->A = NULL;
  }
  if (data->B) {
    free(data->B);
    data->B = NULL;
  }
  if (data->C) {
    free(data->C);
    data->C = NULL;
  }
  if (data->D) {
    free(data->D);
    data->D = NULL;
  }
}


int readFile(const char * filename, Data * data) {
  FILE * f = fopen(filename, "rb");
  if (f == NULL) {
    perror("Error open input file\n");
    return 0;
  }

  if ((fread(&data->H, sizeof(uint32_t), 1, f) == 0) ||
     (fread(&data->W, sizeof(uint32_t), 1, f) == 0)) {
    fclose(f);
    return 0;
  }

  if (data->H == 0 || data->W == 0) {
    printf("Not correct size matrix\n");
    fclose(f);
    return 0;
  }


  uint64_t s_matrix = (uint64_t)data->H * (uint64_t)data->W;
  if (s_matrix > SIZE_MAX) {
    fclose(f);
    freeData(data);
    return 0;
  }

  size_t size_matrix = (size_t)s_matrix;

  data->A = (uint8_t * )malloc(size_matrix);
  data->B = (uint8_t * )malloc(size_matrix);
  data->C = (uint8_t * )malloc(size_matrix);

  if (data->A == NULL || data->B == NULL || data->C == NULL) {
    fclose(f);
    freeData(data);
    printf("Cannot allocate memory\n");
    return 0;
  }


  uint8_t buffer[3];
  for (size_t i = 0; i < size_matrix; ++i) {
    if (fread(buffer, sizeof(uint8_t), 3, f) != 3) {
      fclose(f);
      freeData(data);
      return 0;
    }
    data->A[i] = buffer[0];
    data->B[i] = buffer[1];
    data->C[i] = buffer[2];
  }

  if (fread(&data->DH, sizeof(uint16_t), 1, f) != 1 ||
    fread(&data->DW, sizeof(uint16_t), 1, f) != 1) {
    fclose(f);
    freeData(data);
    return 0;
  }

  if (data->DH == 0 || data->DW == 0) {
    printf("Not correct size matrix D");
    fclose(f);
    freeData(data);
    return 0;
  }

  uint64_t s_matrix_d = (uint64_t)data->DH * (uint64_t)data->DW;
  if (s_matrix_d > SIZE_MAX) {
    fclose(f);
    freeData(data);
    return 0;
  }

  size_t size_matrix_d = (size_t)s_matrix_d;

  data->D = (int8_t * )malloc(size_matrix_d);

  if (data->D == NULL) {
    printf("Cannot allocate memory for D\n");
    freeData(data);
    fclose(f);
    return 0;
  }

  if (fread(data->D, sizeof(int8_t), size_matrix_d, f) != size_matrix_d) {
    freeData(data);
    fclose(f);
    printf("Error read matrix D data\n");
    return 0;
  }

  fclose(f);
  return 1;
}


static inline uint8_t apply_rules(int64_t val) {
  if (val > 255) {
    return (uint8_t)(val % 251);
  } else if (val < 0) {
    int64_t pos_val = -(uint64_t)val;
    return (uint8_t)(pos_val % 241);
  }
  return (uint8_t)val;
}

uint8_t * applyConvolution(const uint8_t * src, uint32_t H, uint32_t W, const int8_t * D, uint16_t DH, uint16_t DW) {
  size_t total_size = (size_t)H * (size_t)W;

  uint8_t * dst = (uint8_t *)malloc(total_size);
  if (dst == NULL) {
    return NULL;
  }

  int32_t padH = (int32_t)DH / 2;
  int32_t padW = (int32_t)DW / 2;

  for (uint32_t r = 0; r < H; ++r) {
    for (uint32_t c = 0; c < W; ++c) {
      int64_t sum = 0;

      for (uint16_t dr = 0; dr < DH; ++dr) {
        for (uint16_t dc = 0; dc < DW; ++dc) {
          int64_t nr = (int64_t)r + dr - padH;
          int64_t nc = (int64_t)c + dc - padW;

          uint8_t val = 0;
          if (nr >= 0 && nr < (int64_t)H && nc >= 0 && nc < (int64_t)W) {
            val = src[nr * (size_t)W + nc];
          }

          int8_t kernel_val = D[dr * (size_t)DW + dc];
          sum += (int64_t)val * kernel_val;
        }
      }

      dst[r * (size_t)W + c] = apply_rules(sum);
    }
  }

  return dst;
}

int writeFile(const char * filename, const Data * data, const uint8_t * matA, const uint8_t * matB, const uint8_t * matC) {
  FILE * f = fopen(filename, "wb");
  if (f == NULL) {
    return 0;
  }

  if (fwrite(&data->H, sizeof(uint32_t), 1, f) != 1 ||
    fwrite(&data->W, sizeof(uint32_t), 1, f) != 1) {
    fclose(f);
    return 0;
  }

  size_t total_size = (size_t)data->H * (size_t)data->W;
  for (size_t i = 0; i < total_size; ++i) {
    if (fwrite(&matA[i], sizeof(uint8_t), 1, f) != 1 ||
      fwrite(&matB[i], sizeof(uint8_t), 1, f) != 1 ||
      fwrite(&matC[i], sizeof(uint8_t), 1, f) != 1) {
      fclose(f);
      return 0;
    }
  }

    fclose(f);
    return 1;
}

int main (int argc, char * argv[]) {
  const char * input_file = NULL;
  const char * output_file = NULL;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-i") == 0) {
      if (i + 1 >= argc || input_file != NULL) {
        return 10;
      }
      input_file = argv[++i];
    } else if (strcmp(argv[i], "-o") == 0) {
      if (i + 1 >= argc || output_file != NULL) {
        return 10;
      }
      output_file = argv[++i];
    } else {
      return 10;
    }
  }

  if (input_file == NULL || output_file == NULL) {
    return 10;
  }

  Data data = {0};
  if (readFile(input_file, &data) == 0) {
    return 1;
  }

  uint8_t * matA = applyConvolution(data.A, data.H, data.W, data.D, data.DH, data.DW);
  uint8_t * matB = applyConvolution(data.B, data.H, data.W, data.D, data.DH, data.DW);
  uint8_t * matC = applyConvolution(data.C, data.H, data.W, data.D, data.DH, data.DW);

  if (matA == NULL || matB == NULL || matC == NULL) {
    free(matA);
    free(matB);
    free(matC);
    freeData(&data);
    return 1;
  }

  int end_program = writeFile(output_file, &data, matA, matB, matC);

  free(matA);
  free(matB);
  free(matC);
  freeData(&data);

  return end_program ? 0 : 1;
}