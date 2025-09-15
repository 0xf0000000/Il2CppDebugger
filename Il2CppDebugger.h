// fully developed by @trickzqw ლ(^o^ლ)

#pragma once

#include <android/log.h>
#include <map>
#include <jni.h>
#include <unistd.h>
#include "xdl.h"
#include <stdio.h>
#include <string>
#include <inttypes.h>
#include <codecvt>
#include <locale>
#include <dlfcn.h>

#ifndef IL2CPPDEBUGGER_H
#define IL2CPPDEBUGGER_H
#define g_LogTag "trickzqww"

struct Il2CppString;

typedef unsigned short UTF16;
typedef wchar_t UTF32;
typedef char UTF8;

namespace {
	const void *(*il2cpp_assembly_get_image)(const void *assembly);
	void *(*il2cpp_domain_get)();
	void **(*il2cpp_domain_get_assemblies)(const void *domain, size_t *size);
	const char *(*il2cpp_image_get_name)(void *image);
    void *(*il2cpp_class_from_name)(const void *image, const char *namespaze, const char *name);
    void *(*il2cpp_class_get_field_from_name)(void *klass, const char *name);
    void *(*il2cpp_class_get_method_from_name)(void *klass, const char *name, int argsCount);
    size_t (*il2cpp_field_get_offset)(void *field);
    void (*il2cpp_field_static_get_value)(void *field, void *value);
    void (*il2cpp_field_static_set_value)(void *field, void *value);
    void *(*il2cpp_array_new)(void *elementTypeInfo, size_t length);
    uint16_t *(*il2cpp_string_chars)(void *str);
    Il2CppString *(*il2cpp_string_new)(const char *str);
    Il2CppString *(*il2cpp_string_new_utf16)(const wchar_t *str, int32_t length);
    char *(*il2cpp_type_get_name)(void *type);
    void* (*il2cpp_method_get_param)(void *method, uint32_t index);
    void* (*il2cpp_class_get_methods)(void *klass, void* *iter);
    const char* (*il2cpp_method_get_name)(void *method);
    void *(*il2cpp_object_new)(void *klass);
}

int is_surrogate(UTF16 uc) {
    return (uc - 0xd800u) < 2048u;
}

int is_high_surrogate(UTF16 uc) {
    return (uc & 0xfffffc00) == 0xd800;
}

int is_low_surrogate(UTF16 uc) {
    return (uc & 0xfffffc00) == 0xdc00;
}

UTF32 surrogate_to_utf32(UTF16 high, UTF16 low) {
    return (high << 10) + low - 0x35fdc00;
}

const char* utf16_to_utf8(const UTF16* source, size_t len) {
    std::u16string s(source, source + len);
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.to_bytes(s).c_str();
}

const wchar_t* utf16_to_utf32(const UTF16* source, size_t len) {
	auto output = new UTF32[len + 1];

    for (int i = 0; i < len; i++) {
        const UTF16 uc = source[i];
        if (!is_surrogate(uc)) {
            output[i] = uc;
        }
        else {
            if (is_high_surrogate(uc) && is_low_surrogate(source[i]))
                output[i] = surrogate_to_utf32(uc, source[i]);
            else
                output[i] = L'?';
        }
    }

    output[len] = L'\0';
    return output;
}

typedef void(*Il2CppMethodPointer)();

struct MethodInfo;

struct VirtualInvokeData
{
    Il2CppMethodPointer methodPtr;
    const MethodInfo* method;
};

struct Il2CppType
{
    void* data;
    unsigned int bits;
};

struct Il2CppClass;

struct Il2CppObject
{
    Il2CppClass *klass;
    void *monitor;
};

union Il2CppRGCTXData
{
    void* rgctxDataDummy;
    const MethodInfo* method;
    const Il2CppType* type;
    Il2CppClass* klass;
};

