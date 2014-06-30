#include <cstring>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include "sipxyard/YardUtils.h"

#define PROC_DIRECTORY "/proc"
#define PROC_CMDLINE "cmdline"

bool YardUtils::string_starts_with(const std::string& str, const char* key)
{
  return str.find(key) == 0;
}

bool YardUtils::string_ends_with(const std::string& str, const char* key)
{
  size_t i = str.rfind(key);
  return (i != std::string::npos) && (i == (str.length() - ::strlen(key)));
}

std::vector<std::string> YardUtils::string_tokenize(const std::string& str, const char* tok)
{
  std::vector<std::string> tokens;
  boost::split(tokens, str, boost::is_any_of(tok), boost::token_compress_on);
  return tokens;
}

void YardUtils::get_path_vector(const std::string& path, std::vector<std::string>& pathVector)
{
  std::vector<std::string> tokens = string_tokenize(path, "/");
  
  for (std::vector<std::string>::const_iterator iter = tokens.begin(); iter != tokens.end(); iter++)
  {
    if (!iter->empty())
      pathVector.push_back(*iter);
  }
}

void YardUtils::prepare_path(std::string& path)
{
  std::vector<std::string> tokens = string_tokenize(path, "?");
  if (tokens.size() > 1)
    path = tokens[0];
  
  if (!string_ends_with(path, "/"))
  {
    path = path + std::string("/");
  }
}

std::string YardUtils::path_to_dot_notation(const std::string& p, std::size_t depth)
{
  std::string path = p;
  prepare_path(path);
  
  std::ostringstream dotNotation;
  std::vector<std::string> pathVector;
  
  YardUtils::get_path_vector(path, pathVector);
  if (depth <= pathVector.size() - 1)
  {
    for (std::size_t i = depth; i < pathVector.size(); i++)
    {
      dotNotation << pathVector[i];
      
      if (i + 1 < pathVector.size())
        dotNotation << ".";
    }
  }
  
  return dotNotation.str();
}

std::string get_file_name(const std::string& pathString)
{
  boost::filesystem::path path(pathString);
  
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
  return path.filename().native();
#else
  return path.filename();
#endif
}


static bool json_print_one(std::size_t filterDepth, const std::string& resourceName, std::vector<LevelDB::Record>& records, std::vector<LevelDB::Record>::iterator& iter, std::ostream& ostr)
{
  //
  // return false if there are no more records
  //
  if (iter == records.end())
    return false;
  
  //
  // This will hold the tokens of the current key
  //
  std::vector<std::string> keyTokens;
  YardUtils::get_path_vector(iter->key, keyTokens);
  
  //
  // This will hold the depth of the current item in the tree
  //
  std::size_t keyDepth = keyTokens.size() - 1;
  
  //
  // This will hold the offSet of the current key relative to the filter depth
  //
  std::size_t keyOffSet = keyDepth - filterDepth;
  
  if (keyOffSet == 0 && keyTokens[keyDepth] == resourceName)
  {
    //
    // This is an exact match.  We print it out and consume the iterator
    //
    ostr << "\"" << keyTokens[keyTokens.size() -1] << "\": " << "\"" << iter->value << "\"";
    iter++;
  }
  else if (keyOffSet > 0)
  {
    //
    // The item falls under a group of elements under the filter tree
    //
    ostr << "\"" << keyTokens[filterDepth] << "\":  {";
    
   
    while (true)
    { 
      std::string previousResource = keyTokens[filterDepth];
      if (!json_print_one(filterDepth + 1, keyTokens[filterDepth + 1], records, iter, ostr))
        break;
      
      keyTokens.clear();
      YardUtils::get_path_vector(iter->key, keyTokens);
      
      if (filterDepth + 1 >= keyTokens.size())
        break;
      
      if (previousResource != keyTokens[filterDepth])
        break;
      
      ostr << ",";
    }
    
    ostr << "}";
  }
  else 
  {
    return false;
  }
  
  return iter != records.end();
}

void YardUtils::json_print(const std::vector<std::string>& pathVector, LevelDB::Records& records, std::ostream& ostr)
{
  ostr << "{";
  std::size_t depth = pathVector.size() - 1;
  std::vector<LevelDB::Record>::iterator iter = records.begin();
  while (json_print_one(depth, pathVector[depth], records, iter, ostr))
    ostr << ",";
  ostr << "}";
}