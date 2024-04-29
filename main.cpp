#include <iostream>
//Знайти кількість елементів, кратних 17, і найменший такий елемент.
#include <functional>
#include <chrono>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <string>
#include <barrier>
#include <cstring>
#include "tracy/Tracy.hpp"


constexpr int arraySize = 100000; // Розмір масиву
constexpr int numThreads = 6; // Кількість потоків
std::mutex mtx;

std::atomic<int> count(0); // Атомарний лічильник кількості елементів, кратних 17
std::atomic<int> smallestElement(INT_MAX); // Атомарна змінна для збереження найменшого елемента
std::barrier my_barrier(numThreads);

int countMutex = 0;
int smallestElementMutex = INT_MAX;

int count1Thread = 0;
int smallestElement1Thread = INT_MAX;

void countMultiples(const std::vector<int>& array, int threadId) {
    my_barrier.arrive_and_wait();
    int smallest = INT_MAX;
    ZoneScopedN( "thread" );
    for (size_t i = threadId; i < array.size(); i += numThreads) {
        if (array[i] % 17 == 0) {
            count.fetch_add(1);
            smallest = std::min(smallest, array[i]);
        }
    }

    // Атомарно оновлюємо найменший елемент
    int currentSmallest = smallestElement.load(std::memory_order_relaxed);
    while (smallest < currentSmallest && !smallestElement.compare_exchange_weak(currentSmallest, smallest)) {
        currentSmallest = smallestElement.load(std::memory_order_relaxed);
    }
}

void countMultiplesMutex(const std::vector<int>& array, int threadId) {
    my_barrier.arrive_and_wait();
    ZoneScopedN( "thread" );
    for (size_t i = threadId; i < array.size(); i += numThreads) {
        if (array[i] % 17 == 0) {
            std::lock_guard<std::mutex> lock(mtx);
            countMutex++;
            smallestElementMutex = std::min(smallestElementMutex, array[i]);
        }
    }
}

void countMultiples1Thread(const std::vector<int>& array) {
    ZoneScopedN( "thread" );
    for (size_t i = 0; i < array.size(); i ++) {
        if (array[i] % 17 == 0) {
            count1Thread++;
            smallestElement1Thread = std::min(smallestElement1Thread, array[i]);
        }
    }
}
int main() {
    FrameMark;
    srand(time(NULL));
    std::vector<int> array(arraySize);
    std::barrier barrier(numThreads);
    // Заповнюємо масив довільними значеннями
    for (int i = 0; i < arraySize; ++i) {
        array[i] = rand() % 1000;
    }

    // Створюємо потоки для обробки масиву
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(countMultiples,array, i);
    }
    auto start = std::chrono::steady_clock::now();
    // Очікуємо завершення роботи всіх потоків
    for (auto& thread : threads) {
        thread.join();
    }
    auto end = std::chrono::steady_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    threads.clear();
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(countMultiplesMutex,array, i);
    }
    auto start1 = std::chrono::steady_clock::now();
    // Очікуємо завершення роботи всіх потоків
    for (auto& thread : threads) {
        thread.join();
    }
    auto end1 = std::chrono::steady_clock::now();
    auto milliseconds1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();

    threads.clear();

    auto start2 = std::chrono::steady_clock::now();
    countMultiples1Thread(array);
    auto end2 = std::chrono::steady_clock::now();
    auto milliseconds2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();

    // Виводимо результати
    std::cout << "count of 17: " << count << std::endl;
    std::cout << "smallest of 17: " << smallestElement << std::endl;
    std::cout << "seconds Atomic " << milliseconds/1000.0 << "\n";
    std::cout << "count of 17: " << countMutex << std::endl;
    std::cout << "smallest of 17: " << smallestElementMutex << std::endl;
    std::cout << "seconds Mutex " << milliseconds1/1000.0 << "\n";
    std::cout << "count of 17: " << count1Thread << std::endl;
    std::cout << "smallest of 17: " << smallestElement1Thread << std::endl;
    std::cout << "seconds 1 Thread " << milliseconds2/1000.0 << "\n";
    return 0;
}

