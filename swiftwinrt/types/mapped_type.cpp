#include "pch.h"

#include "types/mapped_type.h"

#include "file_writers/abi_writer.h"
#include "utility/metadata_helpers.h"
#include "utility/type_helpers.h"
#include "types/type_constants.h"

using namespace std::literals;
using namespace winmd::reader;

namespace swiftwinrt
{
    mapped_type::mapped_type(
        TypeDef const& type,
        std::string_view cpp_name,
        std::string_view mangled_name,
        std::string_view signature) :
        m_type(type),
        m_swift_full_name(swiftwinrt::get_full_type_name(type)),
        m_cpp_name(cpp_name),
        m_mangled_name(mangled_name),
        m_signature(signature)
    {
    }

    mapped_type const* mapped_type::from_typedef(TypeDef const& type)
    {
        if (type.TypeNamespace() == winrt_foundation_namespace)
        {
            if (type.TypeName() == "HResult"sv)
            {
                static mapped_type const hresult_type{ type, "HRESULT"sv, "HRESULT"sv, "struct(Windows.Foundation.HResult;i4)"sv };
                return &hresult_type;
            }
            else if (type.TypeName() == "EventRegistrationToken"sv)
            {
                static mapped_type event_token_type{ type, "EventRegistrationToken"sv, "EventRegistrationToken"sv, "struct(Windows.Foundation.EventRegistrationToken;i8)"sv };
                return &event_token_type;
            }
            else if (type.TypeName() == "AsyncStatus"sv)
            {
                static mapped_type const async_status_type{ type, "AsyncStatus"sv, "AsyncStatus"sv, "enum(Windows.Foundation.AsyncStatus;i4)"sv };
                return &async_status_type;
            }
            else if (type.TypeName() == "IAsyncInfo"sv)
            {
                static mapped_type const async_info_type{ type, "IAsyncInfo"sv, "IAsyncInfo"sv, "{00000036-0000-0000-c000-000000000046}"sv };
                return &async_info_type;
            }
        }

        return nullptr;
    }

    void mapped_type::write_c_abi_param(writer& w) const
    {
        w.write(m_cpp_name);

        auto type_category = get_category(m_type);
        if ((type_category == category::delegate_type) ||
            (type_category == category::interface_type) ||
            (type_category == category::class_type))
        {
            w.write('*');
        }
    }

    std::optional<std::size_t> mapped_type::contract_index(std::string_view type_name, std::size_t) const
    {
        if (type_name != "Windows.Foundation.UniversalApiContract"sv)
        {
            return std::nullopt;
        }

        return 0;
    }

    std::optional<contract_version> mapped_type::contract_from_index(std::size_t index) const
    {
        XLANG_ASSERT(index == 0);
        return contract_version{ "Windows.Foundation.UniversalApiContract"sv, 1 };
    }
}
