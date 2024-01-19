
#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "extern/ubench.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "extern/stb_image_write.h"

#define IMG_WIDTH   1024
#define IMG_HEIGHT  1024
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
        for (size_t x = 0; x < IMG_WIDTH - 3; x += 4)
        {

            /* Calculate pixels corresponding complex values */
            __m128 cx = _mm_set_ps1((float)x);
            cx = _mm_add_ps(cx, _mm_set_ps(0.0f, 1.0f, 2.0f, 3.0f));
            cx = _mm_div_ps(cx, _mm_set_ps1((float)IMG_WIDTH));
            cx = _mm_sub_ps(cx, _mm_set_ps1(.5f));
            cx = _mm_mul_ps(cx, _mm_set_ps1(VIEW));
            cx = _mm_add_ps(cx, _mm_set_ps1(CENTER_X));

            __m128 cy = _mm_set_ps1((float)y);
            cy = _mm_div_ps(cy, _mm_set_ps1((float)IMG_HEIGHT));
            cy = _mm_sub_ps(cy, _mm_set_ps1(.5f));
            cy = _mm_mul_ps(cy, _mm_set_ps1(VIEW));
            cy = _mm_add_ps(cy, _mm_set_ps1(CENTER_Y));

            __m128 zx  = _mm_set_ps1(0.0f);
            __m128 zy  = _mm_set_ps1(0.0f);

            __m128i acc = _mm_set1_epi32(0);
            __m128  bnd = (__m128)_mm_set1_epi32(0);

            size_t i = 256;

            while (--i > 0)
            {
                /* Squared parts */
                __m128 zx2 = _mm_mul_ps(zx, zx);
                __m128 zy2 = _mm_mul_ps(zy, zy);

                /* Determine which values are inside bounds */
                bnd = _mm_cmp_ps(_mm_add_ps(zx2, zy2), _mm_set_ps1(BOUND), _CMP_LT_OQ);

                /* Accumulate pixels inside bounds */
                acc = _mm_add_epi32(acc, (__m128i)_mm_and_ps(bnd, (__m128)_mm_set1_epi32(1)));

                /* Break if none are inside bounds */
                int all_outside = _mm_test_all_zeros(_mm_set1_epi32(0xffffffff), (__m128i)bnd);
                if (all_outside)
                {
                    break;
                }

                /* Iterate */
                __m128 tmp = zx;
                zx = _mm_add_ps(_mm_sub_ps(zx2, zy2), cx);
                zy = _mm_add_ps(_mm_mul_ps(_mm_set_ps1(2.0f), _mm_mul_ps(tmp, zy)), cy);
            }

            /* I can't use _mm_cvtepi32_epi8 :(  */
            img[x + 0 + (IMG_WIDTH * y)] = ((int*)&acc)[3];
            img[x + 1 + (IMG_WIDTH * y)] = ((int*)&acc)[2];
            img[x + 2 + (IMG_WIDTH * y)] = ((int*)&acc)[1];
            img[x + 3 + (IMG_WIDTH * y)] = ((int*)&acc)[0];
        }
    }
}

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
