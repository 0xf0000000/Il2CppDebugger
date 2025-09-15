// fully developed by @trickzqw ლ(^o^ლ)

#ifndef IL2CPP_DEBUGGER_H
#define IL2CPP_DEBUGGER_H

#if __cplusplus < 201703L
#error "Requires C++17"
#endif

#include <string>
#include <vector>
#include <ostream>
#include <mutex>
#include <atomic>
#include <thread>
#include <queue>
#include <regex>
#include <functional>
#include <cstdint>
#include <dlfcn.h>
#include <sstream>
#include <condition_variable>
#include <memory>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef IL2CPP_DEBUG_TEST
#define MOCK_SYMBOL(resolver, name, type) type name = reinterpret_cast<type>(0xDEADBEEF);
#else
#define MOCK_SYMBOL(resolver, name, type)
#endif

namespace il2cpp_debug {

struct DebugError {
    enum Code {
        Success,
        InitializationFailed,
        SymbolNotFound,
        HookFailed,
        InvalidArgument,
        InternalError
    };
    Code code;
    std::string message;
};

struct DebugResult {
    bool success;
    DebugError error;
    DebugResult() : success(true), error{DebugError::Success, ""} {}
    DebugResult(DebugError::Code c, const std::string& msg) : success(false), error{c, msg} {}
};

struct Il2CppClass {
    std::string name;
    std::vector<Il2CppClass*> nestedTypes;
    std::vector<void*> methods;
};

struct Il2CppMethodInfo {
    std::string name;
    std::string signature;
    void* methodPointer;
    Il2CppClass* klass;
};

using HookCallback = std::function<void(const Il2CppMethodInfo*, const std::vector<std::string>&, std::string&)>;

class SymbolResolver {
private:
    void* moduleHandle;
    std::mutex mutex;

    using Il2CppDomainGet = void* (*)();
    using Il2CppDomainGetAssemblies = void** (*)(void*, size_t*);
    using Il2CppAssemblyGetImage = void* (*)(void*);
    using Il2CppImageGetClassCount = uint32_t (*)(void*);
    using Il2CppImageGetClass = Il2CppClass* (*)(void*, uint32_t);
    using Il2CppClassGetMethods = void* (*)(Il2CppClass*, uint32_t*);
    using Il2CppClassGetName = const char* (*)(Il2CppClass*);
    using Il2CppMethodGetName = const char* (*)(void*);
    using Il2CppStringChars = const Il2CppChar* (*)(void*);
    using Il2CppMethodGetParamCount = uint32_t (*)(void*);
    using Il2CppMethodGetParamName = const char* (*)(void*, uint32_t);
    using Il2CppMethodGetReturnType = void* (*)(void*);
    using Il2CppTypeGetName = const char* (*)(void*);

    Il2CppDomainGet il2cpp_domain_get = nullptr;
    Il2CppDomainGetAssemblies il2cpp_domain_get_assemblies = nullptr;
    Il2CppAssemblyGetImage il2cpp_assembly_get_image = nullptr;
    Il2CppImageGetClassCount il2cpp_image_get_class_count = nullptr;
    Il2CppImageGetClass il2cpp_image_get_class = nullptr;
    Il2CppClassGetMethods il2cpp_class_get_methods = nullptr;
    Il2CppClassGetName il2cpp_class_get_name = nullptr;
    Il2CppMethodGetName il2cpp_method_get_name = nullptr;
    Il2CppStringChars il2cpp_string_chars = nullptr;
    Il2CppMethodGetParamCount il2cpp_method_get_param_count = nullptr;
    Il2CppMethodGetParamName il2cpp_method_get_param_name = nullptr;
    Il2CppMethodGetReturnType il2cpp_method_get_return_type = nullptr;
    Il2CppTypeGetName il2cpp_type_get_name = nullptr;

#ifdef IL2CPP_DEBUG_TEST
    std::map<std::string, void*> mockSymbols;
#endif

    void* ResolveSymbol(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex);
#ifdef IL2CPP_DEBUG_TEST
        auto it = mockSymbols.find(name);
        if (it != mockSymbols.end()) return it->second;
        return nullptr;
#else
        if (moduleHandle == nullptr) return nullptr;
        return dlsym(moduleHandle, name.c_str());
#endif
    }

public:
    SymbolResolver(std::uintptr_t moduleBase) : moduleHandle(reinterpret_cast<void*>(moduleBase)) {}

