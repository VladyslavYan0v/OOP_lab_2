#include "common.h"
#include "productivity.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <algorithm>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace std;

int modExp(int base, int exp, int mod) {
    long long res = 1;
    long long b = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * b) % mod;
        exp = exp / 2;
        b = (b * b) % mod;
    }
    return static_cast<int>(res);
}

struct CryptoConfig {
    int rsaE, rsaD, rsaN;
    int shamirE, shamirD, shamirP;
    int caesarShift;
    int xorKey;
};


// ПОВЕДІНКОВІ ПАТЕРНИ

// 2. Memento: Фіксує та зберігає внутрішній стан об'єкта.

class MessageMemento {
private:
    vector<int> state;
public:
    MessageMemento(const vector<int>& s) : state(s) {}
    vector<int> getState() const { return state; }
};

// ТВІРНІ ПАТЕРНИ

// 3. Builder: Ізолює процес конструювання складного повідомлення.

class MessageBuilder {
private:
    vector<int> data;
public:
    MessageBuilder& addNumber(int num) {
        data.push_back(num);
        return *this;
    }
    MessageBuilder& addString(const string& str) {
        for (char c : str) data.push_back(static_cast<int>(c));
        return *this;
    }
    MessageMemento save() { return MessageMemento(data); }
    void restore(const MessageMemento& memento) { data = memento.getState(); }
    vector<int> build() { return data; }
    void clear() { data.clear(); }
};

// СТРУКТУРНІ ПАТЕРНИ 

// 4. Adapter: Дозволяє використовувати "старий" клас LegacyShamir через новий інтерфейс.

class LegacyShamir {
private:
    int encKey, decKey, p;
public:
    LegacyShamir(int e, int d, int prime) : encKey(e), decKey(d), p(prime) {}
    int encryptBlock(int data) { return modExp(data, encKey, p); }
    int decryptBlock(int data) { return modExp(data, decKey, p); }
};

class ShamirAdapter : public ICipher {
private:
    LegacyShamir legacyCipher;
public:
    ShamirAdapter(int e, int d, int prime) : legacyCipher(e, d, prime) {}
    vector<int> encrypt(const vector<int>& data) override {
        CryptoLogger::getInstance().log("Encrypting with Shamir protocol.");
        vector<int> result;
        for (int block : data) result.push_back(legacyCipher.encryptBlock(block));
        return result;
    }
    vector<int> decrypt(const vector<int>& data) override {
        CryptoLogger::getInstance().log("Decrypting with Shamir protocol.");
        vector<int> result;
        for (int block : data) result.push_back(legacyCipher.decryptBlock(block));
        return result;
    }
};

// 5. Decorator: Динамічно додає нову поведінку (вимірювання часу) до шифраторів.

class CipherDecorator : public ICipher {
protected:
    shared_ptr<ICipher> wrappee;
public:
    CipherDecorator(shared_ptr<ICipher> cipher) : wrappee(cipher) {}
    vector<int> encrypt(const vector<int>& data) override { return wrappee->encrypt(data); }
    vector<int> decrypt(const vector<int>& data) override { return wrappee->decrypt(data); }
};

class TimingDecorator : public CipherDecorator {
public:
    TimingDecorator(shared_ptr<ICipher> cipher) : CipherDecorator(cipher) {}
    vector<int> encrypt(const vector<int>& data) override {
        auto start = chrono::high_resolution_clock::now();
        vector<int> result = wrappee->encrypt(data);
        auto end = chrono::high_resolution_clock::now();
        CryptoLogger::getInstance().log("Encryption time: " + to_string(chrono::duration<double, milli>(end - start).count()) + " ms");
        return result;
    }
    vector<int> decrypt(const vector<int>& data) override {
        auto start = chrono::high_resolution_clock::now();
        vector<int> result = wrappee->decrypt(data);
        auto end = chrono::high_resolution_clock::now();
        CryptoLogger::getInstance().log("Decryption time: " + to_string(chrono::duration<double, milli>(end - start).count()) + " ms");
        return result;
    }
};

// 6. Composite: Дозволяє згрупувати декілька алгоритмів і працювати з ними як з одним.

class CipherChain : public ICipher {
private:
    vector<shared_ptr<ICipher>> ciphers;
public:
    void addCipher(shared_ptr<ICipher> cipher) { ciphers.push_back(cipher); }
    vector<int> encrypt(const vector<int>& data) override {
        vector<int> currentData = data;
        for (auto& cipher : ciphers) currentData = cipher->encrypt(currentData);
        return currentData;
    }
    vector<int> decrypt(const vector<int>& data) override {
        vector<int> currentData = data;
        for (auto it = ciphers.rbegin(); it != ciphers.rend(); ++it) {
            currentData = (*it)->decrypt(currentData);
        }
        return currentData;
    }

