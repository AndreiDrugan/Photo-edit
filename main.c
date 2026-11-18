#define BMP_IO_RGB_H
#define BMP_IO_RGB_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>

#define BMP_GET32(p) ((uint32_t)(p)[0] | (uint32_t)(p)[1]<<8 | (uint32_t)(p)[2]<<16 | (uint32_t)(p)[3]<<24)
#define BMP_PUT32(p,v) do { uint32_t bmp_v_ = (uint32_t)(v); for(int bmp_k_=0; bmp_k_<4; ++bmp_k_) (p)[bmp_k_] = (unsigned char)(bmp_v_ >> (8*bmp_k_)); } while(0)

static inline unsigned char ***bmp_read(const char *path, int *N, int *M)
{
    if (!N || !M || N == M) return NULL;
    *N = *M = 0; if (!path) return NULL;
    FILE *f = fopen(path, "rb"); if (!f) return NULL;
    unsigned char h[54], *buf=NULL, ***img=NULL; int nr=0, nc=0;
    if (fread(h, 1, 54, f) != 54 || h[0]!='B' || h[1]!='M') goto fail;
    uint32_t off=BMP_GET32(h+10), dib=BMP_GET32(h+14), w=BMP_GET32(h+18), sh=BMP_GET32(h+22);
    int top = (int)(sh >> 31), b = h[28]/8; uint32_t n = top ? (uint32_t)(0u-sh) : sh;
    if ((dib!=40 && dib!=52 && dib!=56 && dib!=108 && dib!=124) || (uint64_t)off<14ull+dib ||
        h[26]!=1 || h[27] || (h[28]!=24 && h[28]!=32) || h[29] || BMP_GET32(h+30) ||
        !w || w>INT32_MAX || w>INT_MAX || !n || n>INT32_MAX || n>INT_MAX) goto fail;
    uint64_t stride = ((uint64_t)w*b+3)/4*4;
    if (stride>SIZE_MAX || (uint64_t)n*sizeof(*img)>SIZE_MAX || (uint64_t)w*sizeof(**img)>SIZE_MAX) goto fail;
    if (fseek(f, 0, SEEK_END)) goto fail;
    long len = ftell(f);
    if (len<0 || (uint64_t)len<(uint64_t)off+stride*n || fseek(f, (long)off, SEEK_SET)) goto fail;
    img=malloc((size_t)n*sizeof(*img)); buf=malloc((size_t)stride); if (!img || !buf) goto fail;
    for (uint32_t y=0; y<n; ++y) {
        img[y]=malloc((size_t)w*sizeof(**img)); if (!img[y]) goto fail; ++nr; nc=0;
        for (uint32_t x=0; x<w; ++x) { img[y][x]=malloc(3); if (!img[y][x]) goto fail; ++nc; }
    }
    for (uint32_t y=0; y<n; ++y) {
        if (fread(buf, 1, (size_t)stride, f)!=(size_t)stride) goto fail;
        for (size_t x=0; x<w; ++x)
            for (int k=0; k<3; ++k) img[top?y:n-1-y][x][k]=buf[b*x+2-k];
    }
    free(buf); fclose(f); *N=(int)n; *M=(int)w; return img;
fail:
    if (img) { for (int y=0; y<nr; ++y) { for (uint32_t x=0; x<(y==nr-1?(uint32_t)nc:w); ++x) free(img[y][x]); free(img[y]); } free(img); }
    free(buf); fclose(f); return NULL;
}


