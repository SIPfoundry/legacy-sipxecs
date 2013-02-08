#include <string>
#include <iostream>
#include <fstream>
//#include <boost/program_options.hpp>
#include <boost/config.hpp>
#include <boost/program_options/detail/config_file.hpp>
#include <boost/program_options/parsers.hpp>
#include <mongo/client/dbclient.h>
#include <mongo/client/connpool.h>
#include "sipdb/MongoDB.h"

using namespace MongoDB;
using namespace std;

namespace pod = boost::program_options::detail;

bool ConnectionInfo::testConnection(const mongo::ConnectionString &connectionString, const string& errmsg)
{
    bool ret = false;

    try
    {
        mongo::ScopedDbConnection conn(connectionString);
        ret = conn.ok();
        conn.done();
    }
    catch( mongo::DBException& e )
    {
        ret = false;
        errmsg = e.what();
    }

    return ret;
}


const mongo::ConnectionString ConnectionInfo::connectionStringFromFile(const string& configFile)
{
	const char* configFileStr = (configFile.size() == 0 ? SIPX_CONFDIR "/mongo-client.ini" : configFile.c_str());
    ifstream file(configFileStr);
    if (!file)
    {
        BOOST_THROW_EXCEPTION(ConfigError() <<  errmsg_info(std::string("Missing file ")  + configFileStr));
    }
    set<string> options;
    options.insert("*");
    string connectionString;
    for (boost::program_options::detail::config_file_iterator i(file, options), e; i != e; ++i) {
        if (i->string_key == "connectionString") {
            connectionString = i->value[0];
            break;
        }
    }
    file.close();
    if (connectionString.size() == 0)
    {
        BOOST_THROW_EXCEPTION(ConfigError() << errmsg_info(std::string("Invalid contents, missing parameter 'connectionString' in file ")  + configFileStr));
    }

    return ConnectionInfo::connectionString(connectionString);
}


const mongo::ConnectionString ConnectionInfo::connectionString(const string& connectionString)
{
	string errmsg;
	mongo::ConnectionString cs = mongo::ConnectionString::parse(connectionString, errmsg);
	if (!cs.isValid()) {
	    BOOST_THROW_EXCEPTION(ConfigError() << errmsg_info(errmsg));
	}

	return cs;
}

void BaseDB::forEach(mongo::BSONObj& query, boost::function<void(mongo::BSONObj)> doSomething)
{
    mongo::ScopedDbConnection conn(_info.getConnectionString());
    auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
    if (pCursor.get() && pCursor->more())
    {
        while (pCursor->more())
        {
            doSomething(pCursor->next());
        }
    }

    conn.done();
}

