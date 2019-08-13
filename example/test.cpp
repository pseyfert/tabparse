#include "parser.h"
#include "v_opt.h"

int main(int argc, char** argv) {
  Parser p;
  p.addArg<DirectoryArg>("--build-dir", ".", "BUILDDIR", "specify the build directory");
  p.parse(argc, argv);
  return 0;
}
