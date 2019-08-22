#pragma once
#include "v_opt.h"
#include <vector>
#include <string_view>
#include <memory>

class Parser {
  private:
    std::vector<std::unique_ptr<ArgBase>> m_args;
    std::vector<std::unique_ptr<ArgBase>> m_pos;
    std::unique_ptr<EndAwareArg> m_others;
  public:
    Parser() {
      m_args.push_back(
          std::make_unique<SwitchArg>("--help", "Print help message.")
          );
    }
    void parse(int argc, char *argv[]);
    void sanitize();
    template <typename ARGTYPE, typename ...OTHERARGS>
    ARGTYPE* addArg(std::string_view name, typename ARGTYPE::type default_value, std::string_view shortdoc, std::string_view doc, OTHERARGS... otherargs);
    template <typename ARGTYPE, typename ...OTHERARGS>
    ARGTYPE* addPosArg(typename ARGTYPE::type default_value, std::string_view shortdoc, std::string_view doc, OTHERARGS... otherargs);
    template <typename BASE_ARG, typename ...OTHERARGS>
    MultiArg<BASE_ARG>* addOther(std::string_view shortdoc, std::string_view doc, OTHERARGS... otherargs);
    void print_help(std::string_view appname);
    void print_completion(std::string_view appname);
};
