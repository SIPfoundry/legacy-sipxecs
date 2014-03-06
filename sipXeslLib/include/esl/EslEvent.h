#ifndef ESL_EVENT_H_INLCUDED
#define	ESL_EVENT_H_INCLUDED


#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>



class EslEvent : public boost::noncopyable
{
public:

  typedef void* EslEventHandle;
  typedef boost::shared_ptr<EslEvent> Ptr;
  
  enum Priority
  {
    ESL_PRIORITY_NORMAL,
    ESL_PRIORITY_LOW,
    ESL_PRIORITY_HIGH
  };

  EslEvent();
  EslEvent(const std::string& type, const std::string& subClass = std::string());
  EslEvent(EslEventHandle eventHandle, bool autoDelete);
  ~EslEvent();
  std::string serialize(const char* prefix = 0);
  bool setPriority(int priority);
  std::string getHeader(const char* header_name);
  std::string getBody();
  std::string getType();
  bool addBody(const char* value);
  bool addHeader(const char* headerName, const char* value);
  bool delHeader(const char* headerName);
  const char* firstHeader();
  const char* nextHeader();
  void setAutoDeleteHandle(bool autoDelete);
  EslEventHandle getHandle() const;

  std::string toString() const;

  std::string createLoggerData(const std::string& prefix);
private:
	EslEventHandle _hp;
	EslEventHandle _event;
	bool _mine;
};


//
// Inlines
//

inline void EslEvent::setAutoDeleteHandle(bool autoDelete)
{
  _mine = autoDelete;
}

inline EslEvent::EslEventHandle EslEvent::getHandle() const
{
  return _event;
}

inline std::string EslEvent::toString() const
{
  return const_cast<EslEvent*>(this)->serialize();
}


#endif	/// ESL_EVENT_H_INCLUDED