struct Il2CppClass_1
{
    void* image;
    void* gc_desc;
    const char* name;
    const char* namespaze;
    Il2CppType* byval_arg;
    Il2CppType* this_arg;
    Il2CppClass* element_class;
    Il2CppClass* castClass;
    Il2CppClass* declaringType;
    Il2CppClass* parent;
    void *generic_class;
    void* typeDefinition;
    void* interopData;
    void* fields;
    void* events;
    void* properties;
    void* methods;
    Il2CppClass** nestedTypes;
    Il2CppClass** implementedInterfaces;
    void* interfaceOffsets;
};

struct Il2CppClass_2
{
    Il2CppClass** typeHierarchy;
    uint32_t cctor_started;
    uint32_t cctor_finished;
    uint64_t cctor_thread;
    int32_t genericContainerIndex;
    int32_t customAttributeIndex;
    uint32_t instance_size;
    uint32_t actualSize;
    uint32_t element_size;
    int32_t native_size;
    uint32_t static_fields_size;
    uint32_t thread_static_fields_size;
    int32_t thread_static_fields_offset;
    uint32_t flags;
    uint32_t token;
    uint16_t method_count;
    uint16_t property_count;
    uint16_t field_count;
    uint16_t event_count;
    uint16_t nested_type_count;
    uint16_t vtable_count;
    uint16_t interfaces_count;
    uint16_t interface_offsets_count;
    uint8_t typeHierarchyDepth;
    uint8_t genericRecursionDepth;
    uint8_t rank;
    uint8_t minimumAlignment;
    uint8_t packingSize;
    uint8_t bitflags1;
    uint8_t bitflags2;
};

struct Il2CppClass
{
    Il2CppClass_1 _1;
    void* static_fields;
    Il2CppRGCTXData* rgctx_data;
    Il2CppClass_2 _2;
    VirtualInvokeData vtable[255];
};

typedef int32_t il2cpp_array_size_t;
typedef int32_t il2cpp_array_lower_bound_t;
struct Il2CppArrayBounds
{
    il2cpp_array_size_t length;
    il2cpp_array_lower_bound_t lower_bound;
};

struct MethodInfo
{
    Il2CppMethodPointer methodPointer;
    void* invoker_method;
    const char* name;
    Il2CppClass *declaring_type;
    const Il2CppType *return_type;
    const void* parameters;
    union
    {
        const Il2CppRGCTXData* rgctx_data;
        const void* methodDefinition;
    };
    union
    {
        const void* genericMethod;
        const void* genericContainer;
    };
    int32_t customAttributeIndex;
    uint32_t token;
    uint16_t flags;
    uint16_t iflags;
    uint16_t slot;
    uint8_t parameters_count;
    uint8_t bitflags;
};

struct FieldInfo
{
public:
    const char* name;
    const Il2CppType* type;
    Il2CppClass *parent;
    int32_t offset;
    uint32_t token;
};

template<typename T> struct Il2CppArray {
    Il2CppClass *klass;
    void *monitor;
    void *bounds;
    int max_length;
    T m_Items[65535];

    int getLength() {
        return max_length;
    }

    T *getPointer() {
        return (T *)m_Items;
    }

    T &operator[](int i) {
        return m_Items[i];
    }

    T &operator[](int i) const {
        return m_Items[i];
    }
};

template<typename T>
using Array = Il2CppArray<T>;

struct Il2CppString {
    Il2CppClass *klass;
    void *monitor;
    int32_t length;
    uint16_t start_char;

    const char *CString();

    const wchar_t *WCString();

    static Il2CppString *Create(const char *s);
    static Il2CppString *Create(const wchar_t *s, int len);

    int getLength() {
        return length;
    }
};

const char* Il2CppString::CString() {
    return utf16_to_utf8(&this->start_char, this->length);
}

const wchar_t* Il2CppString::WCString() {
    return utf16_to_utf32(&this->start_char, this->length);
}

Il2CppString *Il2CppString::Create(const char *s) {
    return il2cpp_string_new(s);
}

Il2CppString *Il2CppString::Create(const wchar_t *s, int len) {
    return il2cpp_string_new_utf16(s, len);
}

typedef Il2CppString String;

