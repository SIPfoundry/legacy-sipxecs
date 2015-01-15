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

  uw.directive('resize', [
    '$window',
    function ($window) {
      return {
        scope: false,
        link: function (scope, element, attrs) {
          var heightDifference  = (attrs.resize === 'full') ? 42 : 75;
          var $                 = angular.element;
          var w                 = $(window);
          var height;

          scope.getWindowDimensions = function () {
            return { h: $window.innerHeight, w: $window.innerWidth };
          };

          scope.$watch(scope.getWindowDimensions, function (newValue, oldValue) {

            setTimeout(resize, 0);

            scope.$on('$viewContentLoaded', function(){
              setTimeout(resize, 0);
            });

            function resize() {
              height = newValue.h - heightDifference;
              $(element).css('max-height', height + 'px');
              return
            }
          }, true);

          w.bind('resize',function(){
            scope.$apply();
          });

          return true;
        }
      }
    }
  ]);

})();
