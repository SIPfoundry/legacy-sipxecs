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

(function(window, document, undefined) {
'use strict';

var uniteWeb = angular.module('uw', ['ngAnimate', 'ngSanitize', 'ngCookies', 'ngRoute', 'config', 'emoji', 'underscore', 'LocalStorageModule', 'notify', 'xml', 'dragAndDrop']);

uniteWeb.config([
  '$routeProvider',
  '$httpProvider',
  '$compileProvider',
  '$provide',
  function ($routeProvider, $httpProvider, $compileProvider, $provide) {
    $routeProvider.
      when('/', {
        templateUrl: 'views/main.html',
        controller: [
          '$scope',
          '$location',
          'restService',
          'uiService',
          function ($scope, $location, restService, uiService) {
            $scope.received = true;

            restService.getLogindetails().
              then(function (data) {
                restService.updateCredentials(data['login-details']['userName']);
                restService.connected = true;
              }, function (err) {
                console.log(err);
              }).
              then(function () {
                return restService.getPhonebook();
              }).
              then(function (data) {
                restService.phonebook = data.phonebook;
                restService.phonebook.forEach(function (el, i) {
                  restService.phonebook[i].name = (restService.phonebook[i]['contact-information'].imDisplayName) || (restService.phonebook[i]['first-name'] + ' ' + restService.phonebook[i]['last-name']);
                })
                uiService.util.populateContactList()
              }).
              catch(function (err) {
                console.log(err);
              })
          }
        ]
    });

    $compileProvider.aHrefSanitizationWhitelist(/^\s*(http|https|blob|mailto):/);
    $compileProvider.imgSrcSanitizationWhitelist(/^\s*(https?|ftp|file|blob):|data:image\//);

    // https://github.com/angular/angular.js/issues/1404
    $provide.decorator('ngModelDirective', function($delegate) {
      var ngModel = $delegate[0], controller = ngModel.controller;
      ngModel.controller = ['$scope', '$element', '$attrs', '$injector', function(scope, element, attrs, $injector) {
        var $interpolate = $injector.get('$interpolate');
        attrs.$set('name', $interpolate(attrs.name || '')(scope));
        $injector.invoke(controller, this, {
          '$scope': scope,
          '$element': element,
          '$attrs': attrs
        });
      }];
      return $delegate;
    });
    // https://github.com/angular/angular.js/issues/1404
    $provide.decorator('formDirective', function($delegate) {
      var form = $delegate[0], controller = form.controller;
      form.controller = ['$scope', '$element', '$attrs', '$injector', function(scope, element, attrs, $injector) {
        var $interpolate = $injector.get('$interpolate');
        attrs.$set('name', $interpolate(attrs.name || attrs.ngForm || '')(scope));
        $injector.invoke(controller, this, {
          '$scope': scope,
          '$element': element,
          '$attrs': attrs
        });
      }];
      return $delegate;
    });
  }
]);

window.uw = uniteWeb;

// expose uw as an AMD module
if (typeof define === 'function' && define.amd) {
    define(uw);
}

})(window, document);