template<typename T> struct Il2CppList {
    Il2CppClass *klass;
    void *unk1;
    Il2CppArray<T> *items;
    int size;
    int version;

    T *getItems() {
        return items->getPointer();
    }

    int getSize() {
        return size;
    }

    int getVersion() {
        return version;
    }

    T &operator[](int i) {
        return items->m_Items[i];
    }

    T &operator[](int i) const {
        return items->m_Items[i];
    }
};

template<typename T>
using List = Il2CppList<T>;

template<typename K, typename V> struct Il2CppDictionary {
    Il2CppClass *klass;
    void *unk1;
    Il2CppArray<int **> *table;
    Il2CppArray<void **> *linkSlots;
    Il2CppArray<K> *keys;
    Il2CppArray<V> *values;
    int touchedSlots;
    int emptySlot;
    int size;

    K *getKeys() {
        return keys->getPointer();
    }

    V *getValues() {
        return values->getPointer();
    }

    int getNumKeys() {
        return keys->getLength();
    }

    int getNumValues() {
        return values->getLength();
    }

    int getSize() {
        return size;
    }
};

template<typename K, typename V>
using Dictionary = Il2CppDictionary<K, V>;

void Il2CppAttach(const char *name="libil2cpp.so");
void *Il2CppGetImageByName(const char *image);
void *Il2CppGetClassType(const char *image, const char *namespaze, const char *clazz);
void *Il2CppCreateClassInstance(const char *image, const char *namespaze, const char *clazz);
void* Il2CppCreateArray(const char *image, const char *namespaze, const char *clazz, size_t length);

void Il2CppGetStaticFieldValue(const char *image, const char *namespaze, const char *clazz, const char *name, void *output);
void Il2CppSetStaticFieldValue(const char *image, const char *namespaze, const char *clazz, const char *name, void* value);

void *Il2CppGetMethodOffset(const char *image, const char *namespaze, const char *clazz, const char *name, int argsCount = 0);
void *Il2CppGetMethodOffset(const char *image, const char *namespaze, const char *clazz, const char *name, char** args, int argsCount);

size_t Il2CppGetFieldOffset(const char *image, const char *namespaze, const char *clazz, const char *name);
size_t Il2CppGetStaticFieldOffset(const char *image, const char *namespaze, const char *clazz, const char *name);

bool Il2CppIsAssembliesLoaded();

void *Il2CppGetImageByName(const char *image) {
	size_t size;
	void **assemblies = il2cpp_domain_get_assemblies(il2cpp_domain_get(), &size);
	for(int i = 0; i < size; ++i) {
		void *img = (void *)il2cpp_assembly_get_image(assemblies[i]);
		const char *img_name = il2cpp_image_get_name(img);
		if (strcmp(img_name, image) == 0) {
			return img;
		}
	}
	return 0;
}

void *Il2CppGetClassType(const char *image, const char *namespaze, const char *clazz) {
	static std::map<std::string, void *> cache;
	std::string s = image;
	s += namespaze;
	s += clazz;
	if (cache.count(s) > 0)
		return cache[s];
	void *img = Il2CppGetImageByName(image);
	if (!img) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find image %s!", image);
		return 0;
	}
	void *klass = il2cpp_class_from_name(img, namespaze, clazz);
	if (!klass) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find class %s!", clazz);
		return 0;
	}
	cache[s] = klass;
	return klass;
}

void *Il2CppCreateClassInstance(const char *image, const char *namespaze, const char *clazz) {
	void *img = Il2CppGetImageByName(image);
	if (!img) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find image %s!", image);
		return 0;
	}
	void *klass = Il2CppGetClassType(image, namespaze, clazz);
	if (!klass) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find class %s!", clazz);
		return 0;
	}
	void *obj = il2cpp_object_new(klass);
	if (!obj) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't create object %s!", clazz);
		return 0;
	}
	return obj;
}

void* Il2CppCreateArray(const char *image, const char *namespaze, const char *clazz, size_t length) {
	void *img = Il2CppGetImageByName(image);
	if (!img) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find image %s!", image);
		return 0;
	}
	void *klass = Il2CppGetClassType(image, namespaze, clazz);
	if (!klass) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find class %s!", clazz);
		return 0;
	}
	return il2cpp_array_new(klass, length);
}

