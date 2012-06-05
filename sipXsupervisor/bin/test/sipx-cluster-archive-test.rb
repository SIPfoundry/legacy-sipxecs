#!/usr/bin/ruby

# Copyright (c) 2012 eZuce, Inc. All rights reserved.
# Contributed to SIPfoundry under a Contributor Agreement
#
# This software is free software; you can redistribute it and/or modify it under
# the terms of the Affero General Public License (AGPL) as published by the
# Free Software Foundation; either version 3 of the License, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
# details.

require "test/unit"
load "../sipx-cluster-archive.in"

$srcdir = File.dirname(__FILE__)

class ClusterArchiveTest < Test::Unit::TestCase

  def test_purgeable
    c = FileManagerBase.new
    $input = ['0', '1', '2', '3']
    expected = ['0', '1']
    def c.list
      $input
    end
    actual = []
    c.purgable(2) {|b|
      actual.push(b)
    }
    assert_equal(expected, actual)
  end

  def test_select_backups
    c = FileManagerBase.new
    input = ['.', '..', 'temp', '123', '000000000000', '111111111111', '9999999999999999999']
    expected = ['111111111111', '000000000000']
    actual = c.select_backups(input)
    assert_equal(expected, actual)
  end

end
