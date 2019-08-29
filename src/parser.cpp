#include "parser.h"
#include "v_opt.h"
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <algorithm>
#include <regex>
#include <iostream>
#include <fstream>
#include <fmt/format.h>

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
  return static_cast<ARGTYPE*>(m_args.back().get());
}

template <typename ARGTYPE, typename ...OTHERARGS>
ARGTYPE*
Parser::addPosArg(typename ARGTYPE::type default_value,
                  std::string_view shortdoc, std::string_view doc, OTHERARGS... otherargs) {
  auto thearg = std::make_unique<ARGTYPE>(fmt::format("{}", m_pos.size()+1), std::move(default_value), shortdoc, doc, std::forward<OTHERARGS>(otherargs)...);
  m_pos.push_back(std::move(thearg));
  return static_cast<ARGTYPE*>(m_pos.back().get());
}

template <typename BASE_ARG, typename ...OTHERARGS>
MultiArg<BASE_ARG>* Parser::addOther(std::string_view shortdoc, std::string_view doc, OTHERARGS... otherargs) {
  m_others = std::make_unique<MultiArg<BASE_ARG>>("*", typename BASE_ARG::type{}, shortdoc, doc, std::forward<OTHERARGS>(otherargs)...);
  return static_cast<MultiArg<BASE_ARG>*>(m_others.get());
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
    for (std::size_t i = 0 ; i < m_args.size() -1 ; ++i) {
      outfile << "  \"" << m_args[i]->completion_entry(false) << "\" \\\n";
    }
    outfile << "  \"" << m_args.back()->completion_entry(false) << "\"";
    if (!m_pos.empty() || m_others) {
      // if there is more to come
      outfile << " \\";
    }
    outfile << "\n";
  }
  if (!m_pos.empty()) {
    for (std::size_t i = 0 ; i < m_pos.size() -1 ; ++i) {
      outfile << "  \"" << m_pos[i]->completion_entry(true) << "\" \\\n";
    }
    outfile << "  \"" << m_pos.back()->completion_entry(true) << "\"";
    if (m_others) {
      // if there is more to come
      outfile << " \\";
    }
    outfile << "\n";
  }
  if (m_others) {
    outfile << "  \"" << m_others->completion_entry(true) << "\"\n";
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
  for (const auto& arg: m_args) {
    if (arg->m_flags.test(ArgFlags::Required)) {
      fmt::print(" {} {}", arg->m_name, arg->m_shortdoc);
    }
  }
  for (const auto& pos: m_pos) {
    if (pos->m_flags.test(ArgFlags::Required)) {
      fmt::print(" {}", pos->m_shortdoc);
    } else {
      fmt::print(" [{}]", pos->m_shortdoc);
    }
  }
  std::cout << '\n';
  std::cout << '\n';
  for (const auto& arg: m_args) {
    fmt::print("  {} {:<{}}{}\n", arg->m_name, arg->m_shortdoc, max_length + 2 - arg->m_name.size(), arg->m_doc);
  }
}

void Parser::sanitize() {
  std::size_t i = m_pos.size();
  for (; i > 0 ; i--) {
    if (m_pos[i-1]->m_flags.test(ArgFlags::Required)) {
      break;
    }
  }
  for (; i > 0 ; i--) {
    m_pos[i-1]->m_flags.set(ArgFlags::Required);
  }
}

void Parser::parse(int argc, char *argv[]) {
  sanitize();
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
      if (m_pos.empty() && !m_others) {
        throw std::invalid_argument(fmt::format("did not identify {} as option and did not expect positional arguments.", *iter));
      }
      if (posarg_iter != m_pos.end()) {
        (*posarg_iter)->parse(iter++);
        posarg_iter++;
      } else {
        if (m_others) {
          // TODO: can not interleave any m_args after this point :(
          iter = m_others->parse(iter, inargs.end());
        } else {
          throw std::invalid_argument(fmt::format("no more positional arguments expected, received {}.", *iter));
        }
      }
    }
  }
  for (const auto& arg : m_args) {
    if (!arg->m_flags.test(ArgFlags::Present)) {
      if (arg->m_flags.test(ArgFlags::Required)) {
        throw std::invalid_argument(fmt::format("required argument {} not used.", arg->m_name));
      }
    }
  }
  for (const auto& arg : m_pos) {
    if (!arg->m_flags.test(ArgFlags::Present)) {
      if (arg->m_flags.test(ArgFlags::Required)) {
        throw std::invalid_argument(fmt::format("required argument {} not used.", arg->m_shortdoc));
      }
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

template MultiArg<DirectoryArg>* Parser::addOther<DirectoryArg>(std::string_view shortdoc, std::string_view doc);
template MultiArg<FileArg>* Parser::addOther<FileArg>(std::string_view shortdoc, std::string_view doc, std::string_view pattern);
template MultiArg<StringArg>* Parser::addOther<StringArg>(std::string_view shortdoc, std::string_view doc);
template MultiArg<IntArg>* Parser::addOther<IntArg>(std::string_view shortdoc, std::string_view doc);
// template MultiArg<SwitchArg>* Parser::addOther<SwitchArg>(std::string_view shortdoc, std::string_view doc);
template MultiArg<StringChoiceArg>* Parser::addOther<StringChoiceArg>(std::string_view shortdoc, std::string_view doc, std::initializer_list<std::string>);
template MultiArg<StringChoiceArg>* Parser::addOther<StringChoiceArg>(std::string_view shortdoc, std::string_view doc, std::initializer_list<std::string>, std::initializer_list<std::string>);

template VectorArg<DirectoryArg>* Parser::addArg<VectorArg<DirectoryArg>>(std::string_view, std::string, std::string_view, std::string_view);
template VectorArg<FileArg>* Parser::addArg<VectorArg<FileArg>>(std::string_view, std::string, std::string_view, std::string_view, std::string_view);
template VectorArg<StringArg>* Parser::addArg<VectorArg<StringArg>>(std::string_view, std::string, std::string_view, std::string_view);
template VectorArg<IntArg>* Parser::addArg<VectorArg<IntArg>>(std::string_view, int, std::string_view, std::string_view);
// template VectorArg<SwitchArg>* Parser::addArg<VectorArg<SwitchArg>>(std::string_view, bool, std::string_view, std::string_view);
template VectorArg<StringChoiceArg>* Parser::addArg<VectorArg<StringChoiceArg>>(std::string_view, std::string, std::string_view, std::string_view, std::initializer_list<std::string>);
template VectorArg<StringChoiceArg>* Parser::addArg<VectorArg<StringChoiceArg>>(std::string_view, std::string, std::string_view, std::string_view, std::initializer_list<std::string>, std::initializer_list<std::string>);
