#include <string>
#include <iostream>
#include <fstream>
#include <boost/iostreams/filtering_stream.hpp>
#include <mongo/client/dbclient.h>
#include <mongo/client/connpool.h>
#include "sipdb/MongoDB.h"

using namespace MongoDB;
using namespace std;

const mongo::ConnectionString ConnectionInfo::connectionStringFromFile(const string& configFile)
{
	const char* configFileStr = (configFile.size() == 0 ? SIPX_CONFDIR "/sipxmongo-config" : configFile.c_str());
    ifstream file(configFileStr, ios_base::in);
    boost::iostreams::filtering_istream in;
    in.push(file);
    string connectionString;
    if (!getline(in, connectionString))
    {
        throw (string("Invalid contents: ")  + configFileStr);
    }
    file.close();

    return ConnectionInfo::connectionString(connectionString);
}


const mongo::ConnectionString ConnectionInfo::connectionString(const string& connectionString)
{
	string errmsg;
	mongo::ConnectionString cs = mongo::ConnectionString::parse(connectionString, errmsg);
	if (!cs.isValid()) {
		throw errmsg;
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

