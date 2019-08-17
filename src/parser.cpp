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
ARGTYPE*
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
  auto thearg = std::make_unique<ARGTYPE>(name, std::move(default_value), shortdoc, doc, std::forward<OTHERARGS>(otherargs)...);
  m_args.push_back(std::move(thearg));
  return reinterpret_cast<ARGTYPE*>(m_args.back().get());
}

template <typename ARGTYPE, typename ...OTHERARGS>
ARGTYPE*
Parser::addPosArg(typename ARGTYPE::type default_value,
                  std::string_view shortdoc, std::string_view doc, OTHERARGS... otherargs) {
  auto thearg = std::make_unique<ARGTYPE>(fmt::format("{}", m_pos.size()+1), std::move(default_value), shortdoc, doc, std::forward<OTHERARGS>(otherargs)...);
  m_pos.push_back(std::move(thearg));
  return reinterpret_cast<ARGTYPE*>(m_pos.back().get());
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
    if (!m_pos.empty()) {
      outfile << " \\";
    }
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

  auto printlength = [](const auto& arg_unique_ptr) {
    return arg_unique_ptr->m_name.size() + 1 + arg_unique_ptr->m_shortdoc.size();
  };
  auto max_length = printlength(
      *std::max_element(m_args.begin(), m_args.end(),
                        [printlength](const auto &a, const auto &b) {
                          return printlength(a) < printlength(b);
                        }));

  std::cout << "USAGE: " << appname;
  for (const auto& pos: m_pos) {
    fmt::print(" {}", pos->m_shortdoc);
  }
  std::cout << '\n';
  std::cout << '\n';
  for (const auto& arg: m_args) {
    fmt::print("  {} {:<{}}{}\n", arg->m_name, arg->m_shortdoc, max_length + 2 - arg->m_name.size(), arg->m_doc);
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
      }
      if (posarg_iter != m_pos.end()) {
        (*posarg_iter)->parse(iter++);
        posarg_iter++;
      } else {
      throw std::invalid_argument(fmt::format("no more positional arguments expected, received {}.", *iter));
      }
    }
  }
  for (const auto& arg : boost::range::join( m_args, m_pos)) {
    if (!arg->m_flags.test(ArgFlags::Present)) {
      if (arg->m_flags.test(ArgFlags::Required)) {
        throw std::invalid_argument(fmt::format("required argument {} not used.", arg->m_name));
      }
      // arg->m_storage = arg->m_default;
    }
  }

}

template DirectoryArg* Parser::addArg<DirectoryArg>(std::string_view, std::string, std::string_view, std::string_view);
template FileArg* Parser::addArg<FileArg>(std::string_view, std::string, std::string_view, std::string_view, std::string_view);
template StringArg* Parser::addArg<StringArg>(std::string_view, std::string, std::string_view, std::string_view);
template IntArg* Parser::addArg<IntArg>(std::string_view, int, std::string_view, std::string_view);
template SwitchArg* Parser::addArg<SwitchArg>(std::string_view, bool, std::string_view, std::string_view);
template StringChoiceArg* Parser::addArg<StringChoiceArg>(std::string_view, std::string, std::string_view, std::string_view, std::initializer_list<std::string>);
template StringChoiceArg* Parser::addArg<StringChoiceArg>(std::string_view, std::string, std::string_view, std::string_view, std::initializer_list<std::string>, std::initializer_list<std::string>);

template DirectoryArg* Parser::addPosArg<DirectoryArg>(std::string, std::string_view, std::string_view);
template FileArg* Parser::addPosArg<FileArg>(std::string, std::string_view, std::string_view, std::string_view);
template StringArg* Parser::addPosArg<StringArg>(std::string, std::string_view, std::string_view);
template IntArg* Parser::addPosArg<IntArg>(int, std::string_view, std::string_view);
// template bool& Parser::addPosArg<SwitchArg>(bool, std::string_view, std::string_view);
template StringChoiceArg* Parser::addPosArg<StringChoiceArg>(std::string, std::string_view, std::string_view, std::initializer_list<std::string>);
template StringChoiceArg* Parser::addPosArg<StringChoiceArg>(std::string, std::string_view, std::string_view, std::initializer_list<std::string>, std::initializer_list<std::string>);
