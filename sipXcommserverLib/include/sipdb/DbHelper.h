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

#include "sipdb/EntityDB.h"
#include "sipdb/RegDB.h"

#include <string>
#include <set>

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <boost/format.hpp>


typedef boost::error_info<struct DbHelperTag,std::string> DbHelperTagInfo;

// boost exception
class DbHelperException: public boost::exception, public std::exception
{
public:
  DbHelperException(){}
};


class DbHelper
{
  // The unit test classes needs access to private data.
  friend class DbHelperTest;

public:
  // database type
  enum DbType
  {
    DbTypeRegBinding,
    DbTypeEntityRecord
  };

  DbHelper();
  ~DbHelper();

  /** Delete entries from selected database. It can either delete all entries or
   * only some of them filtered by a certain field id
   *
   * @param pConnectionInfo - Pointer to a ConnectionInfo class
   * @param databaseName - Selected database name
   * @param whereOptVector - A vector that will contain the filter conditions used for listing
   *    or deleting entries
   *
   */
  void deleteDbEntries(const MongoDB::ConnectionInfo* pConnectionInfo,
                       const std::string& databaseName,
                       std::vector<std::string>& whereOptVector);

  /** Print entries from selected database. It can either print all entries or
   *  only some of them filtered by a certain field id
   *
   * @param strm - The stream where to print selected database
   * @param pConnectionInfo - Pointer to a ConnectionInfo class
   * @param databaseName - Selected database name
   * @param whereOptVector - A vector that will contain the filter conditions used for listing
   *    or deleting entries
   * @param dbType - Database type
   * @param multipleLines - If set true the elements will be printed on multiple lines
   *
   * Note: Throws a DbHelperException if no databaseName is selected
   *
   */
  void printDbEntries(std::ostream& strm,
                      const MongoDB::ConnectionInfo* pConnectionInfo,
                      const std::string& databaseName,
                      std::vector<std::string>& whereOptVector,
                      const DbType dbType,
                      bool multipleLines);


protected:
  /** Utility function used to print a set container
   * @param strm - The stream where to print the set container
   * @param setName - The name of the set
   * @param set     - The set container
   * @param multipleLines - If set true the elements will be printed on multiple lines
   */
  void printSetElements(std::ostream& strm,
                        const std::string& setName,
                        const std::set<std::string>& set,
                        bool multipleLines);

  /**
   * Converts an unsigned int that contain the seconds from 1 January 1970 into
   * a human readable format
   * @param seconds - Number of seconds to be converted
   * @return  The local time listed in a human readable format
   */
  boost::posix_time::ptime convertSecondsToLocalTime(unsigned int seconds);

  /**
   * Function used to print a database entry of type RegBinding
   *
   * @param strm - The stream where to print the entry
   * @paramt bson - A reference to a BSON object
   * @param currentNr - The current number of the entry
   * @param multipleLines - If set true the elements will be printed on multiple lines
   */
  void printRegBindingEntry(std::ostream& strm, const mongo::BSONObj& bson, int currentNr, bool multipleLines);

  /**
   * Function used to print a database entry of type EntryRecord
   *
   * @param strm - The stream where to print the entry
   * @paramt bson - A reference to a BSON object
   * @param currentNr - The current number of the entry
   * @param multipleLines - If set true the elements will be printed on multiple lines
   */
  void printEntityRecordEntry(std::ostream& strm, const mongo::BSONObj& bson, int currentNr, bool multipleLines);

  /** Utility function used to print entity record aliases vector
   * @param strm - The stream where to print the entity record aliases
   * @param aliasesName      - The name for the aliases
   * @param aliases          - The aliases vector
   * @param multipleLines - If set true the elements will be printed on multiple lines
   */
  void printEntityRecordAliases(std::ostream& strm,
                                const std::string& aliasesName,
                                const std::vector<EntityRecord::Alias>& aliases,
                                bool multipleLines);

