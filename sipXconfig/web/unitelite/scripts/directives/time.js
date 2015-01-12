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

(function() {

  'use strict';

  uw.filter('time', [
    '$filter',
    function ($filter) {
      /**
       * returns formatted date based on UTC
       * e.g.
       *   if it's today, show 12:16 PM
       *   if it's yesterday, show Yesterday
       *   if it's anything else show dd/MM/yyyy
       *
       * @param  {String} input       UTC date
       * @return {String}             formatted date
       */
      return function (input) {
        var day       = new Date(input).getUTCDate().toString();
        var mth       = new Date(input).getUTCMonth().toString()
        var today     = new Date().getUTCDate().toString();
        var todayMth  = new Date().getUTCMonth().toString();

        if ((mth === todayMth) && (day === today)) {
          return $filter('date')(input, 'h:mm a');
        } else {
          return $filter('date')(input, 'short');
        }

      }
    }
  ])

})();
