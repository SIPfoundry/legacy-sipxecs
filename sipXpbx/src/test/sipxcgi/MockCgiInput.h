// 
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
                                                                                                              
#ifndef _MockCgiInput_h_
#define _MockCgiInput_h_

#include <string>
#include <cgicc/CgiInput.h>

class OsConfigDb;
                                                                                                              
/**
 * Emulate a cgi environment from text input files.
 */
class MockCgiInput : public cgicc::CgiInput
{
 private:

    /** Stores strings mocking the env. vars., including query string of GET */
    OsConfigDb *m_env;

    /** Non-null when post of uploaded file */
    OsFile *m_read;

 public:
    MockCgiInput(char *filename);

    ~MockCgiInput();

    /** Mock the POST body or upload from file from a
        filename. Relative to working directory only */
    void setReadFile(const char *filename);

    size_t read(char *data, size_t length);

    std::string getenv(const char *envName);
};

#endif // _MockCgiInput_h_

