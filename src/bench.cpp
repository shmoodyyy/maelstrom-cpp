#include "./common/monad/result.h"
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <random>
#include <string_view>

Result<int> monadic_sum(Result<int> a, Result<int> b)
{
  if (!a.is_ok())
    return ERROR_PROPAGATE(a);
  if (!b.is_ok())
    return ERROR_PROPAGATE(b);
  return a.expect() + b.expect();
}

Result<std::string> monadic_concat(Result<std::string_view> a, Result<std::string_view> b) {
  if (!a.is_ok())
    return ERROR_PROPAGATE(a);
  if (!b.is_ok())
    return ERROR_PROPAGATE(b);
  return std::string(a.expect()).append(b.expect());
}

int primitive_sum(int a, int b) {
  return a + b;
}

std::string primitive_concat(std::string_view a, std::string_view b) {
  return std::string(a).append(b);
}

void manual_benchmark_concat(long iterations) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(1, 64);
  char base64_chars[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
  };

  std::vector<std::pair<std::string, std::string>> random_pairs;
  random_pairs.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    int len1 = dist(gen);
    int len2 = dist(gen);

    std::string str1;
    str1.resize(len1);
    for (int i = 0; i < len1; ++i)
      str1[i] = base64_chars[dist(gen)];
    std::string str2;
    str2.resize(len2);
    for (int i = 0; i < len2; ++i)
      str2[i] = base64_chars[dist(gen)];
    random_pairs.emplace_back(std::move(str1), std::move(str2));
  }

  auto start = std::chrono::high_resolution_clock::now();
  std::string result1;
  for (int i = 0; i < iterations; ++i) {
    std::string_view a = std::string_view(random_pairs[i].first);
    std::string_view b = std::string_view(random_pairs[i].second);
    auto res = monadic_concat(a, b);
    if (res.is_ok())
      result1 = res.expect();
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto monadic_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

  start = std::chrono::high_resolution_clock::now();
  std::string result2;
  for (int i = 0; i < iterations; ++i) {
    result2 = primitive_concat(random_pairs[i].first, random_pairs[i].second);
  }
  end = std::chrono::high_resolution_clock::now();
  auto primitive_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

  std::cout << "Monadic sum (" << iterations << " iterations): " << monadic_time.count() << "ns total\n";
  std::cout << "Primitive sum (" << iterations << " iterations): " << primitive_time.count() << "ns total\n";
  std::cout << "Monadic per operation: " << (double)monadic_time.count() / iterations << "ns\n";
  std::cout << "Primitive per operation: " << (double)primitive_time.count() / iterations << "ns\n";
  std::cout << "Ratio: " << (double)monadic_time.count() / primitive_time.count() << "x slower\n";

  std::cout << "Final results: " << result1 << ", " << result2 << std::endl;
}

void manual_benchmark_sum(long iterations) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(1, 255);

  std::vector<std::pair<int, int>> random_pairs;
  random_pairs.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    random_pairs.emplace_back(dist(gen), dist(gen));
  }

  auto start = std::chrono::high_resolution_clock::now();
  int result1;
  for (int i = 0; i < iterations; ++i) {
    auto res = monadic_sum(random_pairs[i].first, random_pairs[i].second);
    if (res.is_ok())
      result1 = res.expect();
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto monadic_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

  start = std::chrono::high_resolution_clock::now();
  int result2;
  for (int i = 0; i < iterations; ++i) {
    result2 = primitive_sum(random_pairs[i].first, random_pairs[i].second);
  }
  end = std::chrono::high_resolution_clock::now();
  auto primitive_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

  std::cout << "Monadic sum (" << iterations << " iterations): " << monadic_time.count() << "ns total\n";
  std::cout << "Primitive sum (" << iterations << " iterations): " << primitive_time.count() << "ns total\n";
  std::cout << "Monadic per operation: " << (double)monadic_time.count() / iterations << "ns\n";
  std::cout << "Primitive per operation: " << (double)primitive_time.count() / iterations << "ns\n";
  std::cout << "Ratio: " << (double)monadic_time.count() / primitive_time.count() << "x slower\n";

  std::cout << "Final results: " << result1 << ", " << result2 << std::endl;
}

Result<int> create_error() {
  return ERROR();
}

int main() {
  const int iterations = 100'000;
  manual_benchmark_sum(iterations);
  std::cout << std::endl;
  manual_benchmark_concat(iterations);
  return 0;
}
