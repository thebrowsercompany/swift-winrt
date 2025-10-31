// ISO C requires a translation unit to contain at least one declaration.
extern unsigned _;

// To force the linker to produce a .lib file, currently required for dynamic
// linking, at least the way our CMakeLists.txt are currently structured, we
// also need to export a symbol.
#if defined(CWinRT_EXPORTS)
  int __declspec(dllexport) force_lib_generation(void) {
  return 1;
}
#endif
