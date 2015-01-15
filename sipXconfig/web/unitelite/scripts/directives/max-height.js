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

(function(){
'use strict';

uw.directive('maxHeight', [
  'uiService',
  function (uiService) {
    return {
      compile: function () {

        return function link (scope, elem) {
          scope.$watch(
            function() {
              return uiService.groupChat.modal;
            },
            function(newValue, oldValue) {
              if (newValue) {
                elem[0].scrollTop = 0;
                angular.element(elem).css({'height': '100%', 'overflow': 'hidden'})
              } else {
                angular.element(elem).css({'height': 'auto', 'overflow': 'auto'})
              }
            }
          );
        }

      }
    }
  }
]);
})();
