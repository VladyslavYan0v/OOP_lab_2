#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "common.h"

inline void assertTest(bool condition, const std::string& testName) {
    if (condition) {
        std::cout << "[PASSED] " << testName << "\n";
    }
    else {
        std::cout << "[FAILED] " << testName << "\n";
    }
}

template <typename FacadeType, typename BuilderType, typename ConfigType>
inline void runAutomatedTests(FacadeType& facade, BuilderType& builder, const ConfigType& testConfig) {
    std::cout << "\n==========================================\n";
    std::cout << "       STARTING AUTOMATED TESTS\n";
    std::cout << "==========================================\n\n";

    // Дані для тестування (слово "HELLO" в ASCII)
    std::vector<int> testData = { 72, 69, 76, 76, 79 };
    struct TestTarget {
        CipherType type;
        std::string name;
        std::string expectedTime;
    };
    std::vector<TestTarget> targets = {
        {CipherType::RSA, "RSA", "O(N * log(E))"},
        {CipherType::SHAMIR, "Shamir", "O(N * log(E))"},
        {CipherType::CAESAR, "Caesar", "O(N)"},
        {CipherType::XOR, "XOR", "O(N)"},
        {CipherType::DOUBLE_SECURE, "DoubleSecure", "O(N * log(E)) + O(N * log(E))"},
        {CipherType::MULTI_LAYER_SYMMETRIC, "MultiLayer", "O(N) + O(N)"}
    };
    for (const auto& target : targets) {
        std::cout << ">>> Testing " << target.name << "...\n";
        auto cipher = CipherFactory::createCipher(target.type, testConfig);
        std::vector<int> encrypted = cipher->encrypt(testData);
        std::vector<int> decrypted = cipher->decrypt(encrypted);
        assertTest(decrypted == testData, target.name + ": Symmetry check (Integrity)");
        assertTest(cipher->getComplexity().timeComplexity == target.expectedTime,
            target.name + ": Complexity string match");

        std::cout << "------------------------------------------\n";
    }
    std::cout << ">>> Testing Facade & Template Method Systems...\n";
    builder.clear();
    std::vector<int> msg = builder.addString("UNIT").build();
    std::vector<int> sent = facade.sendSecretMessage(msg, CipherType::XOR, testConfig);
    assertTest(sent[0] == 999, "Template Method: Protocol header 999 successfully injected");
    assertTest(facade.readSecretMessage(sent, CipherType::XOR, testConfig) == msg, "Facade: Full E2E cycle works perfectly");
    std::cout << "\n==========================================\n";
    std::cout << "       TESTING COMPLETED\n";
    std::cout << "==========================================\n\n";
}