    DebugResult Initialize() {
        il2cpp_domain_get = reinterpret_cast<Il2CppDomainGet>(ResolveSymbol("il2cpp_domain_get"));
        if (!il2cpp_domain_get) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_domain_get not found");

        il2cpp_domain_get_assemblies = reinterpret_cast<Il2CppDomainGetAssemblies>(ResolveSymbol("il2cpp_domain_get_assemblies"));
        if (!il2cpp_domain_get_assemblies) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_domain_get_assemblies not found");

        il2cpp_assembly_get_image = reinterpret_cast<Il2CppAssemblyGetImage>(ResolveSymbol("il2cpp_assembly_get_image"));
        if (!il2cpp_assembly_get_image) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_assembly_get_image not found");

        il2cpp_image_get_class_count = reinterpret_cast<Il2CppImageGetClassCount>(ResolveSymbol("il2cpp_image_get_class_count"));
        if (!il2cpp_image_get_class_count) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_image_get_class_count not found");

        il2cpp_image_get_class = reinterpret_cast<Il2CppImageGetClass>(ResolveSymbol("il2cpp_image_get_class"));
        if (!il2cpp_image_get_class) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_image_get_class not found");

        il2cpp_class_get_methods = reinterpret_cast<Il2CppClassGetMethods>(ResolveSymbol("il2cpp_class_get_methods"));
        if (!il2cpp_class_get_methods) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_class_get_methods not found");

        il2cpp_class_get_name = reinterpret_cast<Il2CppClassGetName>(ResolveSymbol("il2cpp_class_get_name"));
        if (!il2cpp_class_get_name) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_class_get_name not found");

        il2cpp_method_get_name = reinterpret_cast<Il2CppMethodGetName>(ResolveSymbol("il2cpp_method_get_name"));
        if (!il2cpp_method_get_name) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_method_get_name not found");

        il2cpp_string_chars = reinterpret_cast<Il2CppStringChars>(ResolveSymbol("il2cpp_string_chars"));
        if (!il2cpp_string_chars) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_string_chars not found");

        il2cpp_method_get_param_count = reinterpret_cast<Il2CppMethodGetParamCount>(ResolveSymbol("il2cpp_method_get_param_count"));
        if (!il2cpp_method_get_param_count) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_method_get_param_count not found");

        il2cpp_method_get_param_name = reinterpret_cast<Il2CppMethodGetParamName>(ResolveSymbol("il2cpp_method_get_param_name"));
        if (!il2cpp_method_get_param_name) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_method_get_param_name not found");

        il2cpp_method_get_return_type = reinterpret_cast<Il2CppMethodGetReturnType>(ResolveSymbol("il2cpp_method_get_return_type"));
        if (!il2cpp_method_get_return_type) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_method_get_return_type not found");

        il2cpp_type_get_name = reinterpret_cast<Il2CppTypeGetName>(ResolveSymbol("il2cpp_type_get_name"));
        if (!il2cpp_type_get_name) return DebugResult(DebugError::SymbolNotFound, "Symbol il2cpp_type_get_name not found");

        return DebugResult();
    }

