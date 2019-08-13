#pragma once

#include <string>
#include <vector>
#include <string_view>
class Parser;

// class ArgIter;
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
  protected:
    std::string completion_entry() override;
    ArgIter parse(ArgIter iter) override {
      m_storage = *(++iter);
      return ++iter;
    }
};

class FileArg : public StringArg {
  public:
    using StringArg::StringArg;
  protected:
    std::string completion_entry() override;
    std::string m_pattern;
};

class DirectoryArg : public StringArg {
  public:
    using StringArg::StringArg;
  protected:
    std::string completion_entry() override;
};

class IntArg : public TemplateArg<int> {
  public:
    using TemplateArg<int>::TemplateArg;
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
  protected:
    std::string completion_entry() override;
    ArgIter parse(ArgIter) override;
    bool m_storage{false};
};