static inline int bmp_write(const char *path, unsigned char ***img, int N, int M)
{
    if (!path || !img || N<=0 || M<=0) return 0;
    if ((uint64_t)N>INT32_MAX || (uint64_t)M>INT32_MAX) return 0;
    uint64_t stride=((uint64_t)M*3+3)/4*4, bytes=stride*(uint64_t)N;
    if (stride>SIZE_MAX || bytes>UINT32_MAX-54u) return 0;
    for (int y=0; y<N; ++y) { if (!img[y]) return 0; for (int x=0; x<M; ++x) if (!img[y][x]) return 0; }
    unsigned char h[54]={0}, *buf=calloc(1,(size_t)stride); if (!buf) return 0;
    h[0]='B'; h[1]='M'; h[26]=1; h[28]=24;
    BMP_PUT32(h+2,54+bytes); BMP_PUT32(h+10,54); BMP_PUT32(h+14,40);
    BMP_PUT32(h+18,M); BMP_PUT32(h+22,N); BMP_PUT32(h+34,bytes);
    FILE *f=fopen(path,"wb"); if (!f) { free(buf); return 0; }
    int ok=fwrite(h,1,54,f)==54;
    for (int y=N; ok && y-->0;) {
        for (size_t x=0; x<(size_t)M; ++x)
            for (int k=0; k<3; ++k) buf[3*x+2-k]=img[y][x][k];
        ok=fwrite(buf,1,(size_t)stride,f)==(size_t)stride;
    }
    if (fclose(f)!=0) ok=0;
    free(buf); return ok;
}

typedef struct {
  char name[100];
  unsigned char ***img;
  int N;
  int M;
} Image;

void free_image(unsigned char ***image, int N, int M)
{
  if(!image)
    return;
  for(int i=0; i<N; i++)
  {
    for(int j=0; j<M; j++)
      free(image[i][j]);
    free(image[i]);
  }
  free(image);
}

static inline unsigned char ***bounding_box(unsigned char ***img, int N, int M, int *x1, int *y1, int *x2, int *y2, int R, int G, int B)
{
    int min_x = (*x1 < *x2) ? *x1 : *x2;
    int max_x = (*x1 > *x2) ? *x1 : *x2;
    int min_y = (*y1 < *y2) ? *y1 : *y2;
    int max_y = (*y1 > *y2) ? *y1 : *y2;

    if (min_x < 0) min_x = 0;
    if (max_x >= M) max_x = M - 1;
    if (min_y < 0) min_y = 0;
    if (max_y >= N) max_y = N - 1;

    for (int x = min_x; x <= max_x; x++)
    {
        img[min_y][x][0] = R;
        img[min_y][x][1] = G;
        img[min_y][x][2] = B;

        img[max_y][x][0] = R;
        img[max_y][x][1] = G;
        img[max_y][x][2] = B;
    }

    for (int y = min_y; y <= max_y; y++)
    {
        img[y][min_x][0] = R;
        img[y][min_x][1] = G;
        img[y][min_x][2] = B;

        img[y][max_x][0] = R;
        img[y][max_x][1] = G;
        img[y][max_x][2] = B;
    }

    return img;
}

static inline unsigned char ***crop(unsigned char ***img, int *N, int *M, int *x1, int *y1, int *x2, int *y2)
{
    int min_x = (*x1 < *x2) ? *x1 : *x2;
    int max_x = (*x1 > *x2) ? *x1 : *x2;
    int min_y = (*y1 < *y2) ? *y1 : *y2;
    int max_y = (*y1 > *y2) ? *y1 : *y2;

    int old_N = *N;
    int old_M = *M;

    if (min_x < 0) min_x = 0;
    if (max_x >= old_M) max_x = old_M - 1;
    if (min_y < 0) min_y = 0;
    if (max_y >= old_N) max_y = old_N - 1;

    int new_N = max_y - min_y + 1;
    int new_M = max_x - min_x + 1;

    unsigned char ***cropped_img = malloc(new_N * sizeof(unsigned char **));
    for (int y = 0; y < new_N; y++)
    {
        cropped_img[y] = malloc(new_M * sizeof(unsigned char *));
        for (int x = 0; x < new_M; x++)
        {
            cropped_img[y][x] = malloc(3 * sizeof(unsigned char));
            cropped_img[y][x][0] = img[min_y + y][min_x + x][0];
            cropped_img[y][x][1] = img[min_y + y][min_x + x][1];
            cropped_img[y][x][2] = img[min_y + y][min_x + x][2];
        }
    }

    for (int y = 0; y < old_N; y++) {
        for (int x = 0; x < old_M; x++) {
            free(img[y][x]);
        }
        free(img[y]);
    }
    free(img);


    *N = new_N;
    *M = new_M;
    return cropped_img;
}

