### Względy

`QMenuBar` wyrenderuje dzieci nie w pasku ale w wyskakującym oknie, wraz z akcjami.

### C++

Qt uses C++ with the MOC code genertor. To use the symbols in the same way as C, we have to deal with issues of ABI on all the supported platforms. Namely:

1. Symbol mangling obscures symbols. This is not even necessarily portable between compilers. To get access to intercepted symbols via `dlsym` et al., we need *mangling* support at runtime. Where to get it? Even within a given compiler, the mangling may be dependent on e.g. c++ standard used in program and/or compiler flags.
 
2. Issues with redefining symbols declared in (possibly private) headers in a semi-automatic manner may be more severe now due to namespaces etc. Need a code generator

3. C++ has more complex semantics, and many function calls are implicit (constructors, destructors). The logic to determine them is in the compiler. Also their signatures may actually differ from their ABI, so it's not a matter of just tranmitting the received arguments.

This is a big challenge to any Tetradactyl backend targetting a C++ framework.
