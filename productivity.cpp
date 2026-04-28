#include "productivity.h"
#include <chrono>


PerformanceDecorator::PerformanceDecorator(std::shared_ptr<ICipher> cipher, const std::string& name, const AlgorithmComplexity& comp)
    : wrappee(cipher), algorithmName(name), complexity(comp) {}

AlgorithmComplexity PerformanceDecorator::getComplexity() const {
    return complexity;
}

std::vector<int> PerformanceDecorator::encrypt(const std::vector<int>& data) {
    CryptoLogger::getInstance().log("[" + algorithmName + "] Theoretical Complexity - Time: "
        + complexity.timeComplexity + ", Space: " + complexity.spaceComplexity);
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> result = wrappee->encrypt(data);
    auto end = std::chrono::high_resolution_clock::now();
    double timeTaken = std::chrono::duration<double, std::milli>(end - start).count();
    size_t memoryUsed = result.capacity() * sizeof(int);
    MetricsCollector::getInstance().addMetric(algorithmName, "Encrypt", timeTaken, memoryUsed);
    CryptoLogger::getInstance().log("[" + algorithmName + "] Performance (Encrypt): "
        + std::to_string(timeTaken) + " ms, Memory allocated: "
        + std::to_string(memoryUsed) + " bytes");
    return result;
}

std::vector<int> PerformanceDecorator::decrypt(const std::vector<int>& data) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> result = wrappee->decrypt(data);
    auto end = std::chrono::high_resolution_clock::now();
    double timeTaken = std::chrono::duration<double, std::milli>(end - start).count();
    size_t memoryUsed = result.capacity() * sizeof(int);
    MetricsCollector::getInstance().addMetric(algorithmName, "Decrypt", timeTaken, memoryUsed);
    CryptoLogger::getInstance().log("[" + algorithmName + "] Performance (Decrypt): "
        + std::to_string(timeTaken) + " ms, Memory allocated: "
        + std::to_string(memoryUsed) + " bytes");
    return result;
}