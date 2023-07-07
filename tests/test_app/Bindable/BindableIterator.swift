import test_component

protocol GetAtAndSize {
    func getAt(_ index: UInt32) throws -> Any!
    var size : UInt32 { get }
}

extension BindableVector: GetAtAndSize {}
extension BindableVectorView: GetAtAndSize {}

class BindableIterator: IBindableIterator {

    private var vector: GetAtAndSize
    private var index: UInt32 = 0
    init(_ vector: GetAtAndSize) {
        self.vector = vector
    }   

    func moveNext() throws -> Bool {
        index+=1
        return index < vector.size
    }

    var current: Any! {
        try! vector.getAt(index)
    }

    var hasCurrent: Bool {
        index < vector.size
    }

}
