# SIMD Mandelbrot
## Usage:
- `make run`: Run SIMD version of the program.
- `make benchmark`: Run benchmarks to compare to version without SIMD.
## Results:
```
mandelbrot.no_simd (mean 197.662ms, confidence interval +- 0.048377%)
mandelbrot.simd (mean 53.057ms, confidence interval +- 0.180017%)
```
![Example](mandelbrot.png)
