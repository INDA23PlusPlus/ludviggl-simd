# SIMD Mandelbrot
## Usage:
- `make run`: Run SIMD version of the program.
- `make benchmark`: Run benchmarks to compare to version without SIMD.
## Results:
```
mandelbrot.no_simd (mean 9.671s, confidence interval +- 0.061074%)
mandelbrot.simd (mean 1.633s, confidence interval +- 0.133225%)
```
![Example 1](images/mandelbrot.png)
![Example 2](images/mandelbrot1.png)
