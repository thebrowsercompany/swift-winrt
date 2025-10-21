#pragma once
#include "code_writers/common_writers.h"
#include "code_writers/type_writers.h"
#include "code_writers/can_write.h"
#include "utility/separator.h"
#include "utility/swift_codegen_utils.h"
#include "utility/metadata_cache.h"
namespace swiftwinrt
{
    static void write_guid_value(writer& w, std::vector<FixedArgSig> const& args)
    {
        using std::get;

        w.write_printf("Data1: 0x%08X, Data2: 0x%04X, Data3: 0x%04X, Data4: ( 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X )",
            get<uint32_t>(get<ElemSig>(args[0].value).value),
            get<uint16_t>(get<ElemSig>(args[1].value).value),
            get<uint16_t>(get<ElemSig>(args[2].value).value),
            get<uint8_t>(get<ElemSig>(args[3].value).value),
            get<uint8_t>(get<ElemSig>(args[4].value).value),
            get<uint8_t>(get<ElemSig>(args[5].value).value),
            get<uint8_t>(get<ElemSig>(args[6].value).value),
            get<uint8_t>(get<ElemSig>(args[7].value).value),
            get<uint8_t>(get<ElemSig>(args[8].value).value),
            get<uint8_t>(get<ElemSig>(args[9].value).value),
            get<uint8_t>(get<ElemSig>(args[10].value).value));
    }

