#define BMP_IO_RGB_H
#define BMP_IO_RGB_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

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

static inline unsigned char ***bounding_box(unsigned char ***img, int N, int M, int *x1, int *y1, int *x2, int *y2)
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
        img[min_y][x][0] = 255;
        img[min_y][x][1] = 0;
        img[min_y][x][2] = 0;

        img[max_y][x][0] = 255;
        img[max_y][x][1] = 0;
        img[max_y][x][2] = 0;
    }

    for (int y = min_y; y <= max_y; y++)
    {
        img[y][min_x][0] = 255;
        img[y][min_x][1] = 0;
        img[y][min_x][2] = 0;

        img[y][max_x][0] = 255;
        img[y][max_x][1] = 0;
        img[y][max_x][2] = 0;
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
    img = bounding_box(img, N, M, &x1, &y1, &x2, &y2);
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
  else
  {
    printf("Invalid operation\n");
  }
  return 0;
}