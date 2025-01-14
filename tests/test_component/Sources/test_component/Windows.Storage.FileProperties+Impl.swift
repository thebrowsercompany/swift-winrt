// WARNING: Please don't edit this file. It was generated by Swift/WinRT v0.0.1
// swiftlint:disable all
import Foundation
import Ctest_component

@_spi(WinRTInternal)
public enum __IMPL_Windows_Storage_FileProperties {
    public enum IStorageItemExtraPropertiesBridge : AbiInterfaceBridge {
        public typealias CABI = __x_ABI_CWindows_CStorage_CFileProperties_CIStorageItemExtraProperties
        public typealias SwiftABI = __ABI_Windows_Storage_FileProperties.IStorageItemExtraProperties
        public typealias SwiftProjection = AnyIStorageItemExtraProperties
        public static func from(abi: ComPtr<CABI>?) -> SwiftProjection? {
            guard let abi = abi else { return nil }
            return IStorageItemExtraPropertiesImpl(abi)
        }

        public static func makeAbi() -> CABI {
            let vtblPtr = withUnsafeMutablePointer(to: &__ABI_Windows_Storage_FileProperties.IStorageItemExtraPropertiesVTable) { $0 }
            return .init(lpVtbl: vtblPtr)
        }
    }

    fileprivate class IStorageItemExtraPropertiesImpl: IStorageItemExtraProperties, WinRTAbiImpl {
        fileprivate typealias Bridge = IStorageItemExtraPropertiesBridge
        fileprivate let _default: Bridge.SwiftABI
        fileprivate var thisPtr: test_component.IInspectable { _default }
        fileprivate init(_ fromAbi: ComPtr<Bridge.CABI>) {
            _default = Bridge.SwiftABI(fromAbi)
        }

        /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.fileproperties.istorageitemextraproperties.retrievepropertiesasync)
        fileprivate func retrievePropertiesAsync(_ propertiesToRetrieve: AnyIIterable<String>!) throws -> AnyIAsyncOperation<AnyIMap<String, Any?>?>! {
            try _default.RetrievePropertiesAsync(propertiesToRetrieve)
        }

        /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.fileproperties.istorageitemextraproperties.savepropertiesasync)
        fileprivate func savePropertiesAsync(_ propertiesToSave: AnyIIterable<AnyIKeyValuePair<String, Any?>?>!) throws -> test_component.AnyIAsyncAction! {
            try _default.SavePropertiesAsync(propertiesToSave)
        }

        /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.fileproperties.istorageitemextraproperties.savepropertiesasync)
        fileprivate func savePropertiesAsync() throws -> test_component.AnyIAsyncAction! {
            try _default.SavePropertiesAsyncOverloadDefault()
        }

    }

    public enum BasicPropertiesBridge: AbiBridge {
        public typealias SwiftProjection = BasicProperties
        public typealias CABI = __x_ABI_CWindows_CStorage_CFileProperties_CIBasicProperties
        public static func from(abi: ComPtr<__x_ABI_CWindows_CStorage_CFileProperties_CIBasicProperties>?) -> BasicProperties? {
            guard let abi = abi else { return nil }
            return .init(fromAbi: test_component.IInspectable(abi))
        }
    }

    public enum DocumentPropertiesBridge: AbiBridge {
        public typealias SwiftProjection = DocumentProperties
        public typealias CABI = __x_ABI_CWindows_CStorage_CFileProperties_CIDocumentProperties
        public static func from(abi: ComPtr<__x_ABI_CWindows_CStorage_CFileProperties_CIDocumentProperties>?) -> DocumentProperties? {
            guard let abi = abi else { return nil }
            return .init(fromAbi: test_component.IInspectable(abi))
        }
    }

    public enum ImagePropertiesBridge: AbiBridge {
        public typealias SwiftProjection = ImageProperties
        public typealias CABI = __x_ABI_CWindows_CStorage_CFileProperties_CIImageProperties
        public static func from(abi: ComPtr<__x_ABI_CWindows_CStorage_CFileProperties_CIImageProperties>?) -> ImageProperties? {
            guard let abi = abi else { return nil }
            return .init(fromAbi: test_component.IInspectable(abi))
        }
    }

    public enum MusicPropertiesBridge: AbiBridge {
        public typealias SwiftProjection = MusicProperties
        public typealias CABI = __x_ABI_CWindows_CStorage_CFileProperties_CIMusicProperties
        public static func from(abi: ComPtr<__x_ABI_CWindows_CStorage_CFileProperties_CIMusicProperties>?) -> MusicProperties? {
            guard let abi = abi else { return nil }
            return .init(fromAbi: test_component.IInspectable(abi))
        }
    }

    public enum StorageItemContentPropertiesBridge: AbiBridge {
        public typealias SwiftProjection = StorageItemContentProperties
        public typealias CABI = __x_ABI_CWindows_CStorage_CFileProperties_CIStorageItemContentProperties
        public static func from(abi: ComPtr<__x_ABI_CWindows_CStorage_CFileProperties_CIStorageItemContentProperties>?) -> StorageItemContentProperties? {
            guard let abi = abi else { return nil }
            return .init(fromAbi: test_component.IInspectable(abi))
        }
    }

    public enum StorageItemThumbnailBridge: AbiBridge {
        public typealias SwiftProjection = StorageItemThumbnail
        public typealias CABI = __x_ABI_CWindows_CStorage_CStreams_CIRandomAccessStreamWithContentType
        public static func from(abi: ComPtr<__x_ABI_CWindows_CStorage_CStreams_CIRandomAccessStreamWithContentType>?) -> StorageItemThumbnail? {
            guard let abi = abi else { return nil }
            return .init(fromAbi: test_component.IInspectable(abi))
        }
    }

    public enum VideoPropertiesBridge: AbiBridge {
        public typealias SwiftProjection = VideoProperties
        public typealias CABI = __x_ABI_CWindows_CStorage_CFileProperties_CIVideoProperties
        public static func from(abi: ComPtr<__x_ABI_CWindows_CStorage_CFileProperties_CIVideoProperties>?) -> VideoProperties? {
            guard let abi = abi else { return nil }
            return .init(fromAbi: test_component.IInspectable(abi))
        }
    }

}
