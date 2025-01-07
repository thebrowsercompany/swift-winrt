#pragma once
#include "Class.g.h"
#include "DeferrableEventArgs.g.h"
namespace winrt::test_component::implementation
{

    struct SimpleDelegate : implements<SimpleDelegate, test_component::ISimpleDelegate>
    {
        void DoThis()
        {
            printf("C++ - DoThis\n");
        }

        void DoThat(int32_t value)
        {
            printf("C++ - DoThat: %d\n", value);
        }
    };

    struct BasicDelegate : implements<BasicDelegate, test_component::IBasic>
    {
        void Method()
        {
            printf("C++ - Method\n");
        }
    };

    struct Class : ClassT<Class>
    {
        Class() = default;

        Class(hstring const&) {}
        Class(hstring const&, Fruit const& fruit) : m_fruit(fruit){}
        Class(hstring const& name, Fruit const& fruit, IIAmImplementable const& implementation) : m_fruit(fruit), m_implementation(implementation) {}
        static void StaticTest()
        {
        }

        static int32_t StaticTestReturn()
        {
            return 42;
        }

        static int32_t StaticProperty()
        {
            return 18;
        }

        static float StaticTestReturnFloat()
        {
            return 42.24f;
        }

        static float StaticPropertyFloat()
        {
            return s_float;
        }
        static void StaticPropertyFloat(float value) { s_float = value; }

        void Fail(winrt::hstring const& message)
        {
            throw hresult_illegal_method_call(message);
        }

        Class(Windows::Foundation::Collections::IIterable<hstring> const& arg, int32_t dummy1);
        Class(Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<hstring, hstring>> const& arg, int32_t dummy1, int32_t dummy2);
        Class(Windows::Foundation::Collections::IMap<hstring, hstring> const& arg, int32_t dummy1, int32_t dummy2, int32_t dummy3);
        Class(Windows::Foundation::Collections::IMapView<hstring, hstring> const& arg, int32_t dummy1, int32_t dummy2, int32_t dummy3, int32_t dummy4);
        Class(Windows::Foundation::Collections::IVector<hstring> const& arg, int32_t dummy1, int32_t dummy2, int32_t dummy3, int32_t dummy4, int32_t dummy5);
        Class(Windows::Foundation::Collections::IVectorView<hstring> const& arg, int32_t dummy1, int32_t dummy2, int32_t dummy3, int32_t dummy4, int32_t dummy5, int32_t dummy6);


        void SetDelegate(test_component::ISimpleDelegate const& value)
        {
            m_delegate = value;
            if (m_delegate)
            {
                m_delegate.DoThis();
                m_delegate.DoThat(3);
            }
        }

        test_component::ISimpleDelegate GetDelegate() { return m_delegate != nullptr ? m_delegate : make<SimpleDelegate>(); }

        hstring InInt32(int32_t value);
        hstring InString(hstring const& value);
        hstring InObject(Windows::Foundation::IInspectable const& value);
        hstring InStringable(Windows::Foundation::IStringable const& value);
        hstring InEnum(Signed const& value);

        void OutInt32(int32_t& value);
        void OutString(hstring& value);
        void OutObject(Windows::Foundation::IInspectable& value);
        void OutStringable(Windows::Foundation::IStringable& value);
        void OutEnum(Signed& value);
        void OutBlittableStruct(BlittableStruct& value);
        void OutNonBlittableStruct(NonBlittableStruct& value);

        int32_t ReturnInt32();
        hstring ReturnString();
        Windows::Foundation::IInspectable ReturnObject();
        Windows::Foundation::IStringable ReturnStringable();
        Signed ReturnEnum();
        Windows::Foundation::IReference<Signed> ReturnReferenceEnum() { return Windows::Foundation::IReference<Signed>(ReturnEnum()); }

        Fruit EnumProperty() const;
        void EnumProperty(Fruit const& value);

