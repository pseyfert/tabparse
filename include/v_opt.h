#pragma once
#include <string>
#include <vector>
#include <initializer_list>
#include <fmt/format.h>
#include <string_view>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "enumset.h"

class Parser;
using ArgIter = decltype(std::declval<std::vector<std::string>>().begin());

BETTER_ENUM(ArgFlags, int, Required, Present)

class ArgBase {
  public:
    friend Parser;
    virtual ~ArgBase() {}
  protected:
    [[nodiscard]] virtual std::string completion_entry(bool skip_description) = 0;
    virtual ArgIter parse(ArgIter) = 0;
    std::string m_name;
    std::string m_doc;
    std::string m_shortdoc;
    EnumSet<ArgFlags> m_flags{0};
  private:
};

class EndAwareArg {
  public:
    [[nodiscard]] virtual std::string completion_entry(bool skip_description) = 0;
    virtual ArgIter parse(ArgIter, ArgIter) = 0;
    virtual ~EndAwareArg() {}
};

template <typename BASE_ARG>
class MultiArg : public BASE_ARG, public EndAwareArg {
  public:
    [[nodiscard]] ArgIter parse(ArgIter iter, ArgIter end) override {
      for (; iter != end;) {
        iter = BASE_ARG::parse(iter);
        m_allvals.push_back(BASE_ARG::m_storage);
      }
      return iter;
    }
    using BASE_ARG::BASE_ARG;
    // removes TemplateArg::ref from the overload set
    [[nodiscard]] std::vector<typename BASE_ARG::type>& ref() {
      return m_allvals;
    }
    [[nodiscard]] std::string completion_entry(bool skip_description) override {
      return BASE_ARG::completion_entry(skip_description);
    }
  protected:
    std::vector<typename BASE_ARG::type> m_allvals;
};

template <typename BASE_ARG>
class VectorArg : public BASE_ARG {
  public:
    [[nodiscard]] ArgIter parse(ArgIter iter) override {
      iter = BASE_ARG::parse(iter);
      m_allvals.push_back(BASE_ARG::m_storage);
      return iter;
    }
    using BASE_ARG::BASE_ARG;
    // removes TemplateArg::ref from the overload set
    [[nodiscard]] std::vector<typename BASE_ARG::type>& ref() {
      return m_allvals;
    }
    [[nodiscard]] std::string completion_entry(bool skip_description) override {
      return fmt::format("*{}", BASE_ARG::completion_entry(skip_description));
    }
  protected:
    std::vector<typename BASE_ARG::type> m_allvals;
};

template <typename STORAGE_TYPE, typename FINAL_ARG>
class TemplateArg : public ArgBase {
  public:
    TemplateArg() = default;
    TemplateArg(std::string_view name, STORAGE_TYPE default_value, std::string_view shortdoc, std::string_view doc) : m_storage{default_value} {
      ArgBase::m_name = name;
      ArgBase::m_doc = doc;
      ArgBase::m_shortdoc = shortdoc;
    }
    using type = STORAGE_TYPE;
    [[nodiscard]] type& ref() {
      return m_storage;
    }
    FINAL_ARG* required(bool req) {
      if (req) {
        ArgBase::m_flags.set(ArgFlags::Required);
      } else {
        ArgBase::m_flags.reset(ArgFlags::Required);
      }
      return reinterpret_cast<FINAL_ARG*>(this);
    }
    friend Parser;
  protected:
    STORAGE_TYPE m_storage;
};

template <typename FINAL_ARG>
class StringArgBase : public TemplateArg<std::string, FINAL_ARG> {
  public:
    using TemplateArg<std::string, FINAL_ARG>::TemplateArg;
    virtual ~StringArgBase() {}
  protected:
    [[nodiscard]] ArgIter parse(ArgIter iter) override;
};

class StringArg : public StringArgBase<StringArg> {
  public:
    using StringArgBase::StringArgBase;
    virtual ~StringArg() {}
  protected:
    [[nodiscard]] std::string completion_entry(bool skip_description) override;
};

class StringChoiceArg : public StringArgBase<StringChoiceArg> {
  public:
    StringChoiceArg(std::string_view name, std::string_view default_value,
                    std::string_view shortdoc, std::string_view doc,
                    std::initializer_list<std::string> options,
                    std::initializer_list<std::string> descriptions = {})
        : m_choices{std::move(options)}, m_descriptions{std::move(descriptions)} {
      ArgBase::m_name = name;
      ArgBase::m_shortdoc = shortdoc;
      ArgBase::m_doc = doc;
      TemplateArg<std::string, StringChoiceArg>::m_storage = default_value;
      if (m_choices.size() != m_descriptions.size() && !m_descriptions.empty()) {
        throw std::length_error("if descriptions are provided, then one must be provided for each option");
      }
    }
    virtual ~StringChoiceArg() {}
  protected:
    [[nodiscard]] std::string completion_entry(bool skip_description) override;
    std::vector<std::string> m_choices;
    std::vector<std::string> m_descriptions;
    [[nodiscard]] ArgIter parse(ArgIter iter) override ;
};

class FileArg : public StringArgBase<FileArg> {
  public:
    FileArg(std::string_view name, std::string_view default_value,
        std::string_view shortdoc, std::string_view doc,
        std::string_view pattern)
        : m_pattern{std::move(pattern)} {
      ArgBase::m_name = name;
      ArgBase::m_shortdoc = shortdoc;
      ArgBase::m_doc = doc;
      TemplateArg<std::string, FileArg>::m_storage = default_value;
    }
    virtual ~FileArg() {}
  protected:
    [[nodiscard]] std::string completion_entry(bool skip_description) override;
    std::string m_pattern;
};

class DirectoryArg : public StringArgBase<DirectoryArg> {
  public:
    using StringArgBase::StringArgBase;
    virtual ~DirectoryArg() {}
  protected:
    [[nodiscard]] std::string completion_entry(bool skip_description) override;
};

class IntArg : public TemplateArg<int, IntArg> {
  public:
    using TemplateArg<int, IntArg>::TemplateArg;
    virtual ~IntArg() {}
  protected:
    [[nodiscard]] std::string completion_entry(bool skip_description) override;
    [[nodiscard]] ArgIter parse(ArgIter) override;
};

// class BoolArg : public TemplateArg<bool> {
//   protected:
//     ArgIter parse(ArgIter) override;
// };

class SwitchArg : public TemplateArg<bool, SwitchArg> {
  public:
    SwitchArg(std::string_view name, std::string_view doc) {
      ArgBase::m_name = name;
      ArgBase::m_shortdoc = "";
      ArgBase::m_doc = doc;
    }
    SwitchArg(std::string_view name, bool /*unused*/, std::string_view /*unused*/, std::string_view doc) {
      ArgBase::m_name = name;
      ArgBase::m_shortdoc = "";
      ArgBase::m_doc = doc;
    }
    virtual ~SwitchArg() {}
    friend Parser;
  protected:
    [[nodiscard]] std::string completion_entry(bool skip_description) override;
    [[nodiscard]] ArgIter parse(ArgIter) override;
    bool m_storage{false};
};