void Il2CppGetStaticFieldValue(const char *image, const char *namespaze, const char *clazz, const char *name, void *output) {
	void *img = Il2CppGetImageByName(image);
	if (!img) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find image %s!", image);
		return;
	}
	void *klass = Il2CppGetClassType(image, namespaze, clazz);
	if (!klass) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find field %s!",name);
		return;
	}
    void *field = il2cpp_class_get_field_from_name(klass, name);
	if (!field) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find field %s in class %s!", name, clazz);
		return;
	}
	il2cpp_field_static_get_value(field, output);
}

void Il2CppSetStaticFieldValue(const char *image, const char *namespaze, const char *clazz, const char *name, void* value) {
	void *img = Il2CppGetImageByName(image);
	if (!img) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find image %s!", image);
		return;
	}
	void *klass = Il2CppGetClassType(image, namespaze, clazz);
	if (!klass) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find field %s!", name);
		return;
	}
	void *field = il2cpp_class_get_field_from_name(klass, name);
	if (!field) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find field %s in class %s!", name, clazz);
		return;
	}
	il2cpp_field_static_set_value(field, value);
}

void *Il2CppGetMethodOffset(const char *image, const char *namespaze, const char *clazz, const char *name, int argsCount) {
	void *img = Il2CppGetImageByName(image);
	if (!img) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find image %s!", image);
		return 0;
	}
	void *klass = Il2CppGetClassType(image, namespaze, clazz);
	if (!klass) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find method %s!", name);
		return 0;
	}
	void **method = (void**)il2cpp_class_get_method_from_name(klass, name, argsCount);
	if (!method) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find method %s in class %s!", name, clazz);
		return 0;
	}
	__android_log_print(ANDROID_LOG_DEBUG, g_LogTag, "%s - [%s] %s::%s: %p", image, namespaze, clazz, name, *method);
	return *method;
}

void *Il2CppGetMethodOffset(const char *image, const char *namespaze, const char *clazz, const char *name, char** args, int argsCount) {
	void *img = Il2CppGetImageByName(image);
	if (!img) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find image %s!", image);
		return 0;
	}
	void *klass = Il2CppGetClassType(image, namespaze, clazz);
	if (!klass) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find class %s for method %s!", clazz, name);
		return 0;
	}
	void *iter = 0;
	int score = 0;
	void **method = (void**) il2cpp_class_get_methods(klass, &iter);
	
	while(method) {
		const char *fname = il2cpp_method_get_name(method);
		if (strcmp(fname, name) == 0) {
			for (int i = 0; i < argsCount; i++) {
				void *arg = il2cpp_method_get_param(method, i);
				if (arg) {
					const char *tname = il2cpp_type_get_name(arg);
					if (strcmp(tname, args[i]) == 0) {
						score++;
					} else {
						__android_log_print(ANDROID_LOG_INFO, g_LogTag, "Argument at index %d didn't matched requested argument!\n\tRequested: %s\n\tActual: %s\nnSkipping function...", i, args[i], tname);
						score = 0;
						goto skip;
					}
				}
			}
		}
		skip:
		if (score == argsCount) {
			__android_log_print(ANDROID_LOG_DEBUG, g_LogTag, "%s - [%s] %s::%s: %p", image, namespaze, clazz, name, *method);
			return *method;
		}
		method = (void **) il2cpp_class_get_methods(klass, &iter);
	}
	__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Cannot find function %s in class %s!", name, clazz);
	return 0;
}

size_t Il2CppGetFieldOffset(const char *image, const char *namespaze, const char *clazz, const char *name) {
	void *img = Il2CppGetImageByName(image);
	if (!img) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find image %s!", image);
		return -1;
	}
	void *klass = Il2CppGetClassType(image, namespaze, clazz);
	if (!klass) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find field %s!", name);
		return -1;
	}
	void *field = il2cpp_class_get_field_from_name(klass, name);
	if (!field) {
		__android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find field %s in class %s!", clazz, name);
		return -1;
	}
	auto result = il2cpp_field_get_offset(field);
	__android_log_print(ANDROID_LOG_DEBUG, g_LogTag, "%s - [%s] %s::%s: %p", image, namespaze, clazz, name, (void *) result);
	return result;
}