        void NoexceptVoid() noexcept;
        int32_t NoexceptInt32() noexcept;
        hstring NoexceptString() noexcept;

        event_token DeferrableEvent(Windows::Foundation::TypedEventHandler<test_component::Class, test_component::DeferrableEventArgs> const& handler);
        void DeferrableEvent(event_token const& token);
        Windows::Foundation::IAsyncOperation<int> RaiseDeferrableEventAsync();

        static bool TestNoMakeDetection();

        static int32_t StaticPropertyWithAsyncSetter()
        {
            return 0;
        }

        static fire_and_forget StaticPropertyWithAsyncSetter(int32_t)
        {
            co_return;
        }

        static fire_and_forget StaticMethodWithAsyncReturn()
        {
            co_return;
        }

        char16_t ReturnChar() { return L'd'; }
        hstring InChar(char16_t value)
        {
            return hstring((wchar_t*)&value, 1);
        }

        void OutChar(char16_t& value)
        {
            value = L'z';
        }

        void Method(){
            if (m_basicImpl){
                m_basicImpl.Method();
            }
            printf("Method called!\n");
        }

        test_component::IBasic Implementation()
        {
            return m_basicImpl != nullptr ? m_basicImpl : make<BasicDelegate>();
        }
        void Implementation(test_component::IBasic const& value)
        {
            m_basicImpl = value;
            if (m_basicImpl)
            {
                m_basicImpl.Method();
            }
            else {
                printf("Property cleared!\n");
            }
        }

        Windows::Foundation::IReference<int32_t> StartValue() { return m_startValue; }
        void StartValue(Windows::Foundation::IReference<int32_t> const& value) {
            m_startValue = value;
            if (m_startValue)
            {
                auto pv = m_startValue.as<Windows::Foundation::IPropertyValue>();
                printf("CPP: Start Value from PropertyValue: %d\n", pv.GetInt32());
                if (pv.GetInt32() != 23)
                {
                    throw hresult_invalid_argument(L"value");
                }
            }
        }

        Windows::Foundation::IReference<winrt::guid> Id();
        void Id(Windows::Foundation::IReference<winrt::guid> const& value);
        test_component::Base BaseProperty() { return m_base; }
        void BaseProperty(test_component::Base const& value) { m_base = value; }
        test_component::BaseNoOverrides BaseNoOverridesProperty() { return m_baseNoOverrides; }
        void BaseNoOverridesProperty(test_component::BaseNoOverrides const& value) { m_baseNoOverrides = value; }

        static void TakeBaseAndGiveToCallbackAsObject(test_component::Base const& base, test_component::Delegates::InObjectDelegate const& callback) { callback(base);}
    private:
        static float s_float;
        bool m_fail{};
        event<Windows::Foundation::TypedEventHandler<test_component::Class, test_component::DeferrableEventArgs>> m_deferrableEvent;

        template<typename T>
        static void simulate_rpc_behavior(array_view<T> const& value)
        {
            // RPC requires array pointers to be non-null.
            if (value.begin() == nullptr)
            {
                throw hresult_error(static_cast<hresult>(0x800706f4)); // HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER)
            }
        }

        test_component::Fruit m_fruit{ test_component::Fruit::Banana };

        test_component::IIAmImplementable m_implementation{};
        test_component::ISimpleDelegate m_delegate{};
        test_component::IBasic m_basicImpl{};
        Windows::Foundation::IReference<int32_t> m_startValue{};
        Windows::Foundation::IReference<winrt::guid> m_id{};
        test_component::Base m_base = test_component::Derived();
        test_component::BaseNoOverrides m_baseNoOverrides { nullptr };
    };


    struct DeferrableEventArgs : DeferrableEventArgsT<DeferrableEventArgs>, deferrable_event_args<DeferrableEventArgs>
    {
        DeferrableEventArgs() = default;
        void IncrementCounter() { ++m_counter; }
        std::atomic<int> m_counter = 0;
    };

}
namespace winrt::test_component::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
