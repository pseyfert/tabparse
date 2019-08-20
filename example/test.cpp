#include "parser.h"
#include "v_opt.h"
#include <fmt/format.h>
#include <boost/type_index.hpp>

int main(int argc, char** argv) {
  Parser p;
  auto& a = p.addArg<DirectoryArg>("--build-dir", ".", "BUILDDIR", "specify the build directory")->required(true)->ref();
  auto& b = p.addArg<FileArg>("--some-file", "main.cpp", "FILE", "specify some file", std::string_view{"*.cpp"})->ref();
  auto& c = p.addArg<StringArg>("--name", "themaster", "NAME", "specify some name")->ref();
  auto& d = p.addArg<IntArg>("-j", 42, "CONCURRENCY", "specify the concurrency level")->ref();
  auto& e = p.addArg<StringChoiceArg>("--mode", "demonstrate", "MODE", "specify running mode", std::initializer_list<std::string>{"demonstrate", "party", "lazy"}, std::initializer_list<std::string>{"demonstrate usage", "do something crazy", "do nothing"})->ref();
  auto& f = p.addArg<StringChoiceArg>("--undoc-mode", "demonstrate", "MODE", "specify running mode but don't expect docs", std::initializer_list<std::string>{"demonstrate", "party", "lazy"})->ref();
  auto& g = p.addPosArg<DirectoryArg>("../src", "OTHERDIR", "specify a ship to fly through the delta quadrant")->ref();
  auto& h = p.addOther<DirectoryArg>("OTHERDIR", "specify a ship to fly through the delta quadrant")->ref();

  p.parse(argc, argv);

  fmt::print("build dir   is of type {1} and has value {0}\n", a, boost::typeindex::type_id_with_cvr<decltype(a)>().pretty_name());
  fmt::print("some file   is of type {1} and has value {0}\n", b, boost::typeindex::type_id_with_cvr<decltype(b)>().pretty_name());
  fmt::print("name        is of type {1} and has value {0}\n", c, boost::typeindex::type_id_with_cvr<decltype(c)>().pretty_name());
  fmt::print("concurrency is of type {1} and has value {0}\n", d, boost::typeindex::type_id_with_cvr<decltype(d)>().pretty_name());
  fmt::print("mode        is of type {1} and has value {0}\n", e, boost::typeindex::type_id_with_cvr<decltype(e)>().pretty_name());
  fmt::print("undoc mode  is of type {1} and has value {0}\n", f, boost::typeindex::type_id_with_cvr<decltype(f)>().pretty_name());
  fmt::print("other dir   is of type {1} and has value {0}\n", g, boost::typeindex::type_id_with_cvr<decltype(g)>().pretty_name());
  fmt::print("overflow    is of type {}\n", boost::typeindex::type_id_with_cvr<decltype(h)>().pretty_name());
  for (auto& hhh: h) {
    fmt::print("    overflow has val {}\n", hhh);
  }
  if (h.empty()) {
    fmt::print("    overflow is empty\n");
  }

  return 0;
}
