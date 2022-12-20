// ISO C requires a translation unit to contain at least one declaration.
//
// To force the linker to produce a .lib file, currently required for dynamic
// linking, at least the way our CMakeLists.txt are currently structured, we
// also need to export a symbol.
extern unsigned _;

// ISO C requires a translation unit to contain at least one declaration.
//
// To force the linker to produce a .lib file, currently required for dynamic
// linking, at least the way our CMakeLists.txt are currently structured, we
// also need to export a symbol.
#if defined(Ctest_component_EXPORTS)
  #define CWINRT_ABI __declspec(dllexport)
#else
  #define CWINRT_ABI __declspec(dllimport)
#endif

// ISO C requires a translation unit to contain at least one declaration.
//
// To force the linker to produce a .lib file, currently required for dynamic
// linking, at least the way our CMakeLists.txt are currently structured, we
// also need to export a symbol.
int CWINRT_ABI force_lib_generation(void) {
  return 1;
}