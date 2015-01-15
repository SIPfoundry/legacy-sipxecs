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

  uw.directive('tree', [
    '$compile',
    '_',
    function ($compile, _) {
      return {
        restrict: 'A',
        scope: {
          treeKey : '=',
          treeVal : '=',
          treeParent : '='
        },
        compile: function (el, attrs) {

          var template        = '<label for="{{ treeKey }}" class="col-sm-4 control-label">{{ treeKey | translate }}</label>';
          var spanParent      = template +
          '<div class="col-sm-8">' +
            '<span data-ng-bind-html="treeVal | linky"></span>' +
          '</div><hr>';
          var spanChildren    = template +
          '<div class="form-group-child col-sm-12 clearfix"' +
            'data-ng-repeat="(k, v) in treeVal track by $index">' +
            '<div data-tree data-tree-key="k" data-tree-val="v"></div>' +
          '</div>';
          var inputParent     = template +
          '<div class="col-sm-8">' +
            '<input type="text" class="form-control" data-ng-model="treeParent[treeKey]">' +
          '</div><hr>';
          var selectParent    = template +
          '<div class="col-sm-8">' +
            '<select class="form-control" data-ng-model="treeParent[treeKey]" data-ng-change="profileForm.$setDirty();">' +
              '<option data-ng-selected="treeParent[treeKey] === true">True</option>' +
              '<option data-ng-selected="treeParent[treeKey] === false">False</option>' +
            '</select>' +
          '</div><hr>';
          var inputChildren   = template + '<div class="form-group-child col-sm-12 clearfix"' +
            'data-ng-repeat="(k, v) in treeVal track by $index">' +
            '<div data-tree data-tree-key="k" data-tree-val="v" data-tree-input data-tree-parent="treeParent[treeKey]"></div>' +
          '</div>';

          var children;
          var parent;

          if (attrs.treeInput === '') {
            parent    = inputParent;
            children  = inputChildren;
          } else {
            parent    = spanParent;
            children  = spanChildren;
          }

          return function postLink (scope, element, attrs) {

            switch (scope.treeKey) {
              case 'avatar':
              case 'timestamp':
              case 'imDisplayName':
              case 'enabled':
              case 'ldapManaged':
              case 'branchName':
              case 'branchAddress':
              case 'officeAddress':
              case 'useBranchAddress':
              case 'salutation':
              case 'employeeId':
              case 'emailAddressAliasesSet':
                return;
              default:
                break;
            }

            if (angular.isObject(scope.treeVal)) {
              if (!_.isEmpty(scope.treeVal)) {
                element.append(children);
                $compile(element.contents())(scope);
                return;
              } else {
                return;
              }
            } else {
              if (scope.treeVal === false || scope.treeVal === true) {
                element.append(selectParent);
              } else {
                element.append(parent);
              }
              $compile(element.contents())(scope);
              return;
            }

          };
        }
      }
    }
  ]);

})();
