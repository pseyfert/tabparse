#include "parser.h"
#include "v_opt.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <regex>
#include <iostream>
#include <fstream>
#include <fmt/format.h>

template <typename ARGTYPE, typename ...OTHERARGS>
typename ARGTYPE::type &
Parser::addArg(std::string_view name, typename ARGTYPE::type default_value,
               std::string_view shortdoc, std::string_view doc, OTHERARGS... otherargs) {
  auto thearg = std::make_unique<ARGTYPE>(std::move(name), std::move(default_value), std::move(shortdoc), std::move(doc), std::forward<OTHERARGS>(otherargs)...);
  auto& retval = thearg->m_storage;
  m_args.push_back(std::move(thearg));
  return retval;
}

void Parser::print_completion(std::string_view appname) {
  std::ofstream outfile;
  std::string fname;
  fname.reserve(appname.size());
  std::regex re{"^\\./"};
  std::regex_replace(std::back_inserter(fname), appname.begin(), appname.end(), re, "");

  outfile.open(fmt::format("_{}",fname));
  outfile << "#compdef " << fname << '\n';
  outfile << '\n';
  outfile << "_arguments \\" << '\n';
  if (!m_args.empty()) {
    for (size_t i = 0 ; i < m_args.size() -1 ; ++i) {
      outfile << "  \"" << m_args[i]->completion_entry() << "\" \\\n";
    }
    outfile << "  \"" << m_args.back()->completion_entry() << "\"\n";
  }
  outfile.close();
}

void Parser::print_help(std::string_view appname) {
  auto max_length =
      (*std::max_element(m_args.begin(), m_args.end(),
                         [](const auto &a, const auto &b) {
                           return a->m_name.size() < b->m_name.size();
                         }))
          ->m_name.size();

  std::cout << "USAGE: " << appname << '\n';
  std::cout << '\n';
  for (const auto& arg: m_args) {
    fmt::print("  {:<{}}{}\n", arg->m_name, max_length + 3, arg->m_doc);
  }
}

void Parser::parse(int argc, char *argv[]) {
  std::vector<std::string> inargs(argv + 1, argv + argc);
  auto findres = std::find(inargs.begin(), inargs.end(), "--help");
  if (findres != inargs.end()) {
    print_help(argv[0]);
    return;
  }
  findres = std::find(inargs.begin(), inargs.end(), "complete");
  if (findres != inargs.end()) {
    print_completion(argv[0]);
    return;
  }
  for (auto iter = inargs.begin();
      iter != inargs.end();
      /* increment in parse method */) {
    for (auto &arg: m_args) {
      if (*iter==arg->m_name) {
        iter = arg->parse(iter);
      }
    }
  }
}
template std::string& Parser::addArg<DirectoryArg>(std::string_view, std::string, std::string_view, std::string_view);
template std::string& Parser::addArg<FileArg>(std::string_view, std::string, std::string_view, std::string_view, std::string_view);
template std::string& Parser::addArg<StringArg>(std::string_view, std::string, std::string_view, std::string_view);
template int& Parser::addArg<IntArg>(std::string_view, int, std::string_view, std::string_view);
template bool& Parser::addArg<SwitchArg>(std::string_view, bool, std::string_view, std::string_view);
template std::string& Parser::addArg<StringChoiceArg>(std::string_view, std::string, std::string_view, std::string_view, std::initializer_list<std::string>);
