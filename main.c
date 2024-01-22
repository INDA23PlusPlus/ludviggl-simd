
#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "extern/ubench.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "extern/stb_image_write.h"

#define IMG_WIDTH   2048
#define IMG_HEIGHT  2048
#define CENTER_X   -0.75f
#define CENTER_Y    0.0f
#define VIEW        3.0f
#define BOUND       20.0f

#define IMG_SIZE (IMG_WIDTH * IMG_HEIGHT)

void mandelbrot_no_simd(unsigned char *img)
{
    for (size_t y = 0; y < IMG_HEIGHT; y++)
    {
        for (size_t x = 0; x < IMG_WIDTH; x++)
        {
            float cx = VIEW * ((float)x / (float)IMG_WIDTH - 0.5f) + CENTER_X;
            float cy = VIEW * ((float)y / (float)IMG_HEIGHT - 0.5f) + CENTER_Y;

            float zx = 0.0f;
            float zy = 0.0f;

            size_t i = 256;
            while (--i > 0)
            {
                float tmp = zx;
                zx = zx * zx - zy * zy + cx;
                zy = 2.0f * tmp * zy + cy;

                if (zx * zx + zy * zy > BOUND)
                {
                    break;
                }
            }

            img[x + (IMG_WIDTH * y)] = 255 - i;
        }
    }
}

void mandelbrot_simd(unsigned char *img)
{
    for (size_t y = 0; y < IMG_HEIGHT; y++)
    {
        for (size_t x = 0; x < IMG_WIDTH - 7; x += 8)
        {

            /* Calculate pixels corresponding complex values */
            __m256 cx = _mm256_set1_ps((float)x);
            cx = _mm256_add_ps(cx, _mm256_set_ps(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f));
            cx = _mm256_div_ps(cx, _mm256_set1_ps((float)IMG_WIDTH));
            cx = _mm256_sub_ps(cx, _mm256_set1_ps(.5f));
            cx = _mm256_mul_ps(cx, _mm256_set1_ps(VIEW));
            cx = _mm256_add_ps(cx, _mm256_set1_ps(CENTER_X));

            __m256 cy = _mm256_set1_ps((float)y);
            cy = _mm256_div_ps(cy, _mm256_set1_ps((float)IMG_HEIGHT));
            cy = _mm256_sub_ps(cy, _mm256_set1_ps(.5f));
            cy = _mm256_mul_ps(cy, _mm256_set1_ps(VIEW));
            cy = _mm256_add_ps(cy, _mm256_set1_ps(CENTER_Y));

            __m256 zx  = _mm256_set1_ps(0.0f);
            __m256 zy  = _mm256_set1_ps(0.0f);

            __m256i acc = _mm256_set1_epi32(0);
            __m256  bnd = (__m256)_mm256_set1_epi32(0);

            size_t i = 256;

            while (--i > 0)
            {
                /* Squared parts */
                __m256 zx2 = _mm256_mul_ps(zx, zx);
                __m256 zy2 = _mm256_mul_ps(zy, zy);

                /* Determine which values are inside bounds */
                bnd = _mm256_cmp_ps(_mm256_add_ps(zx2, zy2), _mm256_set1_ps(BOUND), _CMP_LT_OQ);

                /* Accumulate pixels inside bounds */
                acc = _mm256_add_epi32(acc, (__m256i)_mm256_and_ps(bnd, (__m256)_mm256_set1_epi32(1)));

                /* Break if none are inside bounds */
                int all_outside = _mm256_testz_si256((__m256i)bnd, _mm256_set1_epi32(-1u));
                if (all_outside)
                {
                    break;
                }

                /* Iterate */
                __m256 tmp = zx;
                zx = _mm256_add_ps(_mm256_sub_ps(zx2, zy2), cx);
                zy = _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(2.0f), _mm256_mul_ps(tmp, zy)), cy);
            }

            img[x + 0 + (IMG_WIDTH * y)] = ((int*)&acc)[7];
            img[x + 1 + (IMG_WIDTH * y)] = ((int*)&acc)[6];
            img[x + 2 + (IMG_WIDTH * y)] = ((int*)&acc)[5];
            img[x + 3 + (IMG_WIDTH * y)] = ((int*)&acc)[4];
            img[x + 4 + (IMG_WIDTH * y)] = ((int*)&acc)[3];
            img[x + 5 + (IMG_WIDTH * y)] = ((int*)&acc)[2];
            img[x + 6 + (IMG_WIDTH * y)] = ((int*)&acc)[1];
            img[x + 7 + (IMG_WIDTH * y)] = ((int*)&acc)[0];
        }
    }
}

#ifdef BENCHMARK

UBENCH_EX(mandelbrot, no_simd)
{
    void *img = malloc(IMG_SIZE);
    assert(img);

    UBENCH_DO_BENCHMARK() {
            mandelbrot_no_simd(img);
    }

    int write = stbi_write_png("no_simd.png", IMG_WIDTH, IMG_HEIGHT, 1, img, 0);
    assert(write);

    free(img);
}

UBENCH_EX(mandelbrot, simd)
{
    void *img = malloc(IMG_SIZE);
    assert(img);

    UBENCH_DO_BENCHMARK() {
            mandelbrot_simd(img);
    }

    int write = stbi_write_png("simd.png", IMG_WIDTH, IMG_HEIGHT, 1, img, 0);
    assert(write);

    free(img);
}

UBENCH_MAIN();

#else

int main()
{
    void *img = malloc(IMG_SIZE);
    assert(img);

    printf("creating mandelbrot image...");
    mandelbrot_simd(img);

    printf(" done!\nsaving...");
    int write = stbi_write_png("simd.png", IMG_WIDTH, IMG_HEIGHT, 1, img, 0);
    assert(write);

    printf(" done!\n");

    free(img);
}

#endif
