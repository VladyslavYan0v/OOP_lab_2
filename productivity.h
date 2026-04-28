#pragma once
#include <memory>
#include <string>
#include <vector>

#include "common.h"

class PerformanceDecorator : public ICipher {
private:
    std::shared_ptr<ICipher> wrappee;
    std::string algorithmName;
    AlgorithmComplexity complexity;

public:
    PerformanceDecorator(std::shared_ptr<ICipher> cipher, const std::string& name, const AlgorithmComplexity& comp);

    std::vector<int> encrypt(const std::vector<int>& data) override;
    std::vector<int> decrypt(const std::vector<int>& data) override;
    AlgorithmComplexity getComplexity() const override;
};