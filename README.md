Il2CppDebugger

A C++17 library for debugging and monitoring IL2CPP applications at runtime. It provides method hooking, class scanning, and logging capabilities.

Features

· Symbol Resolution: Resolves IL2CPP runtime symbols automatically
· Class Scanning: Discovers all classes and methods in the application
· Method Hooking: Intercept method calls with custom callbacks
· Logging System: Thread-safe logging with filtering and rate limiting
· Cross-platform: Works on Android and other platforms

Usage Example

```cpp
#include "Il2CppDebugger.h"

void MyCallback(const il2cpp_debug::Il2CppMethodInfo* method, 
               const std::vector<std::string>& params, 
               std::string& returnValue) {
    std::string logMsg = "Method called: " + method->name;
}

int main() {
    il2cpp_debug::Il2CppDebugger debugger;
    
    auto result = debugger.Initialize(0x12345678);
    if (!result.success) {
        return -1;
    }
    
    debugger.Enable();
    debugger.MonitorAllMethods(MyCallback);
    debugger.HookSpecificMethod("MyNamespace.MyClass", "MyMethod", MyCallback);
    debugger.DumpAllClasses(std::cout);
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
```

Android Integration

```cpp
void* il2cpp_handle = dlopen("libil2cpp.so", RTLD_LAZY);
std::uintptr_t il2cpp_base = reinterpret_cast<std::uintptr_t>(il2cpp_handle);

il2cpp_debug::Il2CppDebugger debugger;
debugger.Initialize(il2cpp_base);
debugger.Enable();

debugger.GetLogger().SetSink([](const std::string& msg) {
    __android_log_print(ANDROID_LOG_DEBUG, "MyApp", "%s", msg.c_str());
});
```

Building

Requires C++17 compatible compiler. For Android:

```cmake
add_library(mylib SHARED
    Il2CppDebugger.h
)

target_compile_features(mylib PRIVATE cxx_std_17)
```

License

Developed by @trickzqw. Free to use with proper attribution.
