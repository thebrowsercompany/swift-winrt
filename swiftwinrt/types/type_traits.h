#pragma once

#include "types/metadata_type.h"
#include "types/class_type.h"
#include "types/delegate_type.h"
#include "types/element_type.h"
#include "types/generic_inst.h"
#include "types/interface_type.h"
#include "types/type_constants.h"

namespace swiftwinrt
{
    inline bool is_generic_inst(metadata_type const* type)
    {
        return dynamic_cast<const generic_inst*>(type) != nullptr;
    }

    inline bool is_generic_inst(metadata_type const& type)
    {
        return is_generic_inst(&type);
    }

    inline bool is_generic_def(metadata_type const* type)
    {
        auto typedef_base_ptr = dynamic_cast<const typedef_base*>(type);
        return typedef_base_ptr != nullptr && typedef_base_ptr->is_generic();
    }

    inline bool is_generic_def(metadata_type const& type)
    {
        return is_generic_def(&type);
    }

    inline bool is_delegate(metadata_type const* type, bool allow_generic = true)
    {
        if (allow_generic)
        {
            if (auto generic_inst_ptr = dynamic_cast<generic_inst const*>(type))
            {
                return is_delegate(generic_inst_ptr->generic_type());
            }
        }

        return dynamic_cast<delegate_type const*>(type) != nullptr;
    }

    inline bool is_delegate(metadata_type const& type, bool allow_generic = true)
    {
        return is_delegate(&type, allow_generic);
    }

    inline bool is_interface(metadata_type const* type, bool allow_generic = true)
    {
        if (allow_generic)
        {
            if (auto generic_inst_ptr = dynamic_cast<generic_inst const*>(type))
            {
                return is_interface(generic_inst_ptr->generic_type());
            }
        }

        return dynamic_cast<interface_type const*>(type) != nullptr;
    }

    inline bool is_interface(metadata_type const& type, bool allow_generic = true)
    {
        return is_interface(&type, allow_generic);
    }

    inline bool is_class(metadata_type const* type)
    {
        return dynamic_cast<class_type const*>(type) != nullptr;
    }

    inline bool is_reference_type(metadata_type const* type)
    {
        if (is_class(type) || is_interface(type) || is_delegate(type))
        {
            return true;
        }

        if (auto elem = dynamic_cast<element_type const*>(type))
        {
            return elem->type() == winmd::reader::ElementType::Object;
        }

        return false;
    }

    inline bool is_element_type(metadata_type const* type, winmd::reader::ElementType element_type)
    {
        if (auto elem = dynamic_cast<element_type const*>(type))
        {
            return elem->type() == element_type;
        }

        return false;
    }

    inline bool is_boolean(metadata_type const* signature)
    {
        return is_element_type(signature, winmd::reader::ElementType::Boolean);
    }

    inline bool is_floating_point(metadata_type const* signature)
    {
        if (auto element_type_ptr = dynamic_cast<element_type const*>(signature))
        {
            return element_type_ptr->type() == winmd::reader::ElementType::R4 ||
                element_type_ptr->type() == winmd::reader::ElementType::R8;
        }

        return false;
    }

    inline bool is_string(metadata_type const* signature)
    {
        return is_element_type(signature, winmd::reader::ElementType::String);
    }

    inline bool is_winrt_ireference(metadata_type const& type)
    {
        return type.swift_full_name().starts_with("Windows.Foundation.IReference");
    }

    inline bool is_winrt_ireference(metadata_type const* type)
    {
        return is_winrt_ireference(*type);
    }

    inline bool is_winrt_eventhandler(metadata_type const& type)
    {
        return type.swift_full_name().starts_with("Windows.Foundation.EventHandler");
    }

    inline bool is_winrt_eventhandler(metadata_type const* type)
    {
        return is_winrt_eventhandler(*type);
    }

    inline bool is_winrt_typedeventhandler(metadata_type const& type)
    {
        return type.swift_full_name().starts_with("Windows.Foundation.TypedEventHandler");
    }

    inline bool is_winrt_typedeventhandler(metadata_type const* type)
    {
        return is_winrt_typedeventhandler(*type);
    }

    inline bool is_winrt_async_result_type(metadata_type const& type)
    {
        if (type.swift_logical_namespace() == winrt_foundation_namespace)
        {
            auto name = type.swift_type_name();
            return name == "IAsyncAction" || name.starts_with("IAsyncOperation");
        }

        return false;
    }

    inline bool is_winrt_generic_collection(metadata_type const& type)
    {
        if (type.swift_logical_namespace() == winrt_collections_namespace)
        {
            auto name = type.swift_type_name();
            return name.starts_with("IVector`") ||
                name.starts_with("IVectorView`") ||
                name.starts_with("IMap`") ||
                name.starts_with("IMapView`") ||
                name.starts_with("IIterator`") ||
                name.starts_with("IIterable`") ||
                name.starts_with("IObservableMap`") ||
                name.starts_with("IObservableVector`");
        }

        return false;
    }

    inline bool is_winrt_generic_collection(metadata_type const* type)
    {
        return is_winrt_generic_collection(*type);
    }

    inline bool needs_collection_conformance(metadata_type const& type)
    {
        if (type.swift_logical_namespace() == winrt_collections_namespace)
        {
            auto name = type.swift_type_name();
            return name.starts_with("IVector`") ||
                name.starts_with("IVectorView`") ||
                name.starts_with("IObservableVector`");
        }

        return false;
    }

    inline bool needs_collection_conformance(metadata_type const* type)
    {
        return needs_collection_conformance(*type);
    }
}
