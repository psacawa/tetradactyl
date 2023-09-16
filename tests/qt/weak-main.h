// There is a trick that lets us replace the main symbol in any source with our
// own: declare it weak via this pragma, and then redefine it. First definition
// in the compilation line wins.  This header file is included via the  compiler
// argument -include
#pragma weak main
