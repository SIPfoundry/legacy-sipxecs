
#ifndef RegDB_H
#define	RegDB_H


#include "sipdb/RegBinding.h"
#include "boost/noncopyable.hpp"
#include <boost/thread/mutex.hpp>
#include <vector>
#include <map>

#ifndef GRUU_PREFIX
#define GRUU_PREFIX "~~gr~"
#endif

#ifndef SIP_GRUU_URI_PARAM
#define SIP_GRUU_URI_PARAM "gr"
#endif

class RegDB : MongoDB::DBInterface
{
public:
    typedef boost::recursive_mutex Mutex;
    typedef boost::lock_guard<Mutex> mutex_lock;
    typedef std::vector<RegBinding> Bindings;
    RegDB(
        MongoDB& db,
        const std::string& ns = "imdb.registrar");

    ~RegDB();

    void updateBinding(const RegBinding::Ptr& pBinding);

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

    bool isOutOfSequence(
        const std::string& identity,
        const std::string& callId,
        unsigned int cseq) const;


    bool getUnexpiredContactsUser(
        const std::string& identity,
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

     bool getAllOldBindings(int timeNow, Bindings& binding);

    bool clean(int currentExpireTime);

private:
    Mutex _mutex;
};


//
// Inlines
//


#endif	/* RegDB_H */