  /** Utility function used to print entity record static user location vector
   * @param strm - The stream where to print the entity record static user locations
   * @param staticUserLocationsName      - The name for static user locations
   * @param staticUserLocations          - The static user locations vector
   * @param multipleLines - If set true the elements will be printed on multiple lines
   */
  void printEntityRecordStaticUserLocations(std::ostream& strm,
                                            const std::string& staticUserLocationsName,
                                            const std::vector<EntityRecord::StaticUserLoc>& staticUserLocations,
                                            bool multipleLines);

  /**
   * This function transform a logical operator string into a mongo::Labeler::Label
   * structure
   *
   * For example:
   *   >    mongo::GT
   *   <    mongo::LT
   *   <=   mongo::LTE
   *   >=   mongo::GTE
   *   !=   mongo::NE
   *
   * @param logicalOperator - Logical operator as a string
   * @param label - Corresponding mongo::Labeler::Label structure
   * @return true - If the logical operator was successfully transformed
   * into associated mongo::Labeler::Label structure or false otherwise.
   *
   * Note: Throws a DbHelperException if the logicalOperator is different from any
   * of the following: ">", "<", "<=", ">=", "!=", "="
   *
   */
  bool getLogicalOperator(const std::string& logicalOperator,
                          mongo::Labeler::Label& label);

  /**
   * Utility function used to create a query for selecting all entries or only
   * some of them filtered by a certain field. It is used for both listing or
   * deleting entries.
   *
   * @param query - A reference to a BSON object
   * @param pConn - A reference to a ScopedDbConnectionPtr class
   * @param whereOptVector - A vector that will contain the filter conditions used for listing
   * or deleting entries
   * @param databaseName - Selected database name
   */
  void createQuery(mongo::BSONObj& query,
                   MongoDB::ScopedDbConnectionPtr& pConn,
                   std::vector<std::string>& whereOptVector,
                   const std::string& databaseName);


  /**
   * Utility function used to append to query the required value type. This type
   * is computed by checking the key type.
   *
   * @param pConn - A reference to a ScopedDbConnectionPtr class
   * @param queryObjBuilder - A reference to a BSONObjBuilder class
   * @param databaseName - Selected database name
   * @param string - The whole condition expression: Example: expirationTime < $now
   *       cseq < 812
   *
   * Note: Throws a DbHelperException if the right operand from the evaluated condition expression
   * can't be transformed in the required type
   *
   */
  void appendRequiredType(MongoDB::ScopedDbConnectionPtr& pConn,
                          mongo::BSONObjBuilder& queryObjBuilder,
                          const std::string& databaseName,
                          const std::string& string);

  /**
   * This utility function splits the input string in operands and operators
   *
   * @param string The string to be splitted
   * @param leftOperand - The computed left operand
   * @param logicalOperator - The computed logical operator
   * @param rightOperand - The computed right operand
   */
  void extractOperands(const std::string& string,
                       std::string& leftOperand,
                       std::string& logicalOperator,
                       std::string& rightOperand);

  /**
   * Replace the input macro with the corresponding value
   * @param macro - The input macro that will be replaced with the corresponding
   * value
   *
   * Note: Throws a DbHelperException if the macro is unknown
   *
   */
  void expandMacro(std::string& macro);

  /**
   * Return the number of seconds since epoch in string format
   */
  std::string getTimeNowMacro();

  /**
   * Template function used to print a cell field from an entry.
   * Can be configured to print on a single line or on multiple lines
   *
   * @param strm - The stream where to print the cell
   * @param cellName - The name of the cell
   * @param cellValue - The value of the cell
   * @param currentNr - The current number of the entry
   */
  template <class T>
  void printCell(std::ostream& strm, const std::string& cellName, T cellValue, int currentNr, bool multipleLines);


private:
  typedef void (DbHelper::*FnPrintEntry_t)(std::ostream& strm, const mongo::BSONObj& bson, int currentNr, bool multipleLines);
  FnPrintEntry_t _pFnPrintEntry;    // Callback function used to print an entry from selected database
};
