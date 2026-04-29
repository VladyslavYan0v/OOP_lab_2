#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

struct EncryptionResponse {
    std::vector<int> encryptedData;
    std::string timeComplexity;
    double timeMs;
    size_t memoryBytes;
};

struct AlgorithmComplexity {
    std::string timeComplexity;
    std::string spaceComplexity;
};

class ICipher {
public:
    virtual std::vector<int> encrypt(const std::vector<int>& data) = 0;
    virtual std::vector<int> decrypt(const std::vector<int>& data) = 0;
    virtual AlgorithmComplexity getComplexity() const = 0;
    virtual ~ICipher() = default;
};

class CryptoLogger {
private:
    CryptoLogger() {}
    CryptoLogger(const CryptoLogger&) = delete;
    CryptoLogger& operator=(const CryptoLogger&) = delete;
public:
    static CryptoLogger& getInstance() {
        static CryptoLogger instance;
        return instance;
    }
    void log(const std::string& message) {
        std::cout << "[LOG]: " << message << std::endl;
    }
};

struct MetricRecord {
    std::string algorithmName;
    std::string operation;
    double timeMs;
    size_t memoryBytes;
};

class MetricsCollector {
private:
    std::vector<MetricRecord> history;
    MetricsCollector() {}
    MetricsCollector(const MetricsCollector&) = delete;
    MetricsCollector& operator=(const MetricsCollector&) = delete;
public:
    static MetricsCollector& getInstance() {
        static MetricsCollector instance;
        return instance;
    }
    void addMetric(const std::string& algo, const std::string& op, double tMs, size_t memBytes) {
        history.push_back({ algo, op, tMs, memBytes });
    }
    const std::vector<MetricRecord>& getMetrics() const {
        return history;
    }
    void saveToBinary(const std::string& filename) {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs.is_open()) {
            std::cerr << "Error: Could not open file for binary writing!\n";
            return;
        }
        size_t count = history.size();
        ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));

        for (const auto& record : history) {
            size_t algoLen = record.algorithmName.size();
            ofs.write(reinterpret_cast<const char*>(&algoLen), sizeof(algoLen));
            ofs.write(record.algorithmName.data(), algoLen);
            size_t opLen = record.operation.size();
            ofs.write(reinterpret_cast<const char*>(&opLen), sizeof(opLen));
            ofs.write(record.operation.data(), opLen);
            ofs.write(reinterpret_cast<const char*>(&record.timeMs), sizeof(record.timeMs));
            ofs.write(reinterpret_cast<const char*>(&record.memoryBytes), sizeof(record.memoryBytes));
        }

        ofs.close();
        std::cout << "[SYSTEM]: Metrics successfully saved to binary file: " << filename << "\n";
    }
};