    AlgorithmComplexity getComplexity() const override {
        string t = "", s = "";
        for (size_t i = 0; i < ciphers.size(); ++i) {
            t += ciphers[i]->getComplexity().timeComplexity;
            s += ciphers[i]->getComplexity().spaceComplexity;
            if (i < ciphers.size() - 1) { t += " + "; s += " + "; }
        }
        return { t, s };
    }
};

// ПОВЕДІНКОВІ ПАТЕРНИ 

// 7. Strategy: Інкапсулює алгоритм RSA для взаємозамінності з іншими шифрами.

class RSACipher : public ICipher {
private:
    int e, d, n;
public:
    RSACipher(int pubKey, int privKey, int modulus) : e(pubKey), d(privKey), n(modulus) {}
    vector<int> encrypt(const vector<int>& data) override {
        CryptoLogger::getInstance().log("Encrypting with RSA algorithm.");
        vector<int> result;
        for (int block : data) result.push_back(modExp(block, e, n));
        return result;
    }
    vector<int> decrypt(const vector<int>& data) override {
        CryptoLogger::getInstance().log("Decrypting with RSA algorithm.");
        vector<int> result;
        for (int block : data) result.push_back(modExp(block, d, n));
        return result;
    }
    AlgorithmComplexity getComplexity() const override {
        return { "O(N * log(E))", "O(N)" };
    }
};

class CaesarCipher : public ICipher {
private:
    int shift;
public:
    CaesarCipher(int s) : shift(s) {}
    vector<int> encrypt(const vector<int>& data) override {
        CryptoLogger::getInstance().log("Encrypting with Caesar cipher.");
        vector<int> res;
        for (int b : data) res.push_back(b + shift);
        return res;
    }
    vector<int> decrypt(const vector<int>& data) override {
        CryptoLogger::getInstance().log("Decrypting with Caesar cipher.");
        vector<int> res;
        for (int b : data) res.push_back(b - shift);
        return res;
    }
    AlgorithmComplexity getComplexity() const override {
        return { "O(N)", "O(N)" };
    }
};

class XORCipher : public ICipher {
private:
    int key;
public:
    XORCipher(int k) : key(k) {}
    vector<int> encrypt(const vector<int>& data) override {
        CryptoLogger::getInstance().log("Encrypting with XOR cipher.");
        vector<int> res;
        for (int b : data) res.push_back(b ^ key);
        return res;
    }
    vector<int> decrypt(const vector<int>& data) override {
        CryptoLogger::getInstance().log("Decrypting with XOR cipher.");
        vector<int> res;
        for (int b : data) res.push_back(b ^ key);
        return res;
    }
    AlgorithmComplexity getComplexity() const override {
        return { "O(N)", "O(N)" };
    }
};

// ТВІРНІ ПАТЕРНИ

// 8. Factory Method: Централізує логіку створення конкретних шифраторів.

enum class CipherType { RSA, SHAMIR, CAESAR, XOR, DOUBLE_SECURE, MULTI_LAYER_SYMMETRIC };

class CipherFactory {
public:
    static shared_ptr<ICipher> createCipher(CipherType type, const CryptoConfig& config) {
        if (type == CipherType::RSA) {
            auto rsa = make_shared<RSACipher>(config.rsaE, config.rsaD, config.rsaN);
            return make_shared<PerformanceDecorator>(rsa, "RSA", AlgorithmComplexity{ "O(N * log(E))", "O(N)" });
        }
        else if (type == CipherType::SHAMIR) {
            auto shamir = make_shared<ShamirAdapter>(config.shamirE, config.shamirD, config.shamirP);
            return make_shared<PerformanceDecorator>(shamir, "Shamir", AlgorithmComplexity{ "O(N * log(E))", "O(N)" });
        }
        else if (type == CipherType::CAESAR) {
            auto caesar = make_shared<CaesarCipher>(config.caesarShift);
            return make_shared<PerformanceDecorator>(caesar, "Caesar", AlgorithmComplexity{ "O(N)", "O(N)" });
        }
        else if (type == CipherType::XOR) {
            auto xorC = make_shared<XORCipher>(config.xorKey);
            return make_shared<PerformanceDecorator>(xorC, "XOR", AlgorithmComplexity{ "O(N)", "O(N)" });
        }
        else if (type == CipherType::DOUBLE_SECURE) {
            auto chain = make_shared<CipherChain>();
            chain->addCipher(make_shared<RSACipher>(config.rsaE, config.rsaD, config.rsaN));
            chain->addCipher(make_shared<ShamirAdapter>(config.shamirE, config.shamirD, config.shamirP));
            return make_shared<PerformanceDecorator>(chain, "DoubleSecure", chain->getComplexity());
        }
        else if (type == CipherType::MULTI_LAYER_SYMMETRIC) {
            auto chain = make_shared<CipherChain>();
            chain->addCipher(make_shared<CaesarCipher>(config.caesarShift));
            chain->addCipher(make_shared<XORCipher>(config.xorKey));
            return make_shared<PerformanceDecorator>(chain, "MultiLayer", chain->getComplexity());
        }
        return nullptr;
    }
};