    void* GetDomain() { return il2cpp_domain_get ? il2cpp_domain_get() : nullptr; }
    void** GetAssemblies(void* domain, size_t* count) { return il2cpp_domain_get_assemblies ? il2cpp_domain_get_assemblies(domain, count) : nullptr; }
    void* GetImage(void* assembly) { return il2cpp_assembly_get_image ? il2cpp_assembly_get_image(assembly) : nullptr; }
    uint32_t GetClassCount(void* image) { return il2cpp_image_get_class_count ? il2cpp_image_get_class_count(image) : 0; }
    Il2CppClass* GetClass(void* image, uint32_t index) { return il2cpp_image_get_class ? il2cpp_image_get_class(image, index) : nullptr; }
    void* GetMethods(Il2CppClass* klass, uint32_t* iter) { return il2cpp_class_get_methods ? il2cpp_class_get_methods(klass, iter) : nullptr; }
    const char* GetClassName(Il2CppClass* klass) { return il2cpp_class_get_name ? il2cpp_class_get_name(klass) : ""; }
    const char* GetMethodName(void* method) { return il2cpp_method_get_name ? il2cpp_method_get_name(method) : ""; }
    std::string StringToUTF8(void* ilStr) {
        if (!il2cpp_string_chars || !ilStr) return "nullptr";
        const Il2CppChar* chars = il2cpp_string_chars(ilStr);
        if (!chars) return "invalid";
        std::string utf8;
        while (*chars) {
            Il2CppChar c = *chars++;
            if (c < 128) utf8 += static_cast<char>(c);
            else if (c < 2048) {
                utf8 += static_cast<char>(192 | (c >> 6));
                utf8 += static_cast<char>(128 | (c & 63));
            } else {
                utf8 += static_cast<char>(224 | (c >> 12));
                utf8 += static_cast<char>(128 | ((c >> 6) & 63));
                utf8 += static_cast<char>(128 | (c & 63));
            }
        }
        return utf8;
    }
    uint32_t GetParamCount(void* method) { return il2cpp_method_get_param_count ? il2cpp_method_get_param_count(method) : 0; }
    const char* GetParamName(void* method, uint32_t index) { return il2cpp_method_get_param_name ? il2cpp_method_get_param_name(method, index) : ""; }
    void* GetReturnType(void* method) { return il2cpp_method_get_return_type ? il2cpp_method_get_return_type(method) : nullptr; }
    const char* GetTypeName(void* type) { return il2cpp_type_get_name ? il2cpp_type_get_name(type) : ""; }

#ifdef IL2CPP_DEBUG_TEST
    void SetMockSymbol(const std::string& name, void* ptr) {
        std::lock_guard<std::mutex> lock(mutex);
        mockSymbols[name] = ptr;
    }
#endif
};

class ErrorHandler {
private:
    std::vector<std::string> errors;
    std::mutex mutex;

public:
    void AddError(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mutex);
        errors.push_back(msg);
    }

    std::vector<std::string> GetErrors() const {
        std::lock_guard<std::mutex> lock(mutex);
        return errors;
    }

    void ClearErrors() {
        std::lock_guard<std::mutex> lock(mutex);
        errors.clear();
    }
};

class IHookBackend {
public:
    virtual ~IHookBackend() = default;
    virtual DebugResult HookMethod(void* methodPtr, std::function<void(const Il2CppMethodInfo*, const std::vector<std::string>&, std::string&)> callback) = 0;
    virtual void UnhookMethod(void* methodPtr) = 0;
    virtual bool IsHooked(void* methodPtr) const = 0;
};

class NoOpHookBackend : public IHookBackend {
public:
    DebugResult HookMethod(void*, std::function<void(const Il2CppMethodInfo*, const std::vector<std::string>&, std::string&)> ) override { return DebugResult(); }
    void UnhookMethod(void*) override {}
    bool IsHooked(void*) const override { return false; }
};

class PlatformStubBackend : public IHookBackend {
public:
    DebugResult HookMethod(void* , std::function<void(const Il2CppMethodInfo*, const std::vector<std::string>&, std::string&)> ) override {
        return DebugResult(DebugError::HookFailed, "Hooking backend not implemented for this platform");
    }
    void UnhookMethod(void* ) override {
    }
    bool IsHooked(void* ) const override {
        return false;
    }
};

class ClassScanner {
private:
    SymbolResolver& resolver;
    std::vector<Il2CppClass> classes;
    std::mutex mutex;

    void FillMethodInfo(void* methodPtr, Il2CppMethodInfo& info, Il2CppClass* klass) {
        info.name = resolver.GetMethodName(methodPtr);
        info.methodPointer = methodPtr;
        info.klass = klass;
        std::stringstream sig;
        void* retType = resolver.GetReturnType(methodPtr);
        sig << (retType ? resolver.GetTypeName(retType) : "void") << " " << info.name << "(";
        uint32_t paramCount = resolver.GetParamCount(methodPtr);
        for (uint32_t k = 0; k < paramCount; ++k) {
            if (k > 0) sig << ", ";
            sig << "param" << k << ": " << resolver.GetParamName(methodPtr, k);
        }
        sig << ")";
        info.signature = sig.str();
    }

public:
    ClassScanner(SymbolResolver& res) : resolver(res) {}