size_t Il2CppGetStaticFieldOffset(const char *image, const char *namespaze, const char *clazz, const char *name){
    void *img = Il2CppGetImageByName(image);
    if(!img) {
        __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find image %s!", image);
        return -1;
    }
    void *klass = Il2CppGetClassType(image, namespaze, clazz);
    if(!klass) {
        __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find field %s!", name);
        return -1;
    }

    FieldInfo *field = (FieldInfo*)il2cpp_class_get_field_from_name(klass, name);
    if(!field) {
        __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Can't find field %s in class %s!", clazz, name);
        return -1;
    }
    return (unsigned long)((uint64_t)field->parent->static_fields + field->offset);
}

bool Il2CppIsAssembliesLoaded() {
	size_t size;
	void **assemblies = il2cpp_domain_get_assemblies(il2cpp_domain_get(), &size);
	return size != 0 && assemblies != 0;
}

void Il2CppAttach(const char *name) {
	void *handle = xdl_open(name, 0);
	while (!handle) {
		handle = xdl_open(name, 0);
		sleep(1);
	}
	il2cpp_assembly_get_image = (const void *(*)(const void *)) xdl_sym(handle, "il2cpp_assembly_get_image",0);
    il2cpp_domain_get = (void *(*)()) xdl_sym(handle, "il2cpp_domain_get",0);
    il2cpp_domain_get_assemblies = (void **(*)(const void* , size_t*)) xdl_sym(handle, "il2cpp_domain_get_assemblies",0);
    il2cpp_image_get_name = (const char *(*)(void *)) xdl_sym(handle, "il2cpp_image_get_name",0);
    il2cpp_class_from_name = (void* (*)(const void*, const char*, const char *)) xdl_sym(handle, "il2cpp_class_from_name",0);
    il2cpp_class_get_field_from_name = (void* (*)(void*, const char *)) xdl_sym(handle, "il2cpp_class_get_field_from_name",0);
    il2cpp_class_get_method_from_name = (void* (*)(void *, const char*, int)) xdl_sym(handle, "il2cpp_class_get_method_from_name",0);
    il2cpp_field_get_offset = (size_t (*)(void *)) xdl_sym(handle, "il2cpp_field_get_offset",0);
    il2cpp_field_static_get_value = (void (*)(void*, void *)) xdl_sym(handle, "il2cpp_field_static_get_value",0);
    il2cpp_field_static_set_value = (void (*)(void*, void *)) xdl_sym(handle, "il2cpp_field_static_set_value",0);
    il2cpp_array_new = (void *(*)(void*, size_t)) xdl_sym(handle, "il2cpp_array_new",0);
    il2cpp_string_chars = (uint16_t *(*)(void*)) xdl_sym(handle, "il2cpp_string_chars",0);
    il2cpp_string_new = (Il2CppString *(*)(const char *)) xdl_sym(handle, "il2cpp_string_new",0);
    il2cpp_string_new_utf16 = (Il2CppString *(*)(const wchar_t *, int32_t)) xdl_sym(handle, "il2cpp_string_new_utf16",0);
    il2cpp_type_get_name = (char *(*)(void *)) xdl_sym(handle, "il2cpp_type_get_name",0);
    il2cpp_method_get_param = (void *(*)(void *, uint32_t)) xdl_sym(handle, "il2cpp_method_get_param",0);
    il2cpp_class_get_methods = (void *(*)(void *, void **)) xdl_sym(handle, "il2cpp_class_get_methods",0);
    il2cpp_method_get_name = (const char *(*)(void *)) xdl_sym(handle, "il2cpp_method_get_name",0);
    il2cpp_object_new = (void *(*)(void *)) xdl_sym(handle, "il2cpp_object_new",0);
    xdl_close(handle);
}

#endif