// ПОВЕДІНКОВІ ПАТЕРНИ

// 9. Observer: Механізм підписки на події передачі даних.

class ITransferObserver {
public:
    virtual void onTransferComplete(size_t dataSize) = 0;
};

class AuditSystem : public ITransferObserver {
public:
    void onTransferComplete(size_t dataSize) override {
        CryptoLogger::getInstance().log("Audit: processed " + to_string(dataSize) + " data blocks.");
    }
};


// 10. Template Method: Визначає загальний скелет процесу передачі даних.

class SecureDataTransfer {
private:
    vector<ITransferObserver*> observers;
public:
    void addObserver(ITransferObserver* obs) { observers.push_back(obs); }
    vector<int> transfer(const vector<int>& rawData, shared_ptr<ICipher> cipher) {
        vector<int> formatted = formatData(rawData);
        vector<int> encrypted = cipher->encrypt(formatted);
        vector<int> finalData = addHeader(encrypted);
        for (auto obs : observers) obs->onTransferComplete(finalData.size());
        return finalData;
    }
protected:
    virtual vector<int> formatData(const vector<int>& data) { return data; }
    virtual vector<int> addHeader(vector<int> data) {
        data.insert(data.begin(), 999);
        return data;
    }
};

// СТРУКТУРНІ ПАТЕРНИ


// 11. Facade: Надає простий і зрозумілий інтерфейс до складної підсистеми.

class CryptoFacade {
private:
    SecureDataTransfer transferProtocol;
    AuditSystem auditor;
public:
    CryptoFacade() { transferProtocol.addObserver(&auditor); }
    vector<int> sendSecretMessage(const vector<int>& messageArray, CipherType type, const CryptoConfig& config) {
        auto cipher = CipherFactory::createCipher(type, config);
        return transferProtocol.transfer(messageArray, cipher);
    }
    vector<int> readSecretMessage(const vector<int>& encryptedArray, CipherType type, const CryptoConfig& config) {
        auto cipher = CipherFactory::createCipher(type, config);
        vector<int> dataOnly(encryptedArray.begin() + 1, encryptedArray.end());
        return cipher->decrypt(dataOnly);
    }
};

// ПОВЕДІНКОВІ ПАТЕРНИ

// 12. Command: Інкапсулює запит на відправку як об'єкт, дозволяючи відкладене виконання.
 
class SendMessageCommand {
private:
    CryptoFacade* facade;
    vector<int> message;
    CipherType type;
    CryptoConfig config;
    vector<int> result;
public:
    SendMessageCommand(CryptoFacade* f, const vector<int>& m, CipherType t, const CryptoConfig& c)
        : facade(f), message(m), type(t), config(c) {}
    void execute() {
        result = facade->sendSecretMessage(message, type, config);
    }
    vector<int> getResult() const { return result; }
};


