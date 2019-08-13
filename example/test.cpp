#include "parser.h"
#include "v_opt.h"

int main(int argc, char** argv) {
  Parser p;
  p.addArg<DirectoryArg>("--build-dir", ".", "BUILDDIR", "specify the build directory");
  p.addArg<FileArg>("--some-file", "main.cpp", "FILE", "specify some file", std::string_view{"*.cpp"});
  p.addArg<StringArg>("--name", "themaster", "NAME", "specify some name");
  p.addArg<IntArg>("-j", 42, "CONCURRENCY", "specify the concurrency level");
  p.addArg<StringChoiceArg>("--mode", "demonstrate", "MODE", "specify running mode", std::initializer_list<std::string>{"demonstrate\\:'demonstrate usage'", "party\\:'do something crazy'", "lazy\\:'do nothing'"});

  p.parse(argc, argv);
  return 0;
}
