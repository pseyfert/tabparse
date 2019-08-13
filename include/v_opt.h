#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <string_view>
#include <exception>

class Parser;
using ArgIter = decltype(std::declval<std::vector<std::string>>().begin());

class ArgBase {
  public:
    friend Parser;
  protected:
    virtual std::string completion_entry() = 0;
    virtual ArgIter parse(ArgIter) = 0;
    std::string m_name;
    std::string m_doc;
    std::string m_shortdoc;
};

template <typename STORAGE_TYPE>
class TemplateArg : public ArgBase {
  public:
    TemplateArg() = default;
    TemplateArg(std::string_view name, STORAGE_TYPE default_value, std::string_view shortdoc, std::string_view doc) : m_default{default_value} {
      ArgBase::m_name = name;
      ArgBase::m_doc = doc;
      ArgBase::m_shortdoc = shortdoc;
    }
    using type = STORAGE_TYPE;
    friend Parser;
  protected:
    STORAGE_TYPE m_storage;
    STORAGE_TYPE m_default;
};

class StringArg : public TemplateArg<std::string> {
  public:
    using TemplateArg<std::string>::TemplateArg;
    virtual ~StringArg() {}
  protected:
    std::string completion_entry() override;
    ArgIter parse(ArgIter iter) override {
      m_storage = *(++iter);
      return ++iter;
    }
};

class StringChoiceArg : public StringArg {
  public:
    StringChoiceArg(std::string_view name, std::string_view default_value,
                    std::string_view shortdoc, std::string_view doc,
                    std::initializer_list<std::string> options,
                    std::initializer_list<std::string> descriptions = {})
        : m_choices{std::move(options)}, m_descriptions{std::move(descriptions)} {
      ArgBase::m_name = name;
      ArgBase::m_shortdoc = shortdoc;
      ArgBase::m_doc = doc;
      TemplateArg<std::string>::m_default = default_value;
      if (m_choices.size() != m_descriptions.size() && !m_descriptions.empty()) {
        throw std::length_error("if descriptions are provided, then one must be provided for each option");
      }
    }
    virtual ~StringChoiceArg() {}
  protected:
    std::string completion_entry() override;
    std::vector<std::string> m_choices;
    std::vector<std::string> m_descriptions;
    ArgIter parse(ArgIter iter) override ;
};

class FileArg : public StringArg {
  public:
    FileArg(std::string_view name, std::string_view default_value,
        std::string_view shortdoc, std::string_view doc,
        std::string_view pattern)
        : m_pattern{std::move(pattern)} {
      ArgBase::m_name = name;
      ArgBase::m_shortdoc = shortdoc;
      ArgBase::m_doc = doc;
      TemplateArg<std::string>::m_default = default_value;
    }
    virtual ~FileArg() {}
  protected:
    std::string completion_entry() override;
    std::string m_pattern;
};

class DirectoryArg : public StringArg {
  public:
    using StringArg::StringArg;
    virtual ~DirectoryArg() {}
  protected:
    std::string completion_entry() override;
};

class IntArg : public TemplateArg<int> {
  public:
    using TemplateArg<int>::TemplateArg;
    virtual ~IntArg() {}
  protected:
    std::string completion_entry() override;
    ArgIter parse(ArgIter) override;
};

// class BoolArg : public TemplateArg<bool> {
//   protected:
//     ArgIter parse(ArgIter) override;
// };

class SwitchArg : public TemplateArg<bool> {
  public:
    SwitchArg(std::string_view name, std::string_view doc) {
      ArgBase::m_name = name;
      ArgBase::m_doc = doc;
    }
    SwitchArg(std::string_view name, bool /*unused*/, std::string_view /*unused*/, std::string_view doc) {
      ArgBase::m_name = name;
      ArgBase::m_doc = doc;
    }
    virtual ~SwitchArg() {}
    friend Parser;
  protected:
    std::string completion_entry() override;
    ArgIter parse(ArgIter) override;
    bool m_storage{false};
};