static inline unsigned char ***filter(unsigned char ***img, int N, int M)
{
  for(int i=0; i<N; i++)
    for(int j=0; j<M; j++)
      {
        int medie = (img[i][j][0] + img[i][j][1] + img[i][j][2]) / 3;
        img[i][j][0] = medie;
        img[i][j][1] = medie;
        img[i][j][2] = medie;
      }
  return img;
}

static inline unsigned char ***ig_resolution(unsigned char ***img, int N, int M, int dif, char poz)
{
  if(poz == 'N')
  {
    for(int i=0; i<dif/2; i++)
      for(int j=0; j<M; j++)
      {
        img[i][j][0] = 255;
        img[i][j][1] = 255;
        img[i][j][2] = 255;
      }
    for(int i=N-dif/2; i<N; i++)
      for(int j=0; j<M; j++)
      {
        img[i][j][0] = 255;
        img[i][j][1] = 255;
        img[i][j][2] = 255;
      }
  }
  else
  {
    for(int i=0; i<N; i++)
      for(int j=0; j<dif/2; j++)
      {
        img[i][j][0] = 255;
        img[i][j][1] = 255;
        img[i][j][2] = 255;
      }
    for(int i=0; i<N; i++)
      for(int j=M-dif/2; j<M; j++)
      {
        img[i][j][0] = 255;
        img[i][j][1] = 255;
        img[i][j][2] = 255;
      }
  }
  return img;
}

int paste_image(unsigned char ***base_img, int N_base, int M_base, unsigned char ***paste_img, int N_paste, int M_paste, int x, int y)
{
  int start_i = (x > 0) ? x : 0;
  int start_j = (y > 0) ? y : 0;
  int end_i = (N_base < x + N_paste) ? N_base : (x + N_paste);
  int end_j = (M_base < y + M_paste) ? M_base : (y + M_paste);
  for(int i=start_i; i<end_i; i++)
    for(int j=start_j; j<end_j; j++)
    {
      base_img[i][j][0] = paste_img[i-x][j-y][0];
      base_img[i][j][1] = paste_img[i-x][j-y][1];
      base_img[i][j][2] = paste_img[i-x][j-y][2];
    }
  return 1;
}

unsigned char ***create_image(int N, int M)
{
  unsigned char ***img = malloc(N*sizeof(unsigned char **));
  for(int i=0; i<N; i++)
  {
    img[i] = malloc(M*sizeof(unsigned char *));
    for(int j=0; j<M; j++)
      img[i][j] = malloc(3*sizeof(unsigned char));
  }
  return img;
}

void clear_image(unsigned char ***img, int N, int M, int R, int G, int B)
{
  for(int i=0; i<N; i++)
    for(int j=0; j<M; j++)
    {
      img[i][j][0] = R;
      img[i][j][1] = G;
      img[i][j][2] = B;
    }
}

int find_image(Image *v, int ct, char *name)
{
  for(int i=0; i<ct; i++)
    if(strcmp(v[i].name, name) == 0)
      return i;
  return -1;
}

void load_image(Image *v, int *ct, char *name, unsigned char ***img, int N, int M)
{
  int idx = find_image(v, *ct, name);
  if(idx != -1)
  {
    free_image(v[idx].img, v[idx].N, v[idx].M);
    v[idx].img = img;
    v[idx].N = N;
    v[idx].M = M;
  }
  else
  {
    strcpy(v[*ct].name, name);
    v[*ct].img = img;
    v[*ct].N = N;
    v[*ct].M = M;
    (*ct)++;
  }
}