    DebugResult ScanAllClasses() {
        std::lock_guard<std::mutex> lock(mutex);
        classes.clear();

        void* domain = resolver.GetDomain();
        if (!domain) return DebugResult(DebugError::InternalError, "Il2Cpp domain not found");

        size_t assemblyCount = 0;
        void** assemblies = resolver.GetAssemblies(domain, &assemblyCount);
        if (!assemblies) return DebugResult(DebugError::InternalError, "Assemblies not found");

        for (size_t i = 0; i < assemblyCount; ++i) {
            void* image = resolver.GetImage(assemblies[i]);
            if (!image) continue;

            uint32_t classCount = resolver.GetClassCount(image);
            for (uint32_t j = 0; j < classCount; ++j) {
                Il2CppClass* rawKlass = resolver.GetClass(image, j);
                if (!rawKlass) continue;

                Il2CppClass cl;
                cl.name = resolver.GetClassName(rawKlass);

                uint32_t iter = 0;
                while (void* method = resolver.GetMethods(rawKlass, &iter)) {
                    cl.methods.push_back(method);
                }

                classes.push_back(cl);
            }
        }

        return DebugResult();
    }

    const std::vector<Il2CppClass>& GetClasses() const {
        return classes;
    }

    const Il2CppClass* FindClass(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex);
        for (const auto& cl : classes) {
            if (cl.name == name) return &cl;
        }
        return nullptr;
    }

    Il2CppMethodInfo FindMethod(const Il2CppClass* klass, const std::string& methodName) const {
        Il2CppMethodInfo info;
        if (!klass) return info;
        for (auto methodPtr : klass->methods) {
            std::string name = resolver.GetMethodName(methodPtr);
            if (name == methodName) {
                const_cast<ClassScanner*>(this)->FillMethodInfo(methodPtr, info, const_cast<Il2CppClass*>(klass));
                return info;
            }
        }
        return info;
    }
};

class MethodMonitor {
private:
    std::map<void*, HookCallback> hooks;
    std::mutex mutex;
    IHookBackend& backend;
    SymbolResolver& resolver;

    std::string MarshalValue(void* value, const std::string& typeHint) {
        if (typeHint.find("int") != std::string::npos) return std::to_string(*reinterpret_cast<int*>(value));
        if (typeHint.find("float") != std::string::npos) return std::to_string(*reinterpret_cast<float*>(value));
        if (typeHint.find("Il2CppString*") != std::string::npos) return resolver.StringToUTF8(value);
        if (typeHint.find("bool") != std::string::npos) return *reinterpret_cast<bool*>(value) ? "true" : "false";
        if (typeHint.find("char") != std::string::npos) return std::string(1, *reinterpret_cast<char*>(value));
        std::stringstream ss;
        ss << "0x" << std::hex << reinterpret_cast<uintptr_t>(value);
        return ss.str();
    }

public:
    MethodMonitor(IHookBackend& bk, SymbolResolver& res) : backend(bk), resolver(res) {}

    DebugResult AddHook(void* methodPtr, HookCallback callback) {
        std::lock_guard<std::mutex> lock(mutex);
        if (hooks.count(methodPtr) > 0) return DebugResult(DebugError::InvalidArgument, "Method already hooked");
        auto wrappedCallback = [callback](const Il2CppMethodInfo* info, const std::vector<std::string>& params, std::string& ret) {
            callback(info, params, ret);
        };
        auto res = backend.HookMethod(methodPtr, wrappedCallback);
        if (res.success) hooks[methodPtr] = callback;
        return res;
    }

    void RemoveHook(void* methodPtr) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = hooks.find(methodPtr);
        if (it != hooks.end()) {
            backend.UnhookMethod(methodPtr);
            hooks.erase(it);
        }
    }

    DebugResult HookAllMethods(HookCallback callback, const ClassScanner& scanner) {
        const auto& classes = scanner.GetClasses();
        for (const auto& cl : classes) {
            for (auto methodPtr : cl.methods) {
                auto res = AddHook(methodPtr, callback);
                if (!res.success) return res;
            }
        }
        return DebugResult();
    }
};

class MethodLogger {
private:
    std::queue<std::string> logQueue;
    std::mutex mutex;
    std::condition_variable cv;
    std::thread worker;
    std::atomic<bool> running{false};
    std::function<void(const std::string&)> sink;
    std::regex filter;
    double rateLimit = 0.0;
    std::chrono::steady_clock::time_point lastLogTime = std::chrono::steady_clock::now();

