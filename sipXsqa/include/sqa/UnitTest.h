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

#ifndef UNITTEST_H
#define	UNITTEST_H

#include <string>
#include <vector>
#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/any.hpp>

//
// DEFINE_UNIT_TEST - Define a Test Group.  Must be called prior to DEFINE_TEST
// DEFINE_TEST - Define a new unit test belonging to a defined group
// DEFINE_RESOURCE - Register a resource that is accessible to unit tests in the same group
// GET_RESOURCE - Get the value of the resource that was previously created by DEFINE_RESOURCE
// ASSERT_COND(cond) - Assert if the logical condition is false
// ASSERT_STR_EQ(var1, var2) - Assert that two strings are  equal
// ASSERT_STR_CASELESS_EQ(var1, var2) - Assert that two strings are equal but ignoring case comparison
// ASSERT_STR_NEQ(var1, var2) - Asserts that two strings are not eual
// ASSERT_EQ(var1, var2) - Asserts that the two values are equal
// ASSERT_NEQ(var1, var2) - Asserts that the the values are not equal
// ASSERT_LT(var1, var2) - Asserts that the value var1 is less than value of var2
// ASSERT_GT(var1, var2)  Asserts that the value var1 is greater than value of var2
//

#define DEFINE_UNIT_TEST(class_name) \
static int class_name##error_count = 0; \
struct class_name \
{ \
  typedef boost::function<void(bool&, std::string&)> Func; \
  typedef std::map<std::string, Func> FuncList; \
  typedef std::map<std::string, boost::any> UserDataList; \
  class_name() \
  { \
  } \
  FuncList _functions; \
  void register_func(std::string name, Func function) \
  { \
    _functions[name] = function; \
  } \
  void run_func(std::string name) \
  { \
    Func func = _functions[name]; \
    if (!func) return; \
    std::string error; \
    bool hasError = false; \
    func(hasError, error); \
    if (!hasError) \
    { \
      std::cout << #class_name << "::" << name << " ...\tOk" << std::endl; \
    } \
    else \
    { \
      std::cerr << #class_name << "::" << name << " ...\tError: " << error << std::endl;  \
      class_name##error_count++; \
    } \
  } \
  UserDataList _user_data; \
  void registerResource(const std::string& name, const boost::any& resource) \
  { \
    _user_data[name] = resource; \
  } \
  template <typename T> \
  T getResource(const std::string& name) \
  { \
    return boost::any_cast<T>(_user_data[name]); \
  } \
}; \
static class_name* class_name##instance = 0;

#define TEST_RESULT(class_name) class_name##error_count == 0;

#define DEFINE_TEST(class_name, func_name) static void class_name##func_name(bool& ___hasError_, std::string& ___error_)

#define VERIFY_TEST(class_name, func_name) \
if (!class_name##instance) class_name##instance = new class_name; \
class_name##instance->register_func(#func_name, boost::bind(class_name##func_name, _1, _2)); \
class_name##instance->run_func(#func_name);

#define DEFINE_RESOURCE(class_name, name, resource) \
if (!class_name##instance) class_name##instance = new class_name; \
  class_name##instance->registerResource(name, resource)

#define GET_RESOURCE(class_name, type, name) \
  class_name##instance->getResource<type>(name)

#define ASSERT_STR_EQ(var1, var2) \
{ \
std::string ___var1_(var1); \
std::string ___var2_(var2); \
if (___var1_ != ___var2_) \
{ \
  std::ostringstream ___strm_; \
  ___strm_ << "Expecting \"" << var1 << "\" for " << #var1 << " but we got \"" << var2 << "\""; \
  ___error_ = ___strm_.str(); \
  ___hasError_ = true; \
  return; \
} \
}

#define ASSERT_STR_NEQ(var1, var2) \
{ \
std::string ___var1_(var1); \
std::string ___var2_(var2); \
if (___var1_ == ___var2_) \
{ \
  std::ostringstream ___strm_; \
  ___strm_ << "Expecting \"" << #var1 << "\" IS NOT EQUAL TO " << #var2; \
  ___error_ = ___strm_.str(); \
  ___hasError_ = true; \
  return; \
} \
}

#define ASSERT_STR_CASELESS_EQ(var1, var2) \
{ \
std::string ___var1_(var1); \
std::string ___var2_(var2); \
boost::to_lower(___var1_); \
boost::to_lower(___var2_); \
if (___var1_ != ___var2_) \
{ \
  std::ostringstream ___strm_; \
  ___strm_ << "Expecting \"" << var1 << "\" for " << #var1 << " but we got \"" << var2 << "\""; \
  ___error_ = ___strm_.str(); \
  ___hasError_ = true; \
  return; \
} \
}

#define ASSERT_COND(eval) \
{ \
if (!(eval)) \
{ \
  std::ostringstream ___strm_; \
  ___strm_ << "Expecting condition to be TRUE but we got FALSE - " << #eval; \
  ___error_ = ___strm_.str(); \
  ___hasError_ = true; \
  return; \
} \
}

#define ASSERT_EQ(var1, var2) \
{ \
if (var1 != var2) \
{ \
  std::ostringstream ___strm_; \
  ___strm_ << "Expecting " << #var1 << " and " << #var2 \
  << " to be EQUAL but values are `" << var1 << "` and `" << var2 << "` respectively"; \
  ___error_ = ___strm_.str(); \
  ___hasError_ = true; \
  return; \
} \
}

#define ASSERT_NEQ(var1, var2) \
{ \
if (var1 == var2) \
{ \
  std::ostringstream ___strm_; \
  ___strm_ << "Expecting " << #var1 << " and " << #var2 \
  << " to be NOT EQUAL but values are `" << var1 << "` and `" << var2 << "` respectively"; \
  ___error_ = ___strm_.str(); \
  ___hasError_ = true; \
  return; \
} \
}

#define ASSERT_LT(var1, var2) \
{ \
if (var1 >= var2) \
{ \
  std::ostringstream ___strm_; \
  ___strm_ << "Expecting " << #var1 << " is LESS THAN " << #var2 \
  << "  but values are `" << var1 << "` and `" << var2 << "` respectively"; \
  ___error_ = ___strm_.str(); \
  ___hasError_ = true; \
  return; \
} \
}

#define ASSERT_GT(var1, var2) \
{ \
if (var1 <= var2) \
{ \
  std::ostringstream ___strm_; \
  ___strm_ << "Expecting " << #var1 << " is GREATER THAN " << #var2 \
  << "  but values are `" << var1 << "` and `" << var2 << "` respectively"; \
  ___error_ = ___strm_.str(); \
  ___hasError_ = true; \
  return; \
} \
}

#define END_UNIT_TEST(class_name)  \
  delete class_name##instance; \
  class_name##instance = 0;




#endif	/* UNITTEST_H */