int rle_encode(unsigned char ***img, int N, int M, char *output_path)
{
  if(!output_path || !img || N <= 0 || M <= 0)
    return 0;
  FILE *f = fopen(output_path, "wb");
  if(!f)
    return 0;
  int current_r = img[0][0][0];
  int current_g = img[0][0][1];
  int current_b = img[0][0][2];
  int ct=0;

  for(int i=0; i<N; i++)
  {
    for(int j=0; j<M; j++)
      {
        if(img[i][j][0] == current_r && img[i][j][1] == current_g && img[i][j][2] == current_b)
          ct++;
        else
        {
          fwrite(&ct, sizeof(int), 1, f);
          fwrite(&current_r, sizeof(unsigned char), 1, f);
          fwrite(&current_g, sizeof(unsigned char), 1, f);
          fwrite(&current_b, sizeof(unsigned char), 1, f);
          current_r = img[i][j][0];
          current_g = img[i][j][1];
          current_b = img[i][j][2];
          ct = 1;
        }
      }
  }
  fclose(f);
  return 1;
}

unsigned char ***rle_decode(char *input_path, int *N, int *M)
{
  if(!input_path || !N || !M)
    return NULL;
  FILE *f = fopen(input_path, "rb");
  if(!f)
    return NULL;
  int total = (*N) * (*M);
  unsigned char ***img = malloc((*N) * sizeof(unsigned char **));
  int ct=0;
  int filled=0;
  unsigned char rgb[3];
  while(filled < total && fread(rgb, sizeof(unsigned char), 3, f) == 3 && fread(&ct, sizeof(int), 1, f) == 1)
  {
    for(int i=0; i<ct && filled < total; i++)
    {
      int poz_i = filled / (*M);
      int poz_j = filled % (*M);
      img[poz_i][poz_j][0] = rgb[0];
      img[poz_i][poz_j][1] = rgb[1];
      img[poz_i][poz_j][2] = rgb[2];
      filled++;
    }
  }
  fclose(f);
  return img;
}