    void WorkerThread() {
        while (running) {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [this] { return !logQueue.empty() || !running; });
            if (!running && logQueue.empty()) break;
            if (logQueue.empty()) continue;
            std::string msg = std::move(logQueue.front());
            logQueue.pop();
            lock.unlock();

            auto now = std::chrono::steady_clock::now();
            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastLogTime).count();
            if (rateLimit > 0.0 && delta < 1000.0 / rateLimit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(1000.0 / rateLimit - delta)));
                now = std::chrono::steady_clock::now();
            }
            lastLogTime = now;

            if (std::regex_match(msg, filter)) {
                sink(msg);
            }
        }
    }

public:
    MethodLogger() {
        sink = [](const std::string& msg) {
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_DEBUG, "Il2CppDebugger", "%s", msg.c_str());
#else
            std::cout << msg << std::endl;
#endif
        };
        filter = std::regex(".*");
    }

    ~MethodLogger() {
        Stop();
    }

    void Start() {
        if (running) return;
        running = true;
        worker = std::thread(&MethodLogger::WorkerThread, this);
    }

    void Stop() {
        if (!running) return;
        running = false;
        cv.notify_all();
        if (worker.joinable()) worker.join();
    }

    void Log(const std::string& msg) {
        if (!running) return;
        std::lock_guard<std::mutex> lock(mutex);
        logQueue.push(msg);
        cv.notify_one();
    }

    void SetSink(std::function<void(const std::string&)> newSink) { sink = std::move(newSink); }
    void SetFilter(const std::regex& reg) { filter = reg; }
    void SetRateLimit(double msgsPerSec) { rateLimit = msgsPerSec; }
};

class Il2CppDebugger {
private:
    SymbolResolver resolver;
    ErrorHandler errorHandler;
    ClassScanner scanner;
    MethodMonitor monitor;
    MethodLogger logger;
    std::unique_ptr<IHookBackend> backend;
    std::atomic<bool> enabled{false};

public:
    Il2CppDebugger() : resolver(0), scanner(resolver), monitor(*backend, resolver) {
        backend = std::make_unique<NoOpHookBackend>();
    }

    DebugResult Initialize(std::uintptr_t il2cppModule) {
        resolver = SymbolResolver(il2cppModule);
        auto res = resolver.Initialize();
        if (!res.success) {
            errorHandler.AddError(res.error.message);
            return res;
        }
        res = scanner.ScanAllClasses();
        if (!res.success) {
            errorHandler.AddError(res.error.message);
            return res;
        }
        logger.Start();
        return DebugResult();
    }

    void Enable() { enabled = true; }
    void Disable() { enabled = false; }
    bool IsEnabled() const { return enabled; }

    DebugResult MonitorAllMethods(HookCallback globalCallback) {
        if (!enabled) return DebugResult(DebugError::InvalidArgument, "Debugger not enabled");
        return monitor.HookAllMethods(globalCallback, scanner);
    }

    DebugResult HookSpecificMethod(const std::string& className, const std::string& methodName, HookCallback callback) {
        if (!enabled) return DebugResult(DebugError::InvalidArgument, "Debugger not enabled");
        const Il2CppClass* klass = scanner.FindClass(className);
        if (!klass) return DebugResult(DebugError::InvalidArgument, "Class not found");
        Il2CppMethodInfo method = scanner.FindMethod(klass, methodName);
        if (!method.methodPointer) return DebugResult(DebugError::InvalidArgument, "Method not found");
        return monitor.AddHook(method.methodPointer, callback);
    }

    DebugResult DumpAllClasses(std::ostream& output) const {
        const auto& classes = scanner.GetClasses();
        for (const auto& cl : classes) {
            output << "Class: " << cl.name << std::endl;
            for (auto methodPtr : cl.methods) {
                output << "  Method: " << resolver.GetMethodName(methodPtr) << std::endl;
            }
        }
        return DebugResult();
    }

    std::vector<std::string> GetAllErrors() const { return errorHandler.GetErrors(); }
    void ClearAllErrors() { errorHandler.ClearErrors(); }

    void SetHookBackend(std::unique_ptr<IHookBackend> newBackend) { backend = std::move(newBackend); }
    MethodLogger& GetLogger() { return logger; }
};

} // namespace il2cpp_debug

#endif // IL2CPP_DEBUGGER_H