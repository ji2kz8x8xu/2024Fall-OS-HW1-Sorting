#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>
#include <ctime>
#include <unistd.h> // pid_t, fork()
#include <sys/wait.h> // waitpid()
// shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

class SharedMemory {
 private:
  key_t key;    // 唯一標識符
  int shmid;    // 共享記憶體 ID
  int* data;    // 指向共享記憶體的指標
  int size;     // 記憶體段的大小

 public:
  SharedMemory(int size) : size(size) {
    if (size <= 0) {
      throw std::invalid_argument("Size must be greater than 0.");
    }
    key = IPC_PRIVATE;
    shmid = shmget(key, size * sizeof(int), IPC_CREAT | 0666);
    if (shmid == -1) {
      perror("shmget failed");
       exit(EXIT_FAILURE);
    }
    data = static_cast<int*>(shmat(shmid, nullptr, 0));
    if (data == reinterpret_cast<int*>(-1)) {
      perror("shmat failed");
      exit(EXIT_FAILURE);
    }
  }

  
  ~SharedMemory() {
    if (shmdt(data) == -1) {
      perror("shmdt failed");
    }
    if (shmctl(shmid, IPC_RMID, nullptr) == -1) {
      perror("shmctl failed");
    }
  }

  // 把vector資料設定到shared memory
  void setData(const std::vector<int>& input) {
    if (input.size() > size) {
      throw std::out_of_range("Input size exceeds shared memory size.");
    }
    for (size_t i = 0; i < input.size(); ++i) {
      data[i] = input[i];
    }
  }

  // shared memory的資料用vector回傳
  std::vector<int> getData() const {
    std::vector<int> output(size);
    for (size_t i = 0; i < size; ++i) {
      output[i] = data[i];
    }
    return output;
  }

  // 拿到shared memory的起點
  int* getRawData() const {
    return data;
  }

  int getSize() const {
    return size;
  }
};

void readDataFromFile(std::string fileName, std::vector<int>& data) {
  std::ifstream inputFile(fileName);
  std::string line;
  while (getline(inputFile, line)) {
    data.push_back(stoi(line));
  }
}

bool fileExist(std::string fileName) {
  std::ifstream inputFile(fileName);
  if (!inputFile) {
    std::cout << "file does not exist!" << std::endl << std::endl;
    return false;
  } 
  return true;
}

void printData(std::vector<int>& data) {
  for (auto& ele : data) {
    std::cout << ele << " ";
  }
}

void writeDataToFile(std::vector<int>& data, std::string fileName, double executeTime) {
  std::ofstream outputFile(fileName);
  outputFile << "Sort : " << std::endl;
  for (int i = 0; i < data.size(); i++) {
    outputFile << data.at(i) << std::endl;
  }

  outputFile << "CPU Time : " << std::fixed << std::setprecision(6) << executeTime << std::endl;

  // 取得當前時間
  auto now = std::chrono::system_clock::now();
    
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) % 1000;

  // 寫入時間到檔案
  outputFile << "Output Time : " << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S")
             << '.' << std::setw(3) << std::setfill('0') << milliseconds.count() << "+08:00";
}


// arr[j] 等價於 *(arr + j)
double bubbleSort(int* arr, int start, int end) {
  auto startTime = std::chrono::high_resolution_clock::now();
  for (int i = start; i < end; i++) {
    for (int j = start; j < end - 1 - (i - start); j++) {
      if (arr[j] > arr[j + 1]) {
        std::swap(arr[j], arr[j + 1]);
      }
    }
  }
  auto endTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> durationTime = endTime - startTime;

  // 確認結果
  for (int i = start; i < end - 1; i++) {
    if (arr[i] > arr[i+1]) {
      std::cout << "Sort failed at position " << i << std::endl;
      break;
    }
  }

  return durationTime.count();
}


double merge(int* arr, int* temp, int start1, int end1, int start2, int end2) {
  
  auto startTime = std::chrono::high_resolution_clock::now();

  int i = start1;
  int j = start2;
  int k = start1;

  while (i < end1 && j < end2) {
    if (arr[i] <= arr[j]) {
      temp[k++] = arr[i++];
    } else {
      temp[k++] = arr[j++];
    }
  }

  while (i < end1) temp[k++] = arr[i++];
  while (j < end2) temp[k++] = arr[j++];

  for (i = start1; i < end2; i++) {
    arr[i] = temp[i];
  }

  auto endTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> durationTime = endTime - startTime;

  // 確認結果
  for (i = start1; i < end2 - 1; i++) {
    if (arr[i] > arr[i + 1]) {
      std::cout << "Merge failed at position " << i 
                << ": " << arr[i] << " > " << arr[i + 1] << std::endl;
      break;
    }
  }

  return durationTime.count();
}

