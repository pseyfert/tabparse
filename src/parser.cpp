#include "parser.h"
#include "v_opt.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <regex>
#include <iostream>
#include <fstream>
#include <fmt/format.h>
#include <boost/range/join.hpp>

template <typename ARGTYPE, typename ...OTHERARGS>
typename ARGTYPE::type &
Parser::addArg(std::string_view name, typename ARGTYPE::type default_value,
               std::string_view shortdoc, std::string_view doc, OTHERARGS... otherargs) {
  auto findres = std::find_if(m_args.begin(), m_args.end(),
                              [name](const auto &existing_arg) {
                                return existing_arg->m_name == name;
                              });
  if (m_args.end() != findres) {
    throw std::invalid_argument(fmt::format("option with name {} already exists", name));
  }
  if (name[0] != '-') {
    throw std::invalid_argument(fmt::format("flag arguments should start with - or --. {} does not.", name));
  }
  auto thearg = std::make_unique<ARGTYPE>(std::move(name), std::move(default_value), std::move(shortdoc), std::move(doc), std::forward<OTHERARGS>(otherargs)...);
  auto& retval = thearg->m_storage;
  m_args.push_back(std::move(thearg));
  return retval;
}

template <typename ARGTYPE, typename ...OTHERARGS>
typename ARGTYPE::type &
Parser::addPosArg(typename ARGTYPE::type default_value,
                  std::string_view shortdoc, std::string_view doc, OTHERARGS... otherargs) {
  auto thearg = std::make_unique<ARGTYPE>(fmt::format("{}", m_pos.size()+1), std::move(default_value), std::move(shortdoc), std::move(doc), std::forward<OTHERARGS>(otherargs)...);
  auto& retval = thearg->m_storage;
  m_pos.push_back(std::move(thearg));
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
      outfile << "  \"" << m_args[i]->completion_entry(false) << "\" \\\n";
    }
    outfile << "  \"" << m_args.back()->completion_entry(false) << "\"";
    if (!m_pos.empty()) outfile << " \\";
    outfile << "\n";
  }
  if (!m_pos.empty()) {
    for (size_t i = 0 ; i < m_pos.size() -1 ; ++i) {
      outfile << "  \"" << m_pos[i]->completion_entry(true) << "\" \\\n";
    }
    outfile << "  \"" << m_pos.back()->completion_entry(true) << "\"\n";
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
  auto posarg_iter = m_pos.begin();
  for (auto iter = inargs.begin();
      iter != inargs.end();
      /* increment in parse method */) {
    auto matchingarg = std::find_if(m_args.begin(), m_args.end(), [iter](const auto& arg) { return *iter == arg->m_name; });
    if (matchingarg != m_args.end()) {
      iter = (*matchingarg)->parse(++iter);
    } else {
      if (m_pos.empty()) {
        throw std::invalid_argument(fmt::format("did not identify {} as option and did not expect positional arguments.", *iter));
      } else {
        if (posarg_iter != m_pos.end()) {
          (*posarg_iter)->parse(iter++);
          posarg_iter++;
        } else {
        throw std::invalid_argument(fmt::format("no more positional arguments expected, received {}.", *iter));
        }
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
template std::string& Parser::addArg<StringChoiceArg>(std::string_view, std::string, std::string_view, std::string_view, std::initializer_list<std::string>, std::initializer_list<std::string>);

template std::string& Parser::addPosArg<DirectoryArg>(std::string, std::string_view, std::string_view);
template std::string& Parser::addPosArg<FileArg>(std::string, std::string_view, std::string_view, std::string_view);
template std::string& Parser::addPosArg<StringArg>(std::string, std::string_view, std::string_view);
template int& Parser::addPosArg<IntArg>(int, std::string_view, std::string_view);
// template bool& Parser::addPosArg<SwitchArg>(bool, std::string_view, std::string_view);
template std::string& Parser::addPosArg<StringChoiceArg>(std::string, std::string_view, std::string_view, std::initializer_list<std::string>);
template std::string& Parser::addPosArg<StringChoiceArg>(std::string, std::string_view, std::string_view, std::initializer_list<std::string>, std::initializer_list<std::string>);