int main()
{
  int N, M;
  unsigned char ***img = bmp_read("alpaca.bmp", &N, &M);
  if(!img)
    return 1;
  int op;
  printf("Enter the operation:\n");
  scanf("%d", &op);

  if(op==1)
  {
    int x1, y1, x2, y2;
    scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
    img = bounding_box(img, N, M, &x1, &y1, &x2, &y2, 255, 0, 0);
    bmp_write("output.bmp", img, N, M);
  }
  else if(op==2)
  {
    int x1, y1, x2, y2;
    scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
    img = crop(img, &N, &M, &x1, &y1, &x2, &y2);
    bmp_write("output.bmp", img, N, M);
  }
  else if(op==3)
  {
    img = filter(img, N, M);
    bmp_write("output.bmp", img, N, M);
  }
  else if(op==4)
  {
    if(N==M)
      bmp_write("output.bmp", img, N, M);
    else if(N>M)
    {
      int dif = N-M;
      img = ig_resolution(img, N, M, dif, 'N');
      bmp_write("output.bmp", img, N, M);
    }
    else
    {
      int dif = M-N;
      img = ig_resolution(img, N, M, dif, 'M');
      bmp_write("output.bmp", img, N, M);
    }
  }
  else if(op==5)
  {
    int N_paste, M_paste;
    unsigned char ***paste_img = bmp_read("images.bmp", &N_paste, &M_paste);
    if (!paste_img)
    {
      printf("Error reading the image to paste!\n");
      return 1;
    }
    int x, y;
    scanf("%d %d", &x, &y);
    paste_image(img, N, M, paste_img, N_paste, M_paste, x, y);
    bmp_write("output.bmp", img, N, M);
  }
  else if(op==6)
  {
    FILE *f = fopen("command.txt", "r");
    if(!f)
    {
      printf("Error opening command.txt\n");
      return 1;
    }
    Image images[100];
    int ct=0;
    char command[20];
    while(fscanf(f, "%s", command) == 1)
    {
      if(strcmp(command, "CREATE") == 0)
      {
        char tag[100];
        int N_create, M_create;
        fscanf(f, "%s %d %d", tag, &M_create, &N_create);
        unsigned char ***new_img = create_image(N_create, M_create);
        load_image(images, &ct, tag, new_img, N_create, M_create);
      }
      else if(strcmp(command, "CLEAR") == 0)
      {
        char tag[100];
        int R, G, B;
        fscanf(f, "%s %d %d %d", tag, &R, &G, &B);
        int idx = find_image(images, ct, tag);
        if(idx != -1)
          clear_image(images[idx].img, images[idx].N, images[idx].M, R, G, B);
      }
      else if(strcmp(command, "LOAD") == 0)
      {
        char tag[100], path[100];
        fscanf(f, "%s %s", tag, path);
        int N_load, M_load;
        unsigned char ***loaded_img = bmp_read(path, &N_load, &M_load);
        if(loaded_img)
          load_image(images, &ct, tag, loaded_img, N_load, M_load);
      }
      else if(strcmp(command, "PASTE") == 0)
      {
        char tag_base[100], tag_paste[100];
        int x, y;
        fscanf(f, "%s %s %d %d", tag_base, tag_paste, &x, &y);
        int dest = find_image(images, ct, tag_base);
        int src = find_image(images, ct, tag_paste);
        if(dest != -1 && src != -1)
          paste_image(images[dest].img, images[dest].N, images[dest].M, images[src].img, images[src].N, images[src].M, y, x);
      }
      else if(strcmp(command, "RECTANGLE") == 0)
      {
        char tag[100];
        int x1, y1, x2, y2, R, G, B;
        fscanf(f, "%s %d %d %d %d %d %d %d", tag, &x1, &y1, &x2, &y2, &R, &G, &B);
        int idx = find_image(images, ct, tag);
        if(idx != -1)
          images[idx].img = bounding_box(images[idx].img, images[idx].N, images[idx].M, &x1, &y1, &x2, &y2, R, G, B);
      }
      else if(strcmp(command, "CROP") == 0)
      {
         char tag[100];
         int x1, y1, x2, y2;
         fscanf(f, "%s %d %d %d %d", tag, &x1, &y1, &x2, &y2);
        int idx = find_image(images, ct, tag);
        if(idx != -1)
          images[idx].img = crop(images[idx].img, &images[idx].N, &images[idx].M, &x1, &y1, &x2, &y2);
      }
      else if(strcmp(command, "GRAYSCALE") == 0)
      {
        char tag[100];
        fscanf(f, "%s", tag);
        int idx = find_image(images, ct, tag);
        if(idx != -1)
          images[idx].img = filter(images[idx].img, images[idx].N, images[idx].M);
      }
      else if(strcmp(command, "SQUARE") == 0)
      {
        char tag[100];
        int idx = find_image(images, ct, tag);
        if(idx != -1)
          images[idx].img = ig_resolution(images[idx].img, images[idx].N, images[idx].M, abs(images[idx].N - images[idx].M), (images[idx].N > images[idx].M) ? 'N' : 'M');
      }
      else if(strcmp(command, "SAVE") == 0)
      {
        char tag[100], path[100];
        fscanf(f, "%s %s", tag, path);
        int idx = find_image(images, ct, tag);
        if(idx != -1)
          bmp_write(path, images[idx].img, images[idx].N, images[idx].M);
      }
      else if(strcmp(command, "ENCODE") == 0)
      {
        char tag[100], path[100];
        fscanf(f, "%s %s", tag, path);
        int idx = find_image(images, ct, tag);
        if(idx != -1)
          rle_encode(images[idx].img, images[idx].N, images[idx].M, path);
      }
      else if(strcmp(command, "DECODE") == 0)
      {
        char tag[100], path[100];
        int N_decode, M_decode;
        fscanf(f, "%s %s %d %d", tag, path, &M_decode, &N_decode);
        unsigned char ***decoded_img = rle_decode(path, &N_decode, &M_decode);
        if(decoded_img)
          load_image(images, &ct, tag, decoded_img, N_decode, M_decode);
      }
    }
    for(int i=0; i<ct; i++)
    {
      char output[150];
      snprintf(output, sizeof(output), "%s.bmp", images[i].name);
      bmp_write(output, images[i].img, images[i].N, images[i].M);
      free_image(images[i].img, images[i].N, images[i].M);
    }
    fclose(f);
  }
  else
  {
    printf("Invalid operation\n");
  }
  return 0;
}