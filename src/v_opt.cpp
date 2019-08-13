#include "v_opt.h"
#include <cstdlib>

ArgIter IntArg::parse(ArgIter iter) {
  char* end;
  m_storage = int(strtol(&*(*(++iter)).begin(),&end,0));
  if (&*(*(++iter)).end()!=end) {
    // ERROR
  }
  return ++iter;
}

ArgIter SwitchArg::parse(ArgIter iter) {
  m_storage = true;
  return ++iter;
}

std::string IntArg::completion_entry() {
  std::string retval;
  retval += m_name;
  retval += "[" + m_doc + "]";
  retval += ":" + m_shortdoc + ":";
  return retval;
}

std::string FileArg::completion_entry() {
  std::string retval;
  retval += m_name;
  retval += "[" + m_doc + "]";
  retval += ":" + m_shortdoc + ":";
  retval += " _files -g " + m_pattern;
  return retval;
}

std::string DirectoryArg::completion_entry() {
  std::string retval;
  retval += m_name;
  retval += "[" + m_doc + "]";
  retval += ":" + m_shortdoc + ":";
  retval += " _files -g -/";
  return retval;
}

std::string StringArg::completion_entry() {
  std::string retval;
  retval += m_name;
  retval += "[" + m_doc + "]";
  retval += ":" + m_shortdoc + ":";
  return retval;
}

std::string SwitchArg::completion_entry() {
  std::string retval;
  retval += m_name;
  retval += "[" + m_doc + "]";
  return retval;
}