int main() {
    CryptoConfig myConfig = {
        17, 2753, 3233,    // ключі RSA
        11, 2921, 3571,    // ключі Shamir
        5,                 // зсув для цезаря
        170                // ключ для XOR
    };

    string secretText;
    cout << "Enter your secret message: ";
    getline(cin, secretText);
    cout << "\n";

    vector<int> originalMessage = builder.addString(secretText).build();

    cout << "=== INPUT ARRAY (ASCII) ===" << endl;
    for (int val : originalMessage) cout << val << " ";
    cout << "\n\n";
    cout << "==========================================\n";
    cout << "       STARTING ENCRYPTION TESTS\n";
    cout << "==========================================\n\n";

    cout << "--- 1. Testing RSA ALGORITHM ---" << endl;
    SendMessageCommand rsaCmd(&facade, originalMessage, CipherType::RSA, myConfig);
    rsaCmd.execute();
    vector<int> rsaReceived = facade.readSecretMessage(rsaCmd.getResult(), CipherType::RSA, myConfig);
    cout << "RSA Decrypted Text: ";
    for (int val : rsaReceived) cout << static_cast<char>(val);
    cout << "\n\n";

    cout << "--- 2. Testing SHAMIR PROTOCOL ---" << endl;
    SendMessageCommand shamirCmd(&facade, originalMessage, CipherType::SHAMIR, myConfig);
    shamirCmd.execute();
    vector<int> shamirReceived = facade.readSecretMessage(shamirCmd.getResult(), CipherType::SHAMIR, myConfig);
    cout << "Shamir Decrypted Text: ";
    for (int val : shamirReceived) cout << static_cast<char>(val);
    cout << "\n\n";

    cout << "--- 3. Testing CAESAR CIPHER ---" << endl;
    SendMessageCommand caesarCmd(&facade, originalMessage, CipherType::CAESAR, myConfig);
    caesarCmd.execute();
    vector<int> caesarReceived = facade.readSecretMessage(caesarCmd.getResult(), CipherType::CAESAR, myConfig);
    cout << "Caesar Decrypted Text: ";
    for (int val : caesarReceived) cout << static_cast<char>(val);
    cout << "\n\n";

    cout << "--- 4. Testing XOR CIPHER ---" << endl;
    SendMessageCommand xorCmd(&facade, originalMessage, CipherType::XOR, myConfig);
    xorCmd.execute();
    vector<int> xorReceived = facade.readSecretMessage(xorCmd.getResult(), CipherType::XOR, myConfig);
    cout << "XOR Decrypted Text: ";
    for (int val : xorReceived) cout << static_cast<char>(val);
    cout << "\n\n";

    cout << "--- 5. Testing DOUBLE_SECURE (RSA + Shamir) ---" << endl;
    SendMessageCommand doubleCmd(&facade, originalMessage, CipherType::DOUBLE_SECURE, myConfig);
    doubleCmd.execute();
    vector<int> doubleReceived = facade.readSecretMessage(doubleCmd.getResult(), CipherType::DOUBLE_SECURE, myConfig);
    cout << "Double Secure Decrypted Text: ";
    for (int val : doubleReceived) cout << static_cast<char>(val);
    cout << "\n\n";

    cout << "--- 6. Testing MULTI_LAYER_SYMMETRIC (Caesar + XOR) ---" << endl;
    SendMessageCommand multiCmd(&facade, originalMessage, CipherType::MULTI_LAYER_SYMMETRIC, myConfig);
    multiCmd.execute();
    vector<int> multiReceived = facade.readSecretMessage(multiCmd.getResult(), CipherType::MULTI_LAYER_SYMMETRIC, myConfig);
    cout << "Multi-Layer Decrypted Text: ";
    for (int val : multiReceived) cout << static_cast<char>(val);
    cout << "\n\n";

    cout << "==========================================\n";
    cout << "       ALL TESTS COMPLETED\n";
    cout << "==========================================\n\n";

    cout << "=== FINAL PERFORMANCE METRICS REPORT (TESTS + USER) ===\n";
    const auto& finalMetrics = MetricsCollector::getInstance().getMetrics();
    for (const auto& metric : finalMetrics) {
        cout << "Algorithm: " << metric.algorithmName
            << "\t| Op: " << metric.operation
            << "\t| Time: " << metric.timeMs << " ms"
            << "\t| Memory: " << metric.memoryBytes << " bytes\n";
    }
    cout << "=======================================================\n";

    MetricsCollector::getInstance().saveToBinary("full_performance_stats.bin");

    return 0;
}

namespace py = pybind11;

PYBIND11_MODULE(crypto_engine, m) {
    py::enum_<CipherType>(m, "CipherType")
        .value("RSA", CipherType::RSA)
        .value("SHAMIR", CipherType::SHAMIR)
        .value("DOUBLE_SECURE", CipherType::DOUBLE_SECURE)
        .export_values();

    py::class_<CryptoConfig>(m, "CryptoConfig")
        .def(py::init<>())
        .def_readwrite("rsaE", &CryptoConfig::rsaE)
        .def_readwrite("rsaD", &CryptoConfig::rsaD)
        .def_readwrite("rsaN", &CryptoConfig::rsaN)
        .def_readwrite("shamirE", &CryptoConfig::shamirE)
        .def_readwrite("shamirD", &CryptoConfig::shamirD)
        .def_readwrite("shamirP", &CryptoConfig::shamirP);

    py::class_<CryptoFacade>(m, "CryptoFacade")
        .def(py::init<>())
        .def("sendSecretMessage", &CryptoFacade::sendSecretMessage)
        .def("readSecretMessage", &CryptoFacade::readSecretMessage);
}