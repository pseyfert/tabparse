#include "v_opt.h"
#include <cstdlib>
#include <fmt/format.h>
#include <exception>

ArgIter IntArg::parse(ArgIter iter) {
  m_flags.set(ArgFlags::Present);
  char* end;
  m_storage = int(strtol(&*(*iter).begin(),&end,0));
  if (&*(*iter).end()!=end) {
    throw std::invalid_argument(fmt::format("could not parse {} as integer.", *iter));
  }
  return ++iter;
}

template <typename FINAL_ARG>
ArgIter StringArgBase<FINAL_ARG>::parse(ArgIter iter) {
  ArgBase::m_flags.set(ArgFlags::Present);
  TemplateArg<std::string, FINAL_ARG>::m_storage = *iter;
  return ++iter;
}

ArgIter SwitchArg::parse(ArgIter iter) {
  m_flags.set(ArgFlags::Present);
  m_storage = true;
  return iter;
}

std::string IntArg::completion_entry(bool skip_description) {
  std::string retval;
  retval += m_name;
  if (!skip_description) {
    retval += "[" + m_doc + "]";
  }
  retval += ":" + m_shortdoc + ":";
  return retval;
}

std::string FileArg::completion_entry(bool skip_description) {
  std::string retval;
  retval += m_name;
  if (!skip_description) {
    retval += "[" + m_doc + "]";
  }
  retval += ":" + m_shortdoc + ":";
  retval += " _files -g '" + m_pattern + "'";
  return retval;
}

std::string DirectoryArg::completion_entry(bool skip_description) {
  std::string retval;
  retval += m_name;
  if (!skip_description) {
    retval += "[" + m_doc + "]";
  }
  retval += ":" + m_shortdoc + ":";
  retval += " _files -/";
  return retval;
}

std::string StringArg::completion_entry(bool skip_description) {
  std::string retval;
  retval += m_name;
  if (!skip_description) {
    retval += "[" + m_doc + "]";
  }
  retval += ":" + m_shortdoc + ":";
  return retval;
}

std::string StringChoiceArg::completion_entry(bool skip_description) {
  std::string retval;
  retval += m_name;
  if (!skip_description) {
    retval += "[" + m_doc + "]";
  }
  retval += ":" + m_shortdoc + ":";
  if (m_descriptions.empty()) {
    retval += "(";
    for (size_t i = 0; i < m_choices.size() - 1; ++i) {
      retval += m_choices[i] + " ";
    }
    retval += m_choices.back() + ")";
  } else {
    retval += "((";
    for (size_t i = 0; i < m_choices.size() - 1; ++i) {
      retval += m_choices[i] + "\\:'" + m_descriptions[i] + "' ";
    }
    retval += m_choices.back() + "\\:'" + m_descriptions.back() + "'";
    retval += "))";
  }
  return retval;
}

std::string SwitchArg::completion_entry(bool skip_description) {
  std::string retval;
  retval += m_name;
  if (!skip_description) {
    retval += "[" + m_doc + "]";
  }
  return retval;
}

ArgIter StringChoiceArg::parse(ArgIter iter) {
  m_flags.set(ArgFlags::Present);
  m_storage = *iter;
  if (m_choices.end() == std::find(m_choices.begin(), m_choices.end(), m_storage)) {
    throw std::invalid_argument(fmt::format("{} is not a valid choice for {}.", m_storage, m_name));
  }
  return ++iter;
}