    static void write_guid_comment(writer& w, std::vector<FixedArgSig> const& args)
    {
        using std::get;

        w.write_printf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            get<uint32_t>(get<ElemSig>(args[0].value).value),
            get<uint16_t>(get<ElemSig>(args[1].value).value),
            get<uint16_t>(get<ElemSig>(args[2].value).value),
            get<uint8_t>(get<ElemSig>(args[3].value).value),
            get<uint8_t>(get<ElemSig>(args[4].value).value),
            get<uint8_t>(get<ElemSig>(args[5].value).value),
            get<uint8_t>(get<ElemSig>(args[6].value).value),
            get<uint8_t>(get<ElemSig>(args[7].value).value),
            get<uint8_t>(get<ElemSig>(args[8].value).value),
            get<uint8_t>(get<ElemSig>(args[9].value).value),
            get<uint8_t>(get<ElemSig>(args[10].value).value));
    }

    static void write_guid_value_hash(writer& w, std::array<uint8_t, 20ui64> const& iidHash)
    {
        w.write_printf("Data1: 0x%02x%02x%02x%02x, Data2: 0x%02x%02x, Data3: 0x%02x%02x, Data4: ( 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x )",
            iidHash[0], iidHash[1], iidHash[2], iidHash[3],
            iidHash[4], iidHash[5],
            iidHash[6], iidHash[7],
            iidHash[8], iidHash[9],
            iidHash[10], iidHash[11], iidHash[12], iidHash[13], iidHash[14], iidHash[15]);
    }

    static void write_guid_comment_hash(writer& w, std::array<uint8_t, 20ui64> const& iidHash)
    {
        w.write_printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            iidHash[0], iidHash[1], iidHash[2], iidHash[3],
            iidHash[4], iidHash[5],
            iidHash[6], iidHash[7],
            iidHash[8], iidHash[9],
            iidHash[10], iidHash[11], iidHash[12], iidHash[13], iidHash[14], iidHash[15]);
    }

    static void write_array_size_name(writer& w, Param const& param)
    {
        w.write(" __%Size", get_swift_name(param));
    }
    static void write_function_params2(writer& w, std::vector<function_param> const& params, write_type_params const& type_params)
    {
        separator s{ w };

        for (const auto& param : params)
        {
            s();
            w.write("_ %: ", get_swift_name(param));
            if (param.out()) w.write("inout ");
            const bool is_array = param.is_array();
            if (is_array && type_params.layer == projection_layer::swift)
            {
                // Can't allow for implicit unwrap in arrays
                w.write("[%]", bind<write_type>(*param.type, write_type_params::swift));
            }
            else
            {
                write_type(w, *param.type, type_params);
            }
        }
    }
    static void write_function_params(writer& w, function_def const& function, write_type_params const& type_params)
    {
        write_function_params2(w, function.params, type_params);
    }

    template <typename Param>
    static void write_convert_to_abi_arg(writer& w, Param const& param)
    {
        TypeDef signature_type;
        auto type = param.type;
        auto param_name = param.name;
        auto is_out = param.out();

        auto category = get_category(type, &signature_type);

        auto local_name = local_swift_param_name(param_name);
        if (param.is_array())
        {
            if (is_out)
            {
                w.write("%");
            }
            else
            {
                // Arrays are all converted from the swift array to a c array, so they
                // use the local_param_name
                w.write("count, %", local_name);
            }
        }
        else if (category == param_category::object_type)
        {
            if (is_out) throw std::exception("out parameters of reference types should not be converted directly to abi types");

            if (is_class(signature_type))
            {
                w.write("RawPointer(%)", param_name);
            }
            else
            {
                w.write("%", local_name);
            }
        }
        else if (category == param_category::string_type)
        {
            if (!is_out)
            {
                w.write("%.get()", local_name);
            }
            else
            {
                auto format = "try! HString(%).detach()";
                w.write(format, param_name);
            }
        }
        else if (category == param_category::struct_type)
        {
            if (!is_out && type->swift_type_name() == "EventRegistrationToken")
            {
                w.write(param_name);
            }
            else if (!is_out && param.by_ref())
            {
                if (is_struct_blittable(signature_type))
                {
                    w.write("&%", local_name);
                }
                else
                {
                    w.write("&%.val", local_name);
                }
            } 
            else if (is_struct_blittable(signature_type))
            {
                w.write(".from(swift: %)", param_name);
            }
            else
            {
                if (!is_out)
                {
                    w.write("%.val", local_name);
                }
                else
                {
                    w.write("%.detach()", local_name);
                }
            }
        }
        else if (category == param_category::generic_type)
        {
            if (is_out) throw std::exception("out parameters of generic types should not be converted directly to abi types");
            // When passing generics to the ABI we wrap them before making the
            // api call for easy passing to the ABI
            w.write("%", local_name);
        }
        else if (is_type_blittable(category))
        {
            // fundamentals and enums can be simply copied
            w.write(param_name);
        }
        else if (param.by_ref())
        {
            w.write("&%", local_name);
        } 
        else
        {
            w.write(".init(from: %)", param_name);
        }
    }

    static void write_implementation_args(writer& w, function_def const& function)
    {
        separator s{ w };

        for (const auto& param : function.params)
        {
            s();
            if (param.def.Flags().In())
            {
                w.write(get_swift_name(param));
            }
            else
            {
                w.write("&%", get_swift_name(param));
            }
        }
    }

    static void write_abi_args(writer& w, function_def const& function)
    {
        separator s{ w };

        w.write("pThis");
        s();
        for (auto& param: function.params)
        {
            auto param_name = get_swift_name(param);
            auto local_param_name = local_swift_param_name(param_name);
            s();
            if (param.is_array())
            {
                if (param.in())
                {
                    w.write("%.count, %.start", local_param_name, local_param_name);
                }
                else if (param.signature.ByRef())
                {
                    w.write("&%.count, &%.start", local_param_name, local_param_name);
                }
                else
                {
                    w.write("%.count, %.start", local_param_name, local_param_name);
                }
            }
            else if (param.in())
            {
                write_convert_to_abi_arg(w, param);
            }
            else
            {
                auto category = get_category(param.type);
                bool is_blittable = is_type_blittable(param.signature.Type());
                if (needs_wrapper(category))
                {
                    w.write("&%Abi", local_param_name);
                }
                else if (category == param_category::struct_type)
                {
                    if (is_blittable)
                    {
                        w.write("&%", local_param_name);
                    }
                    else
                    {
                        w.write("&%.val", local_param_name);
                    }
                }
                else if (is_blittable)
                {
                    w.write("&%", param_name);
                }
                else
                {
                    w.write("&%", local_param_name);
                }
            }
        }

        if (function.return_type)
        {
            s();
            auto param_name = function.return_type.value().name;
            if (function.return_type.value().is_array())
            {
                w.write("&%.count, &%.start", param_name, param_name);
            }
            else if (needs_wrapper(get_category(function.return_type->type)))
            {
                w.write("&%Abi", param_name);
            }
            else
            {
                w.write("&%", param_name);
            }
        }
    }

    static std::optional<writer::indent_guard> write_init_return_val_abi(writer& w, function_return_type const& signature)
    {
        auto category = get_category(signature.type);
        if (signature.is_array())
        {
            w.write("var %: WinRTArrayAbi<%> = (0, nil)\n",
                signature.name,
                bind<write_type>(*signature.type, write_type_params::c_abi));
            return std::optional<writer::indent_guard>();
        }
        else if (needs_wrapper(category))
        {
            w.write("let (%) = try ComPtrs.initialize { %Abi in\n", signature.name, signature.name);
            return writer::indent_guard(w, 1);
        }
        else
        {
            w.write("var %: ", signature.name);
            auto guard{ w.push_mangled_names_if_needed(category) };
            write_type(w, *signature.type, write_type_params::c_abi);
            write_default_init_assignment(w, *signature.type, projection_layer::c_abi);
            w.write("\n");
            return std::optional<writer::indent_guard>();
        }
    }

    static void write_consume_return_statement(writer& w, function_def const& signature);
    static void write_return_type_declaration(writer& w, function_def function, write_type_params const& type_params)
    {
        if (!function.return_type)
        {
            return;
        }

        w.write(" -> ");
        if (function.return_type->is_array() && type_params.layer == projection_layer::swift)
        {
            w.write("[%]", bind<write_type>(*function.return_type->type, write_type_params::swift));
        }
        else
        {
            write_type(w, *function.return_type->type, type_params);
        }
    }

    static std::vector<function_param> get_projected_params(attributed_type const& factory, function_def const& func);
    static write_scope_guard<writer> write_local_param_wrappers(writer& w, std::vector<function_param> const& params);

    static void write_comma_param_names(writer& w, std::vector<function_param> const& params);

    static void write_vtable(writer& w, delegate_type const& type);

    static void write_convert_vtable_params(writer& w, function_def const& signature)
    {
        int param_number = 1;
        auto full_type_names = w.push_full_type_names(true);

        for (auto& param : signature.params)
        {
            std::string param_name = "$" + std::to_string(param_number);
            if (param.is_array())
            {
                auto array_param_name = "$" + std::to_string(param_number + 1);
                auto count_param_name = param_name;
                if (param.in())
                {
                    w.write("let %: [%] = %\n",
                        get_swift_name(param),
                        bind<write_type>(*param.type, write_type_params::swift),
                        bind<write_convert_array_from_abi>(*param.type, w.write_temp("(count: %, start: %)", count_param_name, array_param_name)));
                }
                else if (param.signature.ByRef())
                {
                    w.write("var % = [%]()\n",
                        get_swift_name(param),
                        bind<write_type>(*param.type, write_type_params::swift));
                }
                else
                {
                    w.write("var %: [%] = %\n",
                        get_swift_name(param),
                        bind<write_type>(*param.type, write_type_params::swift),
                        bind<write_convert_array_from_abi>(*param.type, w.write_temp("(count: %, start: %)", count_param_name, array_param_name)));
                }
                ++param_number;
            }
            else if (param.in())
            {
                assert(!param.out());

                if (is_delegate(param.type))
                {
                    w.write("guard let % = % else { return E_INVALIDARG }\n",
                        get_swift_name(param),
                        bind<write_consume_type>(param.type, param_name, false));
                }
                else
                {
                    if (param.signature.ByRef())
                    {
                        param_name += "!.pointee";
                    }
                    w.write("let %: % = %\n",
                        get_swift_name(param),
                        bind<write_type>(*param.type, write_type_params::swift),
                        bind<write_consume_type>(param.type, param_name, false));
                }
            }
            else
            {
                assert(!param.in());
                assert(param.out());
                w.write("var %: %%\n",
                    get_swift_name(param),
                    bind<write_type>(*param.type, write_type_params::swift),
                    bind<write_default_init_assignment>(*param.type, projection_layer::swift));
                }

            ++param_number;
        }
    }

    static void write_consume_return_statement(writer& w, function_def const& signature)
    {
        if (!signature.return_type)
        {
            return;
        }

        auto return_type = signature.return_type.value();
        auto return_param_name = put_in_backticks_if_needed(std::string(return_type.name));
        if (return_type.is_array())
        {
            w.write("defer { CoTaskMemFree(%.start) }\n", return_param_name);
            w.write("return %\n", bind<write_convert_array_from_abi>(*return_type.type, return_param_name));
        }
        else
        {
            w.write("return %", bind<write_consume_type>(return_type.type, return_param_name, true));
        }

    }

    static void write_consume_args(writer& w, function_def const& function)
    {
        separator s{ w };

        for (auto& param : function.params)
        {
            s();
            if (param.in())
            {
                w.write(get_swift_name(param));
            }
            else
            {
                w.write("&%", get_swift_name(param));
            }
        }
    }

    static void write_property_value_impl(writer& w)
    {
        auto winrtInterfaceConformance = w.write_temp(R"(
    public func queryInterface(_ iid: %.IID) -> IUnknownRef? {
        guard iid == __ABI_Windows_Foundation.IPropertyValueWrapper.IID else { return nil }
        guard let thisAsIPropValue = __ABI_Windows_Foundation.IPropertyValueWrapper(self) else { fatalError("creating non-nil wrapper shouldn't fail") }
        return thisAsIPropValue.queryInterface(iid)
    }
)", w.support);

        w.write(R"(public class IPropertyValueImpl : IPropertyValue, IReference {
    public typealias T = Any
    var _value: Any
    var propertyType : PropertyType

    fileprivate init(_ abi: ComPtr<__x_ABI_CWindows_CFoundation_CIPropertyValue>) { fatalError("not implemented") }
    public init(value: Any) {
        _value = value
        propertyType = switch value {
            case is UInt8: .uint8
            case is Int16: .int16
            case is UInt16: .uint16
            case is Int32: .int32
            case is UInt32: .uint32
            case is Int64: .int64
            case is UInt64: .uint64
            case is Float: .single
            case is Double: .double
            case is Character: .char16
            case is Bool: .boolean
            case is String: .string
            case is DateTime: .dateTime
            case is TimeSpan: .timeSpan
            case is Foundation.UUID: .guid
            case is Point: .point
            case is Size: .size
            case is Rect: .rect
            case is IWinRTObject: .inspectable
            case is IInspectable: .inspectable
            case is [UInt8]: .uint8Array
            case is [Int16]: .int16Array
            case is [UInt16]: .uint16Array
            case is [Int32]: .int32Array
            case is [UInt32]: .uint32Array
            case is [Int64]: .int64Array
            case is [UInt64]: .uint64Array
            case is [Float]: .singleArray
            case is [Double]: .doubleArray
            case is [Character]: .char16Array
            case is [Bool]: .booleanArray
            case is [String]: .stringArray
            case is [DateTime]: .dateTimeArray
            case is [TimeSpan]: .timeSpanArray
            case is [Foundation.UUID]: .guidArray
            case is [Point]: .pointArray
            case is [Size]: .sizeArray
            case is [Rect]: .rectArray
            case is [Any?]: .inspectableArray
            default: .otherType
            }
    }

    public var type: PropertyType { propertyType }
    public var isNumericScalar: Bool {
        switch propertyType {
            case .int16, .int32, .int64, .uint8, .uint16, .uint32, .uint64, .single, .double: return true
            default: return false
        }
    }

    public var value: Any { _value }

    public func getUInt8() -> UInt8 { _value as! UInt8 }
    public func getInt16() -> Int16 { _value as! Int16 }
    public func getUInt16() -> UInt16 { _value as! UInt16 }
    public func getInt32() -> Int32 { _value as! Int32 }
    public func getUInt32() -> UInt32 { _value as! UInt32 }
    public func getInt64() -> Int64 { _value as! Int64 }
    public func getUInt64() -> UInt64 { _value as! UInt64 }
    public func getSingle() -> Float { _value as! Float }
    public func getDouble() -> Double { _value as! Double }
    public func getChar16() -> Character { _value as! Character }
    public func getBoolean() -> Bool { _value as! Bool }
    public func getString() -> String { _value as! String }
    public func getGuid() -> Foundation.UUID { _value as! Foundation.UUID }
    public func getDateTime() -> DateTime { _value as! DateTime }
    public func getTimeSpan() -> TimeSpan { _value as! TimeSpan }
    public func getPoint() -> Point { _value as! Point }
    public func getSize() -> Size { _value as! Size }
    public func getRect() -> Rect { _value as! Rect }
    public func getUInt8Array(_ value: inout [UInt8]) { value = _value as! [UInt8] }
    public func getInt16Array(_ value: inout [Int16]) { value = _value as! [Int16] }
    public func getUInt16Array(_ value: inout [UInt16]) { value = _value as! [UInt16] }
    public func getInt32Array(_ value: inout [Int32]) { value = _value as! [Int32] }
    public func getUInt32Array(_ value: inout [UInt32])  { value = _value as! [UInt32] }
    public func getInt64Array(_ value: inout [Int64]) { value = _value as! [Int64] }
    public func getUInt64Array(_ value: inout [UInt64]) { value = _value as! [UInt64] }
    public func getSingleArray(_ value: inout [Float]) { value = _value as! [Float] }
    public func getDoubleArray(_ value: inout [Double]) { value = _value as! [Double] }
    public func getChar16Array(_ value: inout [Character]) { value = _value as! [Character] }
    public func getBooleanArray(_ value: inout [Bool]) { value = _value as! [Bool] }
    public func getStringArray(_ value: inout [String]) { value = _value as! [String] }
    public func getGuidArray(_ value: inout [Foundation.UUID]) { value = _value as! [Foundation.UUID] }
    public func getDateTimeArray(_ value: inout [DateTime]) { value = _value as! [DateTime] }
    public func getTimeSpanArray(_ value: inout [TimeSpan]) { value = _value as! [TimeSpan] }
    public func getPointArray(_ value: inout [Point]) { value = _value as! [Point] }
    public func getSizeArray(_ value: inout [Size]) { value = _value as! [Size] }
    public func getRectArray(_ value: inout [Rect]) { value = _value as! [Rect] }
    public func getInspectableArray(_ value: inout [Any?]) { value = _value as! [Any?] }
    %
}

)", winrtInterfaceConformance);

    }

    // Due to https://linear.app/the-browser-company/issue/WIN-148/investigate-possible-compiler-bug-crash-when-generating-collection
    // we have to generate the protocol conformance for the Collection protocol (see "// MARK: Collection" below). We shouldn't have to
    // do this because we define an extension on the protocol which does this.
    static void write_collection_protocol_conformance(writer& w, interface_info const& info)
    {
        auto modifier = w.impl_names ? "" : "public ";
        auto typeName = info.type->swift_type_name();
        if (typeName.starts_with("IVector") || typeName.starts_with("IObservableVector"))
        {
            w.write(R"(// MARK: Collection
%typealias Element = T
%var startIndex: Int { 0 }
%var endIndex: Int { Int(size) }
%func index(after i: Int) -> Int {
    i+1
}

%func index(of: Element) -> Int? {
    var index: UInt32 = 0
    let result = indexOf(of, &index)
    guard result else { return nil }
    return Int(index)
}
%var count: Int { Int(size) }
)", modifier, modifier, modifier, modifier, modifier, modifier);
            if (typeName.starts_with("IVectorView"))
            {
                w.write(R"(
%subscript(position: Int) -> Element {
    get {
        getAt(UInt32(position))
    }
}
)", modifier);
            }
            else
            {
                w.write(R"(

%subscript(position: Int) -> Element {
    get {
        getAt(UInt32(position))
    }
    set(newValue) {
        setAt(UInt32(position), newValue)
    }
}

%func removeLast() {
    removeAtEnd()
}

)", modifier, modifier);
            }
        }

        w.write("// MARK: WinRT\n");
    }

    enum class member_type
    {
        property_or_method,
        event
    };

    static std::string modifier_for(typedef_base const& type_definition, interface_info const& iface, member_type member_type = member_type::property_or_method);
    static void write_bufferbyteaccess(writer& w, interface_info const& info, system_type const& type, typedef_base const& type_definition)
    {
        auto bufferType = type.swift_type_name() == "IBufferByteAccess" ? "UnsafeMutablePointer" : "UnsafeMutableBufferPointer";
        w.write(R"(%var buffer: %<UInt8>? {
    get throws {
        let bufferByteAccess: %.__ABI_.% = try %.QueryInterface()
        return try bufferByteAccess.Buffer()
    }
}
)", modifier_for(type_definition, info), bufferType,  w.support, type.swift_type_name(), get_swift_name(info));
    }

    static bool skip_write_from_abi(writer& w, metadata_type const& type)
    {
        if (auto interfaceType = dynamic_cast<const interface_type*>(&type))
        {
            return (interfaceType->is_generic() || is_exclusive(*interfaceType) || !can_write(w, interfaceType) || get_full_type_name(interfaceType) == "Windows.Foundation.IPropertyValue");
        }
        else if (auto classType = dynamic_cast<const class_type*>(&type))
        {
            return classType->default_interface == nullptr;
        }
        return true;
    }

    static void write_comma_param_names(writer& w, std::vector<function_param> const& params)
    {
        separator s{ w };
        for (auto& param : params)
        {
            s();
            w.write(get_swift_name(param));
        }
    }

    static void write_comma_param_types(writer& w, std::vector<function_param> const& params)
    {
        separator s{ w };
        for (auto& param : params)
        {
            s();
            if (param.is_array())
            {
                w.write("[%]", bind<write_type>(*param.type, write_type_params::swift));
            }
            else
            {
                write_type(w, *param.type, write_type_params::swift);
            }
        }
    }

    static void write_param_names(writer& w, std::vector<function_param> const& params, std::string_view format)
    {
        separator s{ w };
        for (const auto& param : params)
        {
            s();
            w.write(format, local_swift_param_name(get_swift_name(param)));
        }
    }

    // When converting from Swift <-> C we put some local variables on the stack in order to help facilitate
    // converting between the two worlds. This method will returns a scope guard which will write any necessary
    // code for after the ABI function is called (such as cleaning up references).
    static write_scope_guard<writer> write_local_param_wrappers(writer& w, std::vector<function_param> const& params)
    {
        write_scope_guard guard{ w, w.swift_module };

        std::vector<function_param> com_ptr_initialize;
        for (auto& param : params)
        {
            TypeDef signature_type{};
            auto category = get_category(param.type, &signature_type);
            auto param_name = get_swift_name(param);
            auto local_param_name = local_swift_param_name(param_name);

            if (param.is_array())
            {
                if (param.signature.ByRef())
                {
                    w.write("var %: WinRTArrayAbi<%> = (0, nil)\n",
                        local_param_name,
                        bind<write_type>(*param.type, write_type_params::c_abi));

                    guard.insert_front("% = %\n", param_name, bind<write_convert_array_from_abi>(*param.type, local_param_name));
                    guard.insert_front("defer { CoTaskMemFree(%.start) }\n", local_param_name);
                }
                else
                {
                    // Array is passed by reference, so we need to convert the input to a buffer and then pass that buffer to C, then convert the buffer back to an array
                    if (is_reference_type(param.type))
                    {
                        w.write("try %.toABI(abiBridge: %.self) { % in\n", param_name, bind_bridge_name(*param.type), local_param_name);
                    }
                    else
                    {
                        w.write("try %.toABI { % in\n", param_name, local_param_name);
                    }

                    // Enums and fundamental (integer) types can just be copied directly into the ABI. So we can
                    // avoid an extra copy by simply passing the array buffer to C directly
                    if (!param.in() && category != param_category::enum_type && category != param_category::fundamental_type)
                    {
                        // While perhaps not the most effient to just create a new array from the elements (rather than filling an existing buffer), it is the simplest for now.
                        // These APIs are few and far between and rarely used. If needed, we can optimize later.
                        guard.insert_front("% = %\n", param_name, bind<write_convert_array_from_abi>(*param.type, local_param_name));
                    }

                    guard.push("}\n");
                    guard.push_indent();
                }
            }
            else if (param.in())
            {
                if (category == param_category::string_type)
                {
                    w.write("let % = try! HString(%)\n",
                        local_param_name,
                        param_name);
                }
                else if (category == param_category::struct_type && !is_struct_blittable(signature_type))
                {
                    w.write("let % = %._ABI_%(from: %)\n",
                        local_param_name,
                        abi_namespace(signature_type),
                        signature_type.TypeName(),
                        param_name);
                }
                else if (is_reference_type(param.type) && !is_class(param.type))
                {
                    w.write("let %Wrapper = %(%)\n",
                        param_name,
                        bind_wrapper_fullname(param.type),
                        param_name);
                    w.write("let % = try! %Wrapper?.toABI { $0 }\n",
                        local_param_name,
                        param_name);
                }
                else if (param.signature.ByRef())
                {
                    w.write("var %: % = .from(swift: %)\n",
                        local_param_name,
                        bind<write_type>(*param.type, write_type_params::c_abi),
                        param_name);
                }
            }
            else
            {
                if (category == param_category::string_type)
                {
                    w.write("var %: HSTRING?\n",
                        local_param_name);
                    guard.push("% = .init(from: %)\n",
                        param_name,
                        local_param_name);
                    guard.push("WindowsDeleteString(%)\n", local_param_name);
                }
                else if (category == param_category::struct_type &&
                    is_struct_blittable(signature_type) &&
                    !is_guid(category))
                {
                    w.write("var %: % = .init()\n",
                        local_param_name,
                        bind_type_mangled(param.type));
                    guard.push("% = .from(abi: %)\n",
                        param_name,
                        local_param_name);
                }
                else if (category == param_category::struct_type)
                {
                    w.write("let %: %._ABI_% = .init()\n",
                        local_param_name,
                        abi_namespace(param.type),
                        param.type->swift_type_name());
                    guard.push("% = .from(abi: %.val)\n",
                        param_name,
                        local_param_name);
                }
                else if (category == param_category::boolean_type || category == param_category::character_type)
                {
                    w.write("var %: % = .init()\n",
                        local_param_name,
                        bind_type_abi(param.type));
                    guard.push("% = .init(from: %)\n",
                        param_name,
                        local_param_name);
                }
                else if (needs_wrapper(category))
                {
                    com_ptr_initialize.push_back(param);
                }
            }
        }

        // At initial writing, ComPtrs.initialize only has overloads for 5 parameters. If we have more than 5
        // then the generated code won't compile. Rather than check for the number here, just let generated
        // code not compile so that we can add the overload to ComPtrs.initialize later on. This would also
        // in theory let someone add a new overload to ComPtrs.initialize with a different number of parameters
        // on their own as a way to unblock themselves
        if (!com_ptr_initialize.empty())
        {
            w.write("let (%) = try ComPtrs.initialize { (%) in\n",
                bind<write_param_names>(com_ptr_initialize, "%"),
                bind<write_param_names>(com_ptr_initialize, "%Abi"));
            guard.push("}\n");
            guard.push_indent();

            for (const auto& param : com_ptr_initialize)
            {
                auto param_name = get_swift_name(param);
                auto local_param_name = local_swift_param_name(param_name);
                guard.push("% = %\n", param_name,
                    bind<write_consume_type>(param.type, local_param_name, true));
            }
        }

        return guard;
    }

    static void write_factory_body(writer& w, function_def const& method, interface_info const& factory, class_type const& type, metadata_type const& default_interface)
    {
        std::string_view func_name = get_abi_name(method);

        auto swift_name = get_swift_name(factory);
        auto return_name = method.return_type.value().name;
        auto func_call = w.write_temp("try! Self.%.%(%)",
            swift_name,
            func_name,
            bind<write_implementation_args>(method));
        if (auto base_class = type.base_class)
        {
            w.write("super.init(fromAbi: %)\n", func_call);
        }
        else
        {
            w.write("super.init(%)\n", func_call);
        }

    }

    // Check if the type has a default constructor. This is a parameterless constructor
    // in Swift. Note that we don't check the args like we do in base_has_matching_constructor
    // because composing constructors project as init() when they really have 2 parameters.
    static bool has_default_constructor(const class_type* type)
    {
        if (type == nullptr) return true;

        for (const auto& [_, factory] : type->factories)
        {
            if (factory.composable && factory.defaultComposable)
            {
                return true;
            }
            else if (factory.activatable && factory.type == nullptr)
            {
                return true;
            }
        }
        return false;
    }

    static std::vector<function_param> get_projected_params(attributed_type const& factory, function_def const& func)
    {
        if (factory.composable)
        {
            std::vector<function_param> result;
            result.reserve(func.params.size() - 2);
            // skip the last two which are the inner and base interface parameters
            for (size_t i = 0; i < func.params.size() - 2; ++i)
            {
                result.push_back(func.params[i]);
            }
            return result;
        }
        else
        {
            return func.params;
        }
    }

    static bool derives_from(class_type const& base, class_type const& derived)
    {
        class_type const* checking = &derived;
        while (checking != nullptr)
        {
            if (checking->base_class == &base)
            {
                return true;
            }
            checking = checking->base_class;
        }
        return false;
    }

    static bool base_matches(function_def const& base, function_def const& derived)
    {
        // Simple cases of name/param count/has return not matching
        if (base.def.Name() != derived.def.Name()) return false;
        if (base.return_type.has_value() != derived.return_type.has_value()) return false;
        if (base.params.size() != derived.params.size()) return false;

        // If they both have a return value, we need to check if the return values
        // are derived from each other. If you have two functions like this:
        //   (A.swift) class func make() -> A
        //   (B.swift) class func make() -> B
        // Then these would "match" according to the swift compiler
        if (base.return_type.has_value())
        {
            auto base_return = base.return_type.value().type;
            auto derived_return = derived.return_type.value().type;
            if (base_return != derived_return)
            {
                // Check if the types are derived
                auto base_return_class = dynamic_cast<const class_type*>(base_return);
                auto derived_return_class = dynamic_cast<const class_type*>(derived_return);
                if (base_return_class != nullptr && derived_return_class != nullptr)
                {
                    if (!derives_from(*base_return_class, *derived_return_class))
                    {
                        // Return types aren't a possible match, return false
                        return false;
                    }
                }
            }
        }

        size_t i = 0;
        while (i < base.params.size())
        {
            if (base.params[i].type != derived.params[i].type)
            {
                // param doesn't match
                return false;
            }
            ++i;
        }
        return true;
    }

    static bool base_has_matching_static_function(class_type const& type, attributed_type const& factory, function_def const& func)
    {
        if (type.base_class == nullptr)
        {
            return false;
        }

        for (const auto& [_, baseFactory] : type.base_class->factories)
        {
            // only look at statics
            if (!baseFactory.statics) continue;
            if (auto factoryIface = dynamic_cast<const interface_type*>(baseFactory.type))
            {
                for (const auto& baseMethod : factoryIface->functions)
                {
                    if (base_matches(baseMethod, func))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    static bool base_has_matching_constructor(class_type const& type, attributed_type const& factory, function_def const& func)
    {
        auto projectedParams = get_projected_params(factory, func);

        if (type.base_class == nullptr)
        {
            return projectedParams.size() == 0;
        }

        for (const auto& [_, baseFactory] : type.base_class->factories)
        {
            // only look at activation or composing constructors
            if (!baseFactory.activatable && !baseFactory.composable) continue;
            if (auto factoryIface = dynamic_cast<const interface_type*>(baseFactory.type))
            {
                for (const auto& baseMethod : factoryIface->functions)
                {
                    auto baseProjectedParams = get_projected_params(baseFactory, baseMethod);
                    if (projectedParams.size() == baseProjectedParams.size())
                    {
                        size_t i = 0;
                        while(i < baseProjectedParams.size())
                        {
                            if (baseProjectedParams[i].type != projectedParams[i].type)
                            {
                                break;
                            }
                            ++i;
                        }
                        if (i == baseProjectedParams.size())
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    static std::string modifier_for(typedef_base const& type_definition, interface_info const& iface, member_type member)
    {
        std::string modifier;
        auto classType = dynamic_cast<const class_type*>(&type_definition);
        const bool isClass = classType != nullptr;
        if (isClass)
        {
            if (iface.overridable)
            {
                modifier = "open ";
            }
            else
            {
                modifier = "public ";
            }
        }
        else
        {
            modifier = "fileprivate ";
        }

        if (iface.attributed && isClass && classType->is_composable() && member == member_type::property_or_method)
        {
            modifier.append("class ");
        }
        else if (iface.attributed)
        {
            modifier.append("static ");
        }

        return modifier;
    }

    static std::string modifier_for(typedef_base const& type_definition, attributed_type const& attributedType, function_def const& func)
    {
        interface_info info { attributedType.type };
        info.attributed = true;
        auto modifier = modifier_for(type_definition, info);
        if (auto classType = dynamic_cast<const class_type*>(&type_definition))
        {
            if (base_has_matching_static_function(*classType, attributedType, func))
            {
                modifier.insert(0, "override ");
            }
        }
        return modifier;
    }

    template <typename T>
    static void write_iinspectable_methods(writer& w, T const& type, std::vector<named_interface_info> const& interfaces, bool composed = false)
    {
        // 3 interfaces for IUnknown, IInspectable, type.
        auto interface_count = 3 + interfaces.size();
        w.write("GetIids: {\n");
        {
            auto indent_guard = w.push_indent();
            w.write("let size = MemoryLayout<%.IID>.size\n", w.support);
            w.write("let iids = CoTaskMemAlloc(UInt64(size) * %).assumingMemoryBound(to: %.IID.self)\n", interface_count, w.support);
            w.write("iids[0] = IUnknown.IID\n");
            w.write("iids[1] = IInspectable.IID\n");
            w.write("iids[2] = %.IID\n", bind_wrapper_fullname(type));

            auto iface_n = 3;
            for (const auto& iface : interfaces)
            {
                if (!can_write(w, iface.second.type)) continue;

                w.write("iids[%] = %.IID\n",
                    iface_n++,
                    bind_wrapper_fullname(iface.second.type)
                );
            }

            w.write("$1!.pointee = %\n", interface_count);
            w.write("$2!.pointee = iids\n");
            w.write("return S_OK\n");
        }
        w.write("},\n\n");

        if (composed)
        {
            // for composed types, get the swift object and grab the typename
            w.write(R"(GetRuntimeClassName: {
    guard let instance = %.tryUnwrapFrom(raw: $0) else { return E_INVALIDARG }
    let hstring = instance.GetRuntimeClassName().detach()
    $1!.pointee = hstring
    return S_OK
},

)", bind_wrapper_name(type));
        }
        else
        {
            w.write(R"(GetRuntimeClassName: {
    _ = $0
    let hstring = try! HString("%").detach()
    $1!.pointee = hstring
    return S_OK
},

)", get_full_type_name(type));
        }


        w.write(R"(GetTrustLevel: {
    _ = $0
    $1!.pointee = TrustLevel(rawValue: 0)
    return S_OK
})"
);
    }

    // assigns return or out parameters in vtable methods
    template<typename T>
    static void do_write_abi_val_assignment(writer& w, T const& return_type, std::string_view return_param_name)
    {
        auto type = return_type.type;
        auto param_name = get_swift_member_name(return_type.name);
        TypeDef signature_type{};
        auto category = get_category(type, &signature_type);

        if (return_type.is_array())
        {
            if (is_reference_type(type))
            {
                w.write("%.fill(abi: %, abiBridge: %.self)\n",
                    param_name, return_param_name, bind_bridge_fullname(*type));
            }
            else if (category == param_category::enum_type || category == param_category::fundamental_type)
            {
                w.write("%.fill(abi: %)\n",
                    param_name, return_param_name);
            }
            else
            {
                w.write(R"(do {
    try %.fill(abi: %)
} catch { return failWith(error: error) }
)", param_name, return_param_name);
            }
            return;
        }
        else if (category == param_category::struct_type && !is_struct_blittable(signature_type))
        {
            w.write("let _% = %._ABI_%(from: %)\n\t",
                param_name,
                abi_namespace(type),
                type->swift_type_name(),
                param_name);
        }
        else if (is_reference_type(type))
        {
            if (!is_class(type))
            {
                w.write("let %Wrapper = %(%)\n",
                    param_name,
                    bind_wrapper_fullname(type),
                    param_name);
                w.write("%Wrapper?.copyTo(%)\n", param_name, return_param_name);
            }
            else
            {
                w.write("%?.copyTo(%)\n", param_name, return_param_name);
            }
            return;
        }

        w.write("%?.initialize(to: %)\n",
            return_param_name,
            bind([&](writer& w) {
                write_convert_to_abi_arg(w, return_type);
            })
        );
    }

    static void write_abi_ret_val(writer& w, function_def signature)
    {
        int param_number = 1;
        for (auto& param : signature.params)
        {
            auto param_name = get_swift_name(param);
            if (param.is_array())
            {
                if (param.signature.ByRef())
                {
                    w.write("$%?.initialize(to: UInt32(%.count))\n", param_number, param_name);
                }
                param_number++;
            }
            if (param.out())
            {
                auto return_param_name = "$" + std::to_string(param_number);
                do_write_abi_val_assignment(w, param, return_param_name);
            }

            param_number++;
        }

        if (signature.return_type)
        {
            if (signature.return_type.value().is_array())
            {
                w.write("$%?.initialize(to: UInt32(%.count))\n", param_number, signature.return_type.value().name);
                param_number++;
            }
            auto return_param_name = "$" + std::to_string(param_number);
            do_write_abi_val_assignment(w, signature.return_type.value(), return_param_name);
        }
    }

    static void write_not_implementable_vtable_method(writer& w, function_def const& sig)
    {
        w.write("%: { _, % in return failWith(hr: E_NOTIMPL) }", get_abi_name(sig), bind([&](writer& w) {
            separator s{ w };
            for (auto& param : sig.params)
            {
                s();
                if (param.signature.Type().is_szarray())
                {
                    w.write("_, ");
                }
                w.write("_");
            }
            if (sig.return_type)
            {
                s();
                if (sig.return_type.value().signature.Type().is_szarray())
                {
                    w.write("_, ");
                }
                w.write("_");
            }}));
    }

    template<typename T>
    static void write_vtable_method(writer& w, function_def const& function, T const& type)
    {
        constexpr bool isInterface = std::is_same_v<T, interface_type>;
        constexpr bool isDelegate = std::is_same_v<T, delegate_type>;
        constexpr bool isGeneric = std::is_same_v<T, generic_inst>;
        static_assert(isInterface || isDelegate | isGeneric);

        auto method = function.def;
        auto func_name = get_abi_name(method);

        // https://linear.app/the-browser-company/issue/WIN-104/swiftwinrt-support-authoring-winrt-events-in-swift
        if (!can_write(w, function, true))
        {
            write_not_implementable_vtable_method(w, function);
            return;
        }
        // we have to write the methods in a certain order and so we will have to detect here whether
        // this method is a property getter/setter and adjust the call so we use the swift syntax
        std::string func_call;
        bool is_get_or_put = true;
        if (is_get_overload(method))
        {
            if (!is_winrt_ireference(&type))
            {
                func_call += w.write_temp(".%", get_swift_name(method));
            }
        }
        else if (is_put_overload(method))
        {
            func_call += w.write_temp(".% = %", get_swift_name(method), bind<write_consume_args>(function));
        }
        else if (is_add_overload(method))
        {
            func_call += w.write_temp(".%.addHandler(%)", get_swift_name(method), bind<write_consume_args>(function));
        }
        else if (is_remove_overload(method))
        {
            assert(function.params.size() == 1);
            func_call += w.write_temp(".%.removeHandler(%)", get_swift_name(method), function.params[0].name);
        }
        else
        {
            is_get_or_put = false;
            // delegate arg types are a tuple, so wrap in an extra paranthesis
            auto format = is_delegate(type) ? "%(%)" : ".%(%)";
            func_call += w.write_temp(format, get_swift_name(method), bind<write_consume_args>(function));
        }

        bool needs_try_catch = !is_get_or_put && !is_winrt_generic_collection(type);
        w.write("%: {\n", func_name);
        if (needs_try_catch) {
            w.m_indent += 1;
            w.write("do {\n");
        }
        {
            auto indent_guard = w.push_indent();
            w.write("guard let __unwrapped__instance = %.tryUnwrapFrom(raw: $0) else { return E_INVALIDARG }\n",
                bind_wrapper_name(type));

            write_convert_vtable_params(w, function);

            if (function.return_type)
            {
                w.write("let % = ", function.return_type.value().name);
            }

            w.write("%__unwrapped__instance%\n",
                needs_try_catch ? "try " : "",
                func_call);

            write_abi_ret_val(w, function);
            w.write("return S_OK\n");
        }
        if (needs_try_catch) {
            w.write("} catch { return failWith(error: error) }\n");
            w.m_indent -= 1;
        }
        w.write("}");
    }

    template <typename T>
    static void do_write_vtable(writer& w, T const& type, std::vector<named_interface_info> interfaces)
    {
        constexpr bool isInterface = std::is_same_v<T, interface_type>;
        constexpr bool isDelegate = std::is_same_v<T, delegate_type>;
        constexpr bool isGeneric = std::is_same_v<T, generic_inst>;
        static_assert(isInterface || isDelegate | isGeneric);
        w.write("internal %var %VTable: %Vtbl = .init(\n",
            isGeneric ? "" : "static ", // generics aren't namespaced since implementations are always internal
            type,
            bind_type_mangled(type));
        {
            auto indent = w.push_indent();
            write_iunknown_methods(w, type);
            separator s{ w, ",\n\n" };

            if (!is_delegate(type))
            {
                write_iinspectable_methods(w, type, interfaces);
                s(); // get first separator out of the way for no-op
            }

            for (const auto& method : type.functions)
            {
                if (method.def.Name() != ".ctor")
                {
                    s();
                    write_vtable_method(w, method, type);
                }

            }
        }

        w.write(R"(
)
)");

    }
}
