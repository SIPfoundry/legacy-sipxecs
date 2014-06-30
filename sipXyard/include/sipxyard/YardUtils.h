/* 
 * File:   Utils.h
 * Author: joegen
 *
 * Created on June 24, 2014, 4:04 PM
 */

#ifndef YARD_UTILS_H_INCLUDED
#define	YARD_UTILS_H_INCLUDED

#include <string>
#include <vector>
#include <unistd.h>
#include <sipxyard/LevelDB.h>

class YardUtils
{
public:
  static bool string_starts_with(const std::string& str, const char* key);

  static bool string_ends_with(const std::string& str, const char* key);

  static std::vector<std::string> string_tokenize(const std::string& str, const char* tok);

  static void get_path_vector(const std::string& path, std::vector<std::string>& pathVector);

  static void prepare_path(std::string& path);
  
  static std::string path_to_dot_notation(const std::string& path, std::size_t depth = 0);
   
  static std::string get_file_name(const std::string& path);
  
  static void json_print(const std::vector<std::string>& pathVector, LevelDB::Records& records, std::ostream& ostr);

};


#endif	/* UTILS_H */

