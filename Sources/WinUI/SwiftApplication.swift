import Foundation
import WinAppSDK
@_spi(WinRTImplements) import WindowsFoundation

/// You should derive from SwiftApplication and mark this class as your @main entry point, like this:
/// ```
/// import WinUI
///
/// @main
/// class MySwiftApp: SwiftApplication {
///   required init() {
///     super.init()
///   }
///
///  override func onLaunched(_ args: LaunchActivatedEventArgs) {
///    let window = Window()
///    window.content = TextBlock(text: "Hello, world!")
///    window.activate()
///   }
/// ```
open class SwiftApplication: Application, IXamlMetadataProvider {
    public required override init() {
        super.init()
        resources.mergedDictionaries.append(XamlControlsResources())
    }

    public static func main() {
        do {
            try withExtendedLifetime(WindowsAppRuntimeInitializer()) {
                let appClass = String(describing: String(reflecting: Self.self))
                guard let instance = NSClassFromString(appClass) else {
                    fatalError("unable to find application class \(appClass)")
                }
                Application.start { _ in
                    _ = (instance as! SwiftApplication.Type).init()
                }
            }
        }
        catch {
            fatalError("Failed to initialize WindowsAppRuntimeInitializer: \(error)")
        }
    }

    override open func queryInterface(_ iid: WindowsFoundation.IID) -> IUnknownRef? {
        switch iid {
        case __ABI_Microsoft_UI_Xaml_Markup.IXamlMetadataProviderWrapper.IID:
            let ixmp = __ABI_Microsoft_UI_Xaml_Markup.IXamlMetadataProviderWrapper(self)
            return ixmp?.queryInterface(iid)
        default:
            return super.queryInterface(iid)
        }
    }

    private lazy var metadataProvider: XamlControlsXamlMetaDataProvider = .init()
    public func getXamlType(_ type: TypeName) throws -> IXamlType! {
        try metadataProvider.getXamlType(type)
    }

    public func getXamlType(_ fullName: String) throws -> IXamlType! {
        try metadataProvider.getXamlType(fullName)
    }
}
