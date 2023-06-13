import test_component

class AppDerived : Base {
  // overriding the default initialzer is a simple test so that we don't need to override any
  // other initializers
  override init(){ super.init() }

  var count = 0
  override func onDoTheThing() {
    print("we in the app yoooo")
    count+=1
  }
}

class AppDerived2 : UnsealedDerived {
  var count = 0

  override func onDoTheThing() {
    print("pt2: we in the app yoooo")
    count+=1
  }
}

class AppDerived3 : UnsealedDerived2 {
  var count = 0
  var beforeCount = 0
  override func onDoTheThing() {
    print("pt3: we in the app yoooo")
    count+=1
  }

  override func onBeforeDoTheThing() {
    print("before doing it")
    beforeCount+=1
  }
}

class AppDerivedNoOverrides : BaseNoOverrides {
  override init() {
    super.init()
  }
}


class AppDerivedNoOverrides2 : UnsealedDerivedNoOverrides {
  override init() {
    super.init()
  }
}