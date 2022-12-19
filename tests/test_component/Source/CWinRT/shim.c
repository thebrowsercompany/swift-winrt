// ISO C requires a translation unit to contain at least one declaration.
//
// To force the linker to produce a .lib file, currently required for dynamic
// linking, at least the way our CMakeLists.txt are currently structured, we
// also need to export a symbol.
extern unsigned _;
