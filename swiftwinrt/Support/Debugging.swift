import WinSDK

// Use with debugPrint, dump, etc. to simultaneously output to the debugger and
// console window.
//
// debugPrint("Useful stuff: \(var)", to: &debugOut)
// dump(self, name: "myObjectName", indent: 2, maxDepth: 4, to: &debugOut)
public var debugOut = DebugOutputStream()

public class DebugOutputStream : TextOutputStream {
  public func write(_ text: String) {
    print(text, terminator: "")
    OutputDebugStringA(text)
  }
}

public func debugPrintAddress(_ description: String, _ object: AnyObject) {
  debugPrint(description, terminator: "", to: &debugOut)
  debugPrint(Unmanaged.passUnretained(object).toOpaque(), to: &debugOut)
}