double bubbleSortAndMerge(std::vector<int>& data, int k) {

  int n = data.size();
  std::vector<int> temp(n);

  
  int segmentSize = n / k;
  int remainder = n % k;
  double totalTime = 0.0;

  for (int i = 0; i < k; i++) {
    int start = 0;
    for (int idx = 0; idx < i; idx++) {
      start += segmentSize + (idx < remainder ? 1 : 0);
    }
    int end = start;
    end += segmentSize + (i < remainder ? 1 : 0); // 當前段的範圍大小

    totalTime += bubbleSort(&data[0], start, end);
  }

  for (int step = 1; step < k; step *= 2) {
    for (int i = 0; i < k; i += 2 * step) {
      if (i + step < k) {
        // 計算第一段範圍
        int start1 = 0;
        for (int idx = 0; idx < i; idx++) {
          start1 += segmentSize + (idx < remainder ? 1 : 0);
        }
        int end1 = start1;
        for (int idx = i; idx < i + step; idx++) {
          end1 += segmentSize + (idx < remainder ? 1 : 0);
        }

        // 計算第二段範圍
        int start2 = end1;
        int end2 = start2;
        for (int idx = i + step; idx < std::min(i + 2 * step, k); ++idx) {
           end2 += segmentSize + (idx < remainder ? 1 : 0);
        }

        totalTime += merge(&data[0], &temp[0], start1, end1, start2, end2);
      }
    }
  }

  // Verify the data is sorted
  for (int i = 0; i < n - 1; i++) {
    if (data[i] > data[i + 1]) {
      std::cout << "Sort failed at position " << i << std::endl;
      break;
    }
  }

  return totalTime;
}

double bubbleSortMergeWithMultiprocessing(SharedMemory& sharedMemorySegment, int k) {
  int n = sharedMemorySegment.getSize();

  SharedMemory tempSharedMemorySegment(n);

  auto startTime = std::chrono::high_resolution_clock::now();

  int segmentSize = n / k;
  int remainder = n % k;
  int start = 0; 

  std::vector<pid_t> sortPids;
  for (int i = 0; i < k; i++) {
    pid_t pid = fork();
    if (pid == 0) {  
      int end = start + segmentSize + (i < remainder ? 1 : 0);
      bubbleSort(sharedMemorySegment.getRawData(), start, end); 
      exit(0);
    } else if (pid > 0) {
      sortPids.push_back(pid);
    } else {
      perror("Fork failed");
      exit(1);
    }
    start += segmentSize + (i < remainder ? 1 : 0);
  }
    
  for (auto pid : sortPids) {
    waitpid(pid, nullptr, 0);
  }
  
  for (int step = 1; step < k; step *= 2) {
    std::vector<pid_t> mergePids;
        
    for (int i = 0; i < k; i += 2 * step) {
      if (i + step < k) {
        pid_t pid = fork();             
        if (pid == 0) {
          int start1 = 0;
          for (int idx = 0; idx < i; idx++) {
            start1 += segmentSize + (idx < remainder ? 1 : 0);
          }
          int end1 = start1;
          for (int idx = i; idx < i + step; idx++) {
            end1 += segmentSize + (idx < remainder ? 1 : 0);
          }

          // 計算第二組的範圍
          int start2 = end1;
          int end2 = start2;
          for (int idx = i + step; idx < std::min(i + 2 * step, k); idx++) {
            end2 += segmentSize + (idx < remainder ? 1 : 0);
          }
                    
          merge(sharedMemorySegment.getRawData(), tempSharedMemorySegment.getRawData(), start1, end1, start2, end2);
          exit(0);
        } else if (pid > 0) {
          mergePids.push_back(pid);
        } else {
          perror("Fork failed");
          exit(1);
        }
      }
    }

    for (auto pid : mergePids) {
      waitpid(pid, nullptr, 0);
    }
  }
    
  auto endTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> durationTime = endTime - startTime;

  // 確認結果
  for (int i = 0; i < n - 1; i++) {
    if (sharedMemorySegment.getRawData()[i] > sharedMemorySegment.getRawData()[i + 1]) {
      std::cout << "Sort failed at position " << i << std::endl;
      break;
    }
  }

  return durationTime.count();
}

