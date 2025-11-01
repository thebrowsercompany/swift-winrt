#include "pch.h"

#include "types/element_type.h"

#include "file_writers/abi_writer.h"
#include "utility/metadata_helpers.h"

using namespace std::literals;
using namespace winmd::reader;

namespace swiftwinrt
{
    element_type::element_type(
        ElementType element_type,
        std::string_view swift_name,
        std::string_view logical_name,
        std::string_view abi_name,
        std::string_view cpp_name,
        std::string_view mangled_name,
        std::string_view signature) :
        m_swift_name(swift_name),
        m_logical_name(logical_name),
        m_abi_name(abi_name),
        m_cpp_name(cpp_name),
        m_mangled_name(mangled_name),
        m_signature(signature),
        m_type(element_type)
    {
    }

    element_type const& element_type::from_type(ElementType type)
    {
        static element_type const boolean_type{ ElementType::Boolean, "Bool"sv, "bool"sv, "boolean"sv, "boolean"sv, "boolean"sv, "b1"sv };
        static element_type const char_type{ ElementType::Char, "Character"sv, "wchar_t"sv, "wchar_t"sv, "WCHAR"sv, "wchar__zt"sv, "c2"sv };
        static element_type const u1_type{ ElementType::U1, "UInt8"sv, "::byte"sv, "::byte"sv, "BYTE"sv, "byte"sv, "u1"sv };
        static element_type const i2_type{ ElementType::I2, "Int16"sv, "short"sv, "short"sv, "INT16"sv, "short"sv, "i2"sv };
        static element_type const u2_type{ ElementType::U2, "UInt16"sv, "UINT16"sv, "UINT16"sv, "UINT16"sv, "UINT16"sv, "u2"sv };
        static element_type const i4_type{ ElementType::I4, "Int32"sv, "int"sv, "int"sv, "INT32"sv, "int"sv, "i4"sv };
        static element_type const u4_type{ ElementType::U4, "UInt32"sv, "UINT32"sv, "UINT32"sv, "UINT32"sv, "UINT32"sv, "u4"sv };
        static element_type const i8_type{ ElementType::I8, "Int64"sv, "__int64"sv, "__int64"sv, "INT64"sv, "__z__zint64"sv, "i8"sv };
        static element_type const u8_type{ ElementType::U8, "UInt64"sv, "UINT64"sv, "UINT64"sv, "UINT64"sv, "UINT64"sv, "u8"sv };
        static element_type const r4_type{ ElementType::R4, "Float"sv, "float"sv, "float"sv, "FLOAT"sv, "float"sv, "f4"sv };
        static element_type const r8_type{ ElementType::R8, "Double"sv, "double"sv, "double"sv, "DOUBLE"sv, "double"sv, "f8"sv };
        static element_type const string_type{ ElementType::String, "String"sv, "HSTRING"sv, "HSTRING"sv, "HSTRING"sv, "HSTRING"sv, "string"sv };
        static element_type const object_type{ ElementType::Object, "Any"sv, "IInspectable"sv, "IInspectable"sv, "IInspectable*"sv, "IInspectable"sv, "cinterface(IInspectable)"sv };

        switch (type)
        {
        case ElementType::Boolean: return boolean_type;
        case ElementType::Char: return char_type;
        case ElementType::U1: return u1_type;
        case ElementType::I2: return i2_type;
        case ElementType::U2: return u2_type;
        case ElementType::I4: return i4_type;
        case ElementType::U4: return u4_type;
        case ElementType::I8: return i8_type;
        case ElementType::U8: return u8_type;
        case ElementType::R4: return r4_type;
        case ElementType::R8: return r8_type;
        case ElementType::String: return string_type;
        case ElementType::Object: return object_type;
        default: swiftwinrt::throw_invalid("Unrecognized ElementType: ", std::to_string(static_cast<int>(type)));
        }
    }

    bool element_type::is_blittable() const
    {
        switch (m_type)
        {
        case ElementType::Boolean:
        case ElementType::Char:
        case ElementType::String:
        case ElementType::Object:
            return false;
        default:
            return true;
        }
    }

    void element_type::write_c_abi_param(writer& w) const
    {
        w.write(m_cpp_name);
    }
}
