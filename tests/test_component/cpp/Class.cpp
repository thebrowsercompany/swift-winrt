#include "pch.h"
#include "Class.h"
#include "Class.g.cpp"

namespace winrt::test_component::implementation
{
    using namespace Windows::Foundation;
    float Class::s_float = 0.0f;

    struct Value : implements<Value, IStringable>
    {
        Value(int32_t value) :
            m_value(value)
        {
        }

        hstring ToString()
        {
            return hstring{ std::to_wstring(m_value) };
        }

    private:

        int32_t m_value{};
    };

    hstring Class::InInt32(int32_t value)
    {
        if (m_implementation)
        {
            return m_implementation.InInt32(value);
        }
        return hstring{ std::to_wstring(value) };
    }

    hstring Class::InString(hstring const& value)
    {
        if (m_implementation)
        {
            return m_implementation.InString(value);
        }
        return value;
    }

    hstring Class::InObject(Windows::Foundation::IInspectable const& value)
    {
        if (m_implementation)
        {
            return m_implementation.InObject(value);
        }
        if (auto stringable = value.try_as<IStringable>())
        {
            return stringable.ToString();
        }
        else if (auto pv = value.try_as<IPropertyValue>())
        {
            switch (pv.Type())
            {
            case PropertyType::UInt8:
                return winrt::to_hstring(pv.GetUInt8());
            case PropertyType::Int16:
                return winrt::to_hstring(pv.GetInt16());
            case PropertyType::UInt16:
                return winrt::to_hstring(pv.GetUInt16());
            case PropertyType::Int32:
                return winrt::to_hstring(pv.GetInt32());
            case PropertyType::Int64:
                return winrt::to_hstring(pv.GetInt64());
            case PropertyType::String:
                return pv.GetString();
            case PropertyType::Char16:
                throw hresult_invalid_argument(L"char16 not expected - should be boxed as string");
            default:
                throw hresult_not_implemented(L"Unimplemented switch case");
            }
        }
        else if (auto basic = value.try_as<IBasic>() && value.try_as<ISimpleDelegate>())
        {
            return L"simply basic";
        }
        else if (auto simple = value.try_as<ISimpleDelegate>())
        {
            return L"simple";
        }
        else if (auto basic = value.try_as<IBasic>())
        {
            return L"basic";
        }
        return winrt::get_class_name(value);
    }

    hstring Class::InStringable(Windows::Foundation::IStringable const& value)
    {
        return value.ToString();
    }

    hstring Class::InEnum(Signed const& value)
    {
        if (m_implementation)
        {
            return m_implementation.InEnum(value);
        }

        switch (value)
        {
        case Signed::First: return L"First";
        case Signed::Second: return L"Second";
        case Signed::Third: return L"Third";
        }

        throw hresult_invalid_argument();
    }

    Class::Class(Windows::Foundation::Collections::IIterable<hstring> const& arg, int32_t)
    {
        if (arg.First().Current() != L"test")
        {
            throw hresult_error();
        }
    }
    Class::Class(Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<hstring, hstring>> const& arg, int32_t, int32_t)
    {
        if (arg.First().Current().Key() != L"test")
        {
            throw hresult_error();
        }
    }
    Class::Class(Windows::Foundation::Collections::IMap<hstring, hstring> const& arg, int32_t, int32_t, int32_t)
    {
        if (arg.Lookup(L"test") != L"test")
        {
            throw hresult_error();
        }
    }
    Class::Class(Windows::Foundation::Collections::IMapView<hstring, hstring> const& arg, int32_t, int32_t, int32_t, int32_t)
    {
        if (arg.Lookup(L"test") != L"test")
        {
            throw hresult_error();
        }
    }
    Class::Class(Windows::Foundation::Collections::IVector<hstring> const& arg, int32_t, int32_t, int32_t, int32_t, int32_t)
    {
        if (arg.GetAt(0) != L"test")
        {
            throw hresult_error();
        }
    }
    Class::Class(Windows::Foundation::Collections::IVectorView<hstring> const& arg, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t)
    {
        if (arg.GetAt(0) != L"test")
        {
            throw hresult_error();
        }
    }


    void Class::OutInt32(int32_t& value)
    {
        if (m_implementation)
        {
            m_implementation.OutInt32(value);
        }
        else
        {
            value = 123;
        }
    }

    void Class::OutString(hstring& value)
    {
        if (m_implementation)
        {
            m_implementation.OutString(value);
        }
        else
        {
            value = L"123";
        }
    }

