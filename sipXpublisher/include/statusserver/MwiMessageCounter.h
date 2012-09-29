#ifndef MWIMESSAGECOUNTER_H
#define	MWIMESSAGECOUNTER_H

#include <string>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

class MwiMessageCounter
{
public:
  MwiMessageCounter(const std::string& user) :
    _user(user),
    _old(0),
    _new(0),
    _urgentOld(0),
    _urgentNew(0)
  {
    prepareCounters();
  }

  int getOldMessages() const
  {
    return _old;
  }

  int getNewMessages() const
  {
    return _new;
  }

  int getOldUrgentMessages() const
  {
    return _urgentOld;
  }

  int getNewUrgentMessages() const
  {
    _urgentNew;
  }

  std::string getMailBoxData(const std::string& domain)
  {
    std::ostringstream mwi;
    if (_new)
      mwi << "Message-Waiting: yes\r\n";
    else
      mwi << "Message-Waiting: no\r\n";

    mwi << "Message-Account: sip:" << _user << "@" << domain << "\r\n";
    mwi << "Voice-Message: " << _new << "/" << _old << " (" << _urgentNew << "/" << _urgentOld << ")" << "\r\n\r\n";
    return mwi.str();
  }

private:
  std::string boost_file_name(const boost::filesystem::path& path)
  {
  #if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
    return path.filename().native();
  #else
    return path.filename();
  #endif
  }

  bool string_ends_with(const std::string& str, const char* key)
  {
    size_t i = str.rfind(key);
    return (i != std::string::npos) && (i == (str.length() - ::strlen(key)));
  }

  std::vector<std::string> string_tokenize(const std::string& str, const char* tok)
  {
    std::vector<std::string> tokens;
    boost::split(tokens, str, boost::is_any_of(tok), boost::token_compress_on);
    return tokens;
  }

  void prepareCounters()
  {
    std::string mailStore (SIPX_VARDIR "/mediaserver/data/mailstore");
    std::ostringstream userInbox;
    userInbox << mailStore << "/" << _user << "/inbox";
    std::string directory = userInbox.str();
    
    std::vector<std::string> messages;
    try
    {
      boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
      for (boost::filesystem::directory_iterator itr(directory); itr != end_itr; ++itr)
      {
        if (boost::filesystem::is_directory(itr->status()))
        {
          continue;
        }
        else
        {
          boost::filesystem::path currentFile = itr->path();
          std::string fileName = boost_file_name(currentFile);
          if (boost::filesystem::is_regular(currentFile))
          {
            if (string_ends_with(fileName, "-00.xml"))
            {
              std::vector<std::string> tokens = string_tokenize(fileName, "-");
              if (tokens.size() == 2);
              {
                std::ostringstream sta;
                sta << directory << "/" << tokens[0] << "-00.sta";

                std::ostringstream urg;
                urg << directory << "/" << tokens[0] << "-00.urg";

                if (boost::filesystem::exists(sta.str().c_str()))
                  _new++;
                else if (boost::filesystem::exists(urg.str().c_str()))
                  _urgentOld++;
                else
                  _old++;
              }
            }
          }
        }
      }
    }
    catch(...)
    {
    }
  }


  

protected:
  std::string _user;
  int _old;
  int _new;
  int _urgentOld;
  int _urgentNew;
};


#endif	/* MWIMESSAGECOUNTER_H */

