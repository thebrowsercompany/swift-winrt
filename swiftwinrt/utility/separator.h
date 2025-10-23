#pragma once
#include "type_writers.h"

namespace swiftwinrt {
    struct separator
    {
        writer& w;
        std::string s = ", ";
        bool first{ true };

        void operator()()
        {
            if (first)
            {
                first = false;
            }
            else
            {
                w.write(s);
            }
        }
    };
}