// Copyright 2023 Paweł Sacawa. All rights reserved.

// There is a trick that lets us replace the main symbol in any source with our
// own: declare it weak via this pragma, and then redefine it. First definition
// in the compilation line wins.  This header file is included via the  compiler
// argument -include
// The  alternative "#pragma weak main" effects a toolchain dependency on GCC,
// so we prefer this.
int __attribute__((weak)) main(int argc, char *argv[]);
