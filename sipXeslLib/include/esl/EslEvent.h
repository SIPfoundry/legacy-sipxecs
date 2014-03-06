/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

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

