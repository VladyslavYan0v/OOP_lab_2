#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>

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
};

// ТВІРНІ ПАТЕРНИ 

// 1. Singleton: Гарантує існування лише одного екземпляра логера.
 
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
    void log(const string& message) {
        cout << "[LOG]: " << message << endl;
    }
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
};

// СТРУКТУРНІ ПАТЕРНИ 

class IAsymmetricCipher {
public:
    virtual vector<int> encrypt(const vector<int>& data) = 0;
    virtual vector<int> decrypt(const vector<int>& data) = 0;
    virtual ~IAsymmetricCipher() = default;
};

// 4. Adapter: Дозволяє використовувати "старий" клас LegacyShamir через новий інтерфейс.

class LegacyShamir {
private:
    int encKey, decKey, p;
public:
    LegacyShamir(int e, int d, int prime) : encKey(e), decKey(d), p(prime) {}
    int encryptBlock(int data) { return modExp(data, encKey, p); }
    int decryptBlock(int data) { return modExp(data, decKey, p); }
};

class ShamirAdapter : public IAsymmetricCipher {
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

class CipherDecorator : public IAsymmetricCipher {
protected:
    shared_ptr<IAsymmetricCipher> wrappee;
public:
    CipherDecorator(shared_ptr<IAsymmetricCipher> cipher) : wrappee(cipher) {}
    vector<int> encrypt(const vector<int>& data) override { return wrappee->encrypt(data); }
    vector<int> decrypt(const vector<int>& data) override { return wrappee->decrypt(data); }
};

class TimingDecorator : public CipherDecorator {
public:
    TimingDecorator(shared_ptr<IAsymmetricCipher> cipher) : CipherDecorator(cipher) {}
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

class CipherChain : public IAsymmetricCipher {
private:
    vector<shared_ptr<IAsymmetricCipher>> ciphers;
public:
    void addCipher(shared_ptr<IAsymmetricCipher> cipher) { ciphers.push_back(cipher); }
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
};

// ПОВЕДІНКОВІ ПАТЕРНИ 

// 7. Strategy: Інкапсулює алгоритм RSA для взаємозамінності з іншими шифрами.

class RSACipher : public IAsymmetricCipher {
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
};

// ТВІРНІ ПАТЕРНИ

// 8. Factory Method: Централізує логіку створення конкретних шифраторів.

enum class CipherType { RSA, SHAMIR, DOUBLE_SECURE };

class CipherFactory {
public:
    static shared_ptr<IAsymmetricCipher> createCipher(CipherType type, const CryptoConfig& config) {
        if (type == CipherType::RSA) {
            return make_shared<TimingDecorator>(make_shared<RSACipher>(config.rsaE, config.rsaD, config.rsaN));
        }
        else if (type == CipherType::SHAMIR) {
            return make_shared<TimingDecorator>(make_shared<ShamirAdapter>(config.shamirE, config.shamirD, config.shamirP));
        }
        else if (type == CipherType::DOUBLE_SECURE) {
            auto chain = make_shared<CipherChain>();
            chain->addCipher(make_shared<RSACipher>(config.rsaE, config.rsaD, config.rsaN));
            chain->addCipher(make_shared<ShamirAdapter>(config.shamirE, config.shamirD, config.shamirP));
            return make_shared<TimingDecorator>(chain);
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
    vector<int> transfer(const vector<int>& rawData, shared_ptr<IAsymmetricCipher> cipher) {
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
        17, 2753, 3233,
        11, 2921, 3571
    };

    CryptoFacade facade;
    MessageBuilder builder;

    vector<int> originalMessage = builder.addNumber(42).addString("KNU").build();

    cout << "=== INPUT ARRAY ===" << endl;
    for (int val : originalMessage) cout << val << " ";
    cout << "\n\n";

    SendMessageCommand sendCmd(&facade, originalMessage, CipherType::DOUBLE_SECURE, myConfig);
    sendCmd.execute();
    vector<int> sentData = sendCmd.getResult();

    cout << "=== ENCRYPTED ARRAY ===" << endl;
    for (int val : sentData) cout << val << " ";
    cout << "\n\n";

    vector<int> receivedData = facade.readSecretMessage(sentData, CipherType::DOUBLE_SECURE, myConfig);

    cout << "=== DECRYPTED ARRAY ===" << endl;
    for (int val : receivedData) cout << val << " ";
    cout << "\n";

    return 0;
}