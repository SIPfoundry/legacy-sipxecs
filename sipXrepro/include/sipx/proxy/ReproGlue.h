/**
 *
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
 * 
 */


#ifndef REPROGLUE_H_INCLUDED
#define	REPROGLUE_H_INCLUDED


#include <repro/ReproRunner.hxx>


namespace sipx {
namespace proxy {

class ReproGlue : public repro::ReproRunner
{
public:
  ReproGlue();
  
  bool run(const std::string& path);
  /// Start the repro subsystem using the configuration specified
  /// by path
  ///
};

} } // sipx::proxy

#endif	// REPROGLUE_H_INCLUDED

