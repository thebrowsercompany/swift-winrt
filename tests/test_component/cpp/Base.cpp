#include "pch.h"
#include "Base.h"
#include "Base.g.cpp"

namespace winrt::test_component::implementation
{
    void Base::DoTheThing()
    {
        overridable().OnDoTheThing();
    }

    void Base::OnDoTheThing()
    {
        printf("Base class done\n");
    }
}
