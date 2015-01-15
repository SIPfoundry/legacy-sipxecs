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

uw.controller('loginController', [
  '$rootScope',
  '$scope',
  '$location',
  '$cookieStore',
  'restService',
  'uiService',
  'CONFIG',
  function ($rootScope, $scope, $location, $cookieStore, restService, uiService, CONFIG) {
    var authCookie    = CONFIG.authCookie;
    var auth          = $cookieStore.get(authCookie);
    $scope.error      = null;
    $scope.info       = null;
    $scope.username   = null;
    $scope.password   = null;
    $scope.remember   = false;

    if (($location.search()['unitexmppbind']) &&
        ($location.search()['unitexmppdomain']) &&
        ($location.search()['uniterestbase']) &&
        ($location.search()['uniteusername']) &&
        ($location.search()['unitepassword'])) {

      CONFIG.httpBindUrl  = $location.search()['unitexmppbind'];
      CONFIG.domain       = $location.search()['unitexmppdomain'];
      CONFIG.baseRest     = $location.search()['uniterestbase'];

      $scope.info         = 'Connecting...';
      $scope.error        = null;
      $scope.username     = $location.search()['uniteusername'];
      $scope.password     = $location.search()['unitepassword'];
    }


    $scope.submit = function () {
      var user = angular.copy($scope.username);
      var pass = angular.copy($scope.password);

      $scope.info   = 'Connecting...';
      $scope.error  = null;

      if ( (!_.isEmpty(user)) && (!_.isEmpty(pass)) ) {
        restService.updateCredentials(angular.copy($scope.username), angular.copy($scope.password));

        restService.getPhonebook().
          then(function (data) {
            if ($scope.remember) {
              var auth = Base64.encode(user + ':' + pass);
              $cookieStore.put(authCookie, auth);
            }

            restService.connected = true;
            restService.phonebook = data.phonebook;
            restService.phonebook.forEach(function (el, i) {
              restService.phonebook[i].name = (restService.phonebook[i]['contact-information'].imDisplayName) || (restService.phonebook[i]['first-name'] + ' ' + restService.phonebook[i]['last-name']);
            })


            uiService.util.populateContactList();

            // $scope.$apply(function () {
              $location.path('/main');
            // });
          }).
          catch(function (er) {
            $scope.info   = null;
            $scope.error  = 'Invalid credentials';
            console.log(er);
          })
      } else {
        $scope.info   = null;
        $scope.error  = 'Invalid credentials';
      }
    };

    function onCannotConnect() {
      $scope.$apply(function () {
        $scope.error  = 'Cannot connect';
        $scope.info   = null;
      });
    }

    function onInvalidCredentials() {
      $scope.$apply(function () {
        $scope.error  = 'Invalid credentials';
        $scope.info   = null;
      })
    }

    function onConnect() {
      if ($scope.remember) {
        var auth = Base64.encode($scope.username + ':' + $scope.password);
        $cookieStore.put(authCookie, auth);
      }

      // $scope.$apply(function () {
        $location.path('/main');
      // });
    }

    $scope.$on('services.chat.error', onCannotConnect);
    $scope.$on('services.chat.connfail', onCannotConnect);
    $scope.$on('services.chat.authfail', onInvalidCredentials);
    $scope.$on('services.chat.connected', onConnect);

    if (auth) {
      var arr = Base64.decode(auth).split(':');
      $scope.username = arr[0];
      $scope.password = arr[1];

      $scope.info = 'Connecting...';
      $scope.error = null;
      $scope.remember = true;// remember for another 7 days

      restService.updateCredentials(angular.copy($scope.username), angular.copy($scope.password));

      restService.getPhonebook().
        then(function (data) {
          if ($scope.remember) {
            var auth = Base64.encode(angular.copy($scope.username) + ':' + angular.copy($scope.password));
            $cookieStore.put(authCookie, auth);
          }

          restService.connected = true;
          restService.phonebook = data.phonebook;
          restService.phonebook.forEach(function (el, i) {
            restService.phonebook[i].name = (restService.phonebook[i]['contact-information'].imDisplayName) || (restService.phonebook[i]['first-name'] + ' ' + restService.phonebook[i]['last-name']);
          })

          uiService.util.populateContactList();

          $location.path('/main');
        }).
        catch(function () {
          $scope.info   = null;
          $scope.error  = 'Invalid credentials';
        })
    }
  }
]);
})();
