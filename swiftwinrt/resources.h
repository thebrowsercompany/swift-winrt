#pragma once

// Header file shared with the resource compiler
#define RESOURCE_TYPE_SWIFT_FILE SWIFT_FILE
#define RESOURCE_TYPE_SWIFT_FILE_STR "SWIFT_FILE"

#ifndef RC_INVOKED

#include <span>

namespace swiftwinrt
{
    // Gets all of a win32 module's resources of a given type as text strings.
    static std::map<std::string, std::span<const std::byte>> get_named_resources_of_type(
        HMODULE hModule, LPCSTR type, bool make_lowercase = false)
    {
        struct closure_type
        {
            std::map<std::string, std::span<const std::byte>> resources;
            bool make_lowercase = make_lowercase;
        } closure;

        EnumResourceNamesA(hModule, type,
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

                auto res_handle = FindResourceA(hModule, lpName, lpType);
                if (res_handle == 0)
                {
                    assert(!"Resource lookup failure.");
                    return TRUE; // Skip this resource
                }

                auto data_handle = LoadResource(hModule, res_handle);
                if (data_handle == 0)
                {
                    assert(!"Resource load failure.");
                    return TRUE; // Skip this resource
                }

                // From Microsoft docs:
                //   The pointer returned by LockResource is valid until the module containing the resource is unloaded.
                //   It is not necessary to unlock resources because the system automatically deletes them when the process that created them terminates.
                auto text_ptr = reinterpret_cast<const std::byte*>(LockResource(data_handle));
                auto text_size = static_cast<size_t>(SizeofResource(hModule, res_handle));
                if (text_ptr == nullptr)
                {
                    assert(!"Resource lock failure.");
                    return TRUE; // Skip this resource
                }
                
                // Resources always have ALL_CAPS names (assume ASCII)
                std::string res_name{ lpName };
                if (closure.make_lowercase)
                {
                    std::transform(res_name.begin(), res_name.end(), res_name.begin(),
                        [](unsigned char c) { return std::tolower(c); });
                }

                closure.resources.emplace(std::move(res_name), std::span(text_ptr, text_size));
                return TRUE;
            },
            (LONG_PTR)&closure);

        return std::move(closure.resources);
    }
}

#endif
