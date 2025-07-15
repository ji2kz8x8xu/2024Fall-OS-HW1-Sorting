# 2024Fall-OS-HW1-Sorting

## Description

This project implements Assignment 1: performance comparison of four sorting methods (Method 1–4). The program will:

1. Prompt the user to enter the input file name (without the `.txt` extension).
2. Load non-negative integers (one per line) from `<FileName>.txt`.
3. Display a menu to choose a sorting method (1–4).
4. For Methods 2–4, prompt for the number of partitions `K`.
5. Execute the chosen method, measure CPU time, and verify the result.
6. Write sorted data and performance metrics to `<FileName>_output<MethodNumber>.txt`.

## Sorting Methods

**Method 1: Pure Bubble Sort**

A basic O(n²) algorithm that repeatedly steps through the list, compares adjacent elements, and swaps them if they are in the wrong order. It requires no extra memory but is inefficient for large data sets.

**Method 2: Bubble Merge Sort**

First divides the data into K partitions, applies bubble sort on each partition, then merges sorted partitions pairwise until fully sorted. This reduces individual partition size, improving performance over pure bubble sort.

**Method 3: Bubble Merge Sort with Multiprocessing**

Extends Method 2 by using multiple processes: each partition is sorted in a separate process, taking advantage of multiple CPU cores. After local sorts, processes coordinate to merge partitions, reducing wall-clock time.

**Method 4: Bubble Merge Sort with Multithreading**

Similar to Method 3 but uses threads within a single process. Threads sort partitions in parallel and share memory space, which can reduce inter-process communication overhead but may suffer from synchronization costs.

## Usage

1. **Development Environment**: Windows Subsystem for Linux (WSL2)

   * **Linux Subsystem**: Ubuntu 22.04.3 LTS

2. **Compiler**: g++ (Ubuntu 11.3.0-1ubuntu1\~22.04) 11.3.0

3. **Compile**

   ```bash
   g++ -std=c++17 -O2 main.cpp -o main
   ```

4. **Run**

   ```bash
   ./main
   ```

   Then follow the interactive prompts:

   ```text
   Input file name: input_10w
   ********           Sort           ********
   * 1.bubble sort                          *
   * 2.bubble merge sort                    *
   * 3.bubble merge sort with multiprocess  *
   * 4.bubble merge sort with multithread   *
   ******************************************
   Input a command (1, 2, 3, 4): 2
   Enter the number of partitions (1~1000000): 5
   writing...
   Complete!
   ```

## I/O Format

* **Input files** (placed at test_data file):

  * `input_1w`   (10,000 integers)
  * `input_10w`  (100,000 integers)
  * `input_50w`  (500,000 integers)
  * `input_100w` (1,000,000 integers)
  * Each file: one non-negative integer per line, no header.

* **Output file**:

  * Name: `<InputFile>_output<MethodNumber>.txt` (e.g., `input_10w_output2.txt`)
  * Contents:

    ```text
    <sorted numbers...>
    CPU Time: <time> ms
    Output Time: YYYY-MM-DD HH:MM:SS.<mmm>+08:00
    ```

## Example Session

```bash
$ ./main
Input file name: input_10w
********           Sort           ********
* 1.bubble sort                          *
* 2.bubble merge sort                    *
* 3.bubble merge sort with multiprocess  *
* 4.bubble merge sort with multithread   *
******************************************
Input a command (1, 2, 3, 4): 2
Enter the number of partitions (1~1000000): 5
writing...
Complete!
```

Check `input_10w_output2.txt`:

```text
12
34
56
...
99999
CPU Time: 1234.567890 ms
Output Time: 2024-11-21 15:30:45.123+08:00
```

## Chart Analysis


<img src="assets/diff_method.png" width="800"/>

### Figure 1. Execution Time vs Data Size


  As the data size increases from 10,000 to 1,000,000 elements, Method 1 (pure Bubble Sort) shows a rapid, non-linear growth in execution time due to its O(n²) complexity. Method 2 (Bubble Merge Sort) improves performance significantly for smaller sizes but still degrades as chunks grow larger. Methods 3 and 4 (multiprocessing and multithreading) maintain much lower execution times across all sizes, with multiprocessing slightly outperforming multithreading at larger scales, likely due to reduced contention in shared memory accesses.
  

<img src="assets/diff_k.png" width="725"/>

### Figure 2. Execution Time vs Partition Count (N = 500,000)

#### Since method 1 cannot adjust the k value, only methods 2, 3, and 4 are discussed.
**Method 2: Bubble Merge Sort**
  When $k$ is small, execution time is high because fewer partitions mean each bubble sort handles a larger data chunk, resulting in longer runs. As $k$ increases, partition sizes shrink and execution time decreases accordingly.

**Method 3: Bubble Merge Sort with Multiprocessing**
  The fastest execution occurs when $k$ is between 250 and 1000: this balances the overhead of spawning processes against parallel gains. Beyond this range, the cost of managing too many processes outweighs benefits, and execution time grows.

**Method 4: Bubble Merge Sort with Multithreading**
  Once $k$ exceeds around 50, execution time drops sharply and remains low even as $k$ grows further, with only slight increases due to thread synchronization and memory contention.


## Development Environment

* **Platform**: Windows 10 + WSL2
* **Linux Subsystem**: Ubuntu 22.04.3 LTS
* **Compiler**: g++ (Ubuntu 11.3.0-1ubuntu1\~22.04) 11.3.0

---

*Ensure the input file exists as `<FileName>.txt` before running. The program will append `.txt` internally.*