double bubbleSortMergeWithMultithreading(std::vector<int>& data, int k) {
  int n = data.size();

  std::vector<int> temp(n);

  auto startTime = std::chrono::high_resolution_clock::now();

  int segmentSize = n / k;
  int remainder = n % k;
  int start = 0; 
    
  std::vector<std::thread> sortThreads;
  std::vector<std::pair<int, int>> sortRanges;

  for (int i = 0; i < k; i++) {

    int end = start + segmentSize + (i < remainder ? 1 : 0);
    
    sortRanges.emplace_back(start, end);
    
    // std::thread 的建構函式接收兩個部分(function, parameter...) 
    sortThreads.emplace_back(bubbleSort, &data[0], start, end);

    start += segmentSize + (i < remainder ? 1 : 0);
  }
 

  // thread不能複製 用reference
  for (auto& thread : sortThreads) {
    // 等待thread完成才繼續
    thread.join();
  }
    

  for (int step = 1; step < k; step *= 2) {
    std::vector<std::thread> mergeThreads;
        
    for (int i = 0; i < k; i += 2 * step) {
      if (i + step < k) {
        int start1 = 0;
        for (int idx = 0; idx < i; idx++) {
          start1 += segmentSize + (idx < remainder ? 1 : 0);
        }
        int end1 = start1;
        for (int idx = i; idx < i + step; idx++) {
          end1 += segmentSize + (idx < remainder ? 1 : 0);
        }

        // 計算第二組的範圍
        int start2 = end1;
        int end2 = start2;
        for (int idx = i + step; idx < std::min(i + 2 * step, k); idx++) {
          end2 += segmentSize + (idx < remainder ? 1 : 0);
        }

        mergeThreads.emplace_back(merge, &data[0], &temp[0], start1, end1, start2, end2);

      }
    }

    for (auto& thread : mergeThreads) {
      thread.join();
    }
  }
    
  auto endTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> durationTime = endTime - startTime;

  // 確認結果
  for (int i = 0; i < n - 1; i++) {
    if (data[i] > data[i + 1]) {
      std::cout << "Sort failed at position " << i << std::endl;
      break;
    }
  }

  return durationTime.count();
}


int main() {

  std::string command, fileName;
  
  while (1) {

    std::cout << "Input file name:";
    std::cin >> fileName;
    std::cout << std::endl;

    if (!fileExist(fileName + ".txt")) continue;

    std::cout << "********           Sort           ********" << std::endl;
    std::cout << "* 1.bubble sort                          *" << std::endl;
    std::cout << "* 2.bubble merge sort                    *" << std::endl;
    std::cout << "* 3.bubble merge sort with multiprocess  *" << std::endl;
    std::cout << "* 4.bubble merge sort with multithread   *" << std::endl;
    std::cout << "******************************************" << std::endl;
    std::cout << "Input a command(1, 2, 3, 4): ";

    std::cin >> command;

    std::cout << std::endl;


    if (command == "0") break;
    else if (command != "1" && command != "2" && command != "3" && command != "4") {
      std::cout << "command does not exist!" << std::endl;
      continue;
    } 
    
    std::vector<int> data;
    // read file to vector
    readDataFromFile(fileName + ".txt", data);

    int n = data.size();
    int k = 0;
    double executeTime = 0.0;
    
    if (command == "1") {

      executeTime = bubbleSort(&data[0], 0, data.size());
     
      std::cout << "Total Time:" << std::fixed << std::setprecision(6) << executeTime << "ms" << std::endl;
      
      std::cout << "writing..." << std::endl;

      writeDataToFile(data, fileName + "_output" + command + ".txt", executeTime);

      std::cout << "Complete!" << std::endl;
    } else if (command == "2") {
      std::cout << "Enter the number of partitions(1~" << n << "): ";
      std::cin >> k;
       
      while (k > n || k < 1) {
        std::cout << "Invalid input. Please try again(1~" << n << "): ";
        std::cin >> k;
      }

      executeTime = bubbleSortAndMerge(data, k);

      std::cout << "Total Time:" << std::fixed << std::setprecision(6) << executeTime << "ms" << std::endl;

      std::cout << "writing..." << std::endl;

      writeDataToFile(data, fileName + "_output" + command + ".txt", executeTime);

      std::cout << "Complete!!" << std::endl;

    } else if (command == "3") {

      std::cout << "Enter the number of partitions(1~" << n << "): ";
      std::cin >> k;
       
      while (k > n || k < 1) {
        std::cout << "Invalid input. Please try again(1~" << n << "): ";
        std::cin >> k;
      }

      try {
        SharedMemory sharedMemorySegment(n);

        sharedMemorySegment.setData(data);

        executeTime = bubbleSortMergeWithMultiprocessing(sharedMemorySegment, k);

        std::cout << "Total Time:" << std::fixed << std::setprecision(6) << executeTime << "ms" << std::endl;

        data = sharedMemorySegment.getData();
      
        std::cout << "writing..." << std::endl;

        writeDataToFile(data, fileName + "_output" + command + ".txt", executeTime);

        std::cout << "Complete!" << std::endl;

      } catch (std::invalid_argument& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
      }
      
    } else if (command == "4") {

      std::cout << "Enter the number of partitions(1~" << n << "): ";
      std::cin >> k;
       
      while (k > n || k < 1) {
        std::cout << "Invalid input. Please try again(1~" << n << "): ";
        std::cin >> k;
      }

      executeTime = bubbleSortMergeWithMultithreading(data, k);

      std::cout << "Total Time:" << std::fixed << std::setprecision(6) << executeTime << "ms" << std::endl;
      
      std::cout << "writing..." << std::endl;

      writeDataToFile(data, fileName + "_output" + command + ".txt", executeTime);

      std::cout << "Complete!" << std::endl;
    }

  }   
  return 0;
}