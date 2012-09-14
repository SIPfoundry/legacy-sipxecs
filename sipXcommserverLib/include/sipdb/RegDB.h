/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#ifndef RegDB_H
#define	RegDB_H

//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>
#include <vector>
#include "sipdb/RegBinding.h"
#include "sipdb/MongoDB.h"

#ifndef GRUU_PREFIX
#define GRUU_PREFIX "~~gr~"
#endif

#ifndef SIP_GRUU_URI_PARAM
#define SIP_GRUU_URI_PARAM "gr"
#endif

class RegDB : public MongoDB::BaseDB
{
public:
	static const std::string NS;
    typedef std::vector<RegBinding> Bindings;

    RegDB(const MongoDB::ConnectionInfo& info) :
		BaseDB(info)
	{
	}
	;

	~RegDB()
	{
	}
	;

    void updateBinding(const RegBinding::Ptr& pBinding);

    void updateBinding(RegBinding& binding);

    void expireOldBindings(
        const std::string& identity,
        const std::string& callId,
        unsigned int cseq,
        unsigned int timeNow);

    void expireAllBindings(
        const std::string& identity,
        const std::string& callId,
        unsigned int cseq,
        unsigned int timeNow);

    void removeAllExpired();

    bool isOutOfSequence(
        const std::string& identity,
        const std::string& callId,
        unsigned int cseq) const;

    bool getUnexpiredContactsUser(
        const std::string& identity,
        int timeNow,
        Bindings& bindings) const;

    bool getUnexpiredContactsUserContaining(
        const std::string& matchIdentity,
        int timeNow,
        Bindings& bindings) const;

    bool getUnexpiredContactsUserInstrument(
        const std::string& identity,
        const std::string& instrument,
        int timeNow,
        Bindings& bindings) const;

    bool getUnexpiredContactsInstrument(
        const std::string& instrument,
        int timeNow,
        Bindings& bindings) const;

    void cleanAndPersist(int currentExpireTime);

    void clearAllBindings();

    void setLocalAddress(const std::string& localAddress) { _localAddress = localAddress; };

    const std::string& getLocalAddress() const { return _localAddress; };

protected:

private:
    std::string _localAddress;
};

#endif	/* RegDB_H */