    void Class::OutObject(Windows::Foundation::IInspectable& value)
    {
        if (m_implementation)
        {
            m_implementation.OutObject(value);
        }
        else
        {
            value = make<Value>(123);
        }
    }

    void Class::OutStringable(Windows::Foundation::IStringable& value)
    {
        value = make<Value>(123);
    }

    void Class::OutEnum(Signed& value)
    {
        if (m_implementation)
        {
            m_implementation.OutEnum(value);
        }
        else
        {
            value = Signed::First;
        }
    }

    void Class::OutBlittableStruct(test_component::BlittableStruct& value)
    {
        if (m_implementation)
        {
            m_implementation.OutBlittableStruct(value);
        }
        else
        {
            value = { 867, 5309 };
        }
    }

    void Class::OutNonBlittableStruct(test_component::NonBlittableStruct& value)
    {
        if (m_implementation)
        {
            m_implementation.OutNonBlittableStruct(value);
        }
        else
        {
            value = { L"please", L"vote", 4, L"pedro"};
        }
    }

    int32_t Class::ReturnInt32()
    {
        return 123;
    }
    hstring Class::ReturnString()
    {
        return L"123";
    }
    Windows::Foundation::IInspectable Class::ReturnObject()
    {
        if (m_implementation)
        {
            return m_implementation.ReturnObject();
        }
        else
        {
            return make<Value>(123);
        }
    }
    Windows::Foundation::IStringable Class::ReturnStringable()
    {
        return make<Value>(123);
    }

    Fruit Class::EnumProperty() const
    {
        if (m_implementation)
        {
            return m_implementation.EnumProperty();
        }
        return m_fruit;
    }

    void Class::EnumProperty(Fruit const& value)
    {
        if (m_implementation)
        {
            m_implementation.EnumProperty(value);
        }
        else
        {
            m_fruit = value;
        }
    }

    Signed Class::ReturnEnum()
    {
        if (m_implementation)
        {
            return m_implementation.ReturnEnum();
        }

        return Signed::First;
    }

    void Class::NoexceptVoid() noexcept
    {
    }

    int32_t Class::NoexceptInt32() noexcept
    {
        return 123;
    }

    hstring Class::NoexceptString() noexcept
    {
        return L"123";
    }

    void Class::Id(Windows::Foundation::IReference<winrt::guid> const& value)
    {
        if (m_implementation)
        {
            m_implementation.Id(value);
        }
        else
        {
            m_id = value;
        }
    }

    Windows::Foundation::IReference<winrt::guid> Class::Id()
    {
        if (m_implementation)
        {
            return m_implementation.Id();
        }
        else
        {
            return m_id;
        }
    }

    event_token Class::DeferrableEvent(TypedEventHandler<test_component::Class, test_component::DeferrableEventArgs> const& handler)
    {
        return m_deferrableEvent.add(handler);
    }

    void Class::DeferrableEvent(event_token const& token)
    {
        return m_deferrableEvent.remove(token);
    }

    IAsyncOperation<int32_t> Class::RaiseDeferrableEventAsync()
    {
        auto args = make_self<DeferrableEventArgs>();
        m_deferrableEvent(*this, *args);
        co_await args->wait_for_deferrals();
        co_return args->m_counter;
    }

    // This test validates that defining WINRT_NO_MAKE_DETECTION actually
    // allows an implementation to be final and have a private destructor.
    // This is *not* recommended as there are no safeguards for direct and
    // invalid allocations, but is provided for compatibility.
/*
    bool Class::TestNoMakeDetection()
    {
        static bool Destroyed{};

        struct Stringable final : implements<Stringable, IStringable>
        {
            hstring ToString()
            {
                return L"Stringable";
            }

        private:

            ~Stringable()
            {
                Destroyed = true;
            }
        };

        bool pass = true;

        {
            Destroyed = false;
            IStringable stringable{ (new Stringable())->get_abi<IStringable>(), take_ownership_from_abi };
            pass = pass && !Destroyed;

            stringable = nullptr;
            pass = pass && Destroyed;
        }
        {
            Destroyed = false;
            IStringable stringable = make<Stringable>();
            pass = pass && !Destroyed;
            stringable = nullptr;
            pass = pass && Destroyed;
        }
        return pass;
    }
    */
}

/*
namespace
{
    void ValidateStaticEventAutoRevoke() {
        auto x = winrt::test_component::Simple::StaticEvent(winrt::auto_revoke, [](auto&&, auto&&) {});
    }
}
*/