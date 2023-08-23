# SHM-BENCH: Benchmark Suite for System V Shared Memory

Welcome to SHM-BENCH, a comprehensive benchmark suite designed to evaluate System V Shared Memory performance.

## Workloads

The suite encompasses the following workloads:

1) **CAP (Contention Access Pattern):** CAP assesses applications that are susceptible to thrashing. It simulates situations where certain memory areas are accessed simultaneously by multiple processes.

2) **EAP (Exclusive Access Pattern):** EAP measures the performance of exclusive access to shared memory. This is achieved by creating shared memory-based locks.

3) **DAP (Dynamic Access Pattern):** DAP is designed to evaluate the performance of random access to shared memory.

4) **RAP (Regular Access Pattern):** RAP employs a modified version of SPLASH that is compatible with System V IPC. In this version, the FFT, LU, and RADIX kernels are supported (with partial support for RADIX).

## Version

The current version of SHM-BENCH is 0.1.

## License

SHM-BENCH is distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT). For more information, please refer to the `LICENSE` file in this repository.
