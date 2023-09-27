#pragma once

// Header file shared with the resource compiler
#define RESOURCE_TYPE_SWIFT_SUPPORT_FILE SWIFT_SUPPORT_FILE
#define RESOURCE_TYPE_SWIFT_SUPPORT_FILE_STR "SWIFT_SUPPORT_FILE"

#define RESOURCE_TYPE_C_INCLUDE_FILE C_HEADER_FILE
#define RESOURCE_TYPE_C_INCLUDE_FILE_STR "C_HEADER_FILE"

#define RESOURCE_TYPE_OTHER_FILE OTHER_FILE
#define RESOURCE_TYPE_OTHER_FILE_STR "OTHER_FILE"

#define RESOURCE_NAME_CWINRT_SHIM_C CWINRT_SHIM_C
#define RESOURCE_NAME_CWINRT_SHIM_C_STR "CWINRT_SHIM_C"

#define RESOURCE_NAME_CWINRT_PACKAGE_SWIFT CWINRT_PACKAGE_SWIFT
#define RESOURCE_NAME_CWINRT_PACKAGE_SWIFT_STR "CWINRT_PACKAGE_SWIFT"

#define RESOURCE_NAME_CWINRT_PACKAGE_SWIFT CWINRT_PACKAGE_SWIFT
#define RESOURCE_NAME_CWINRT_PACKAGE_SWIFT_STR "CWINRT_PACKAGE_SWIFT"

#define RESOURCE_NAME_CWINRT_CPPINTEROPWORKAROUND_H CWINRT_CPPINTEROPWORKAROUND
#define RESOURCE_NAME_CWINRT_CPPINTEROPWORKAROUND_H_STR "CWINRT_CPPINTEROPWORKAROUND"

#define RESOURCE_NAME_CWINRT_IVECTORCHANGEDEVENTARGS_H CWINRT_IVECTORCHANGEDEVENTARGS
#define RESOURCE_NAME_CWINRT_IVECTORCHANGEDEVENTARGS_H_STR "CWINRT_IVECTORCHANGEDEVENTARGS"

#ifndef RC_INVOKED

#include <span>

namespace swiftwinrt
{
    static std::span<const std::byte> find_resource(
        HMODULE hModule, LPCSTR type, LPCSTR name)
    {
        auto res_handle = FindResourceA(hModule, name, type);
        if (res_handle == 0)
        {
            assert(!"Resource lookup failure.");
            return {};
        }

        auto data_handle = LoadResource(hModule, res_handle);
        if (data_handle == 0)
        {
            assert(!"Resource load failure.");
            return {};
        }

        // From Microsoft docs:
        //   The pointer returned by LockResource is valid until the module containing the resource is unloaded.
        //   It is not necessary to unlock resources because the system automatically deletes them when the process that created them terminates.
        auto text_ptr = reinterpret_cast<const std::byte*>(LockResource(data_handle));
        auto text_size = static_cast<size_t>(SizeofResource(hModule, res_handle));
        if (text_ptr == nullptr)
        {
            assert(!"Resource lock failure.");
            return {};
        }

        return std::span(text_ptr, text_size);
    }

    static std::span<const std::byte> find_resource(LPCSTR type, LPCSTR name)
    {
        return find_resource(GetModuleHandle(NULL), type, name);
    }

    // Gets all of a win32 module's resources of a given type as byte spans, keyed by name.
    static std::map<std::string, std::span<const std::byte>> get_named_resources_of_type(
        HMODULE hModule, LPCSTR type, bool make_lowercase = false)
    {
        struct closure_type
        {
            std::map<std::string, std::span<const std::byte>> resources;
            bool make_lowercase = make_lowercase;
        } closure;

        EnumResourceNamesExA(hModule, type,
            [](HMODULE hModule, LPCSTR lpType, LPSTR lpName, LONG_PTR lParam) -> BOOL
            {
                auto& closure = *(closure_type*)lParam;

                // This method should be used with resource types indexed by strings,
                // bail out for built-in resource types indexed by integers.
                if (IS_INTRESOURCE(lpName))
                {
                    assert(!"Unexpected integer-named resource.");
                    return TRUE; // Skip this resource
                }

                auto res_data = find_resource(hModule, lpType, lpName);

                // Resources always have ALL_CAPS names (assume ASCII)
                std::string res_name{ lpName };
                if (closure.make_lowercase)
                {
                    std::transform(res_name.begin(), res_name.end(), res_name.begin(),
                        [](unsigned char c) { return std::tolower(c); });
                }

                closure.resources.emplace(std::move(res_name), res_data);
                return TRUE;
            },
            (LONG_PTR)&closure, /* dwFlags: */ 0, /* LangId: */ 0);

        return std::move(closure.resources);
    }
}

#endif
