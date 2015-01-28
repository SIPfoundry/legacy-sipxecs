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

  uw.service('uiService', [
    '$rootScope',
    '$interval',
    '$sce',
    'sharedFactory',
    'restService',
    '_',
    'CONFIG',
    function ($rootScope, $interval, $sce, sharedFactory, restService, _, CONFIG) {

      var activityList = {
        main: null
      };

      var rosterList = {
        main:     null,
        noGroups: null
      };

      var search = {
        t: ''
      };

      var timers = {
        all: {}
      };

      var groupChat = {
        modal: false
      };

      var ui = {

        root: {
          templates:    sharedFactory.settings.mainMenu,
          template:     sharedFactory.settings.mainMenu[1],
          oldTemplate:  sharedFactory.settings.mainMenu[1]
        },

        groupChat: {

          showModal : function () {
            groupChat.modal = !groupChat.modal;

            return;
          },

          hideModal : function () {
            groupChat.modal           = !groupChat.modal;
            groupChat.serverRooms     = null;

            return;
          }

        },

        activityList: {

          selectConversation : function () {

          },

          removeConversation : function (entry) {

            var spliceFromActiList = function () {
              _.find(activityList.main, function (obj) {
                if (obj.jid === entry.jid) {
                  activityList.main.splice(_.indexOf(activityList.main, obj), 1);
                  sharedFactory.saveConversationList(angular.copy(activityList.main));
                  return true;
                }
              });
            };

            spliceFromActiList();

            sharedFactory.delContactMessageList(entry.jid);

            return;

          }

        }

      };

      var secondary = {

        template : {
          main: ui.root.templates[ui.root.templates.length - 1],
        },

        logout : {
          init : function () {
            window.location.assign('/sipxconfig/rest/my/logindetailsunite/logout');
          }
        },

        chat: {

          showDefault: function () {
            util.changeView(ui.root.templates[9]);
            secondary.conference.participants   = [];
            secondary.voicemail.messages        = [];
            secondary.voicemail.folder          = {};
            secondary.conference.active         = false;
            secondary.conference.timers.cancelAll(true);
            $rootScope.leftSideView = '';
            return;
          }

        },

        voicemail: {

          messages: [],

          backup: [],

          folder: {},

          tooltip: {
            refresh: {
              title: 'Refresh',
              checked: false
            },
            download: {
              title: 'Download',
              checked: false
            },
            trash: {
              title: 'Trash',
              checked: false
            }
          },

          folders: [
            { name: 'inbox' },
            { name: 'saved' },
            { name: 'deleted' },
            { name: 'conference' }
          ],

          moveVoicemail: {},

          init: function () {
            var temp;
            secondary.voicemail.messages  = null;
            secondary.voicemail.messages  = [];
            secondary.voicemail.backup    = [];
            if (!_.isEmpty(secondary.voicemail.folder)) {
              if (secondary.voicemail.folder.name === 'all') {
                temp = 'messages';
              } else {
                temp = angular.copy(secondary.voicemail.folder.name);
              }
              restService.getMessages(temp).then(function (messages) {
                var msgs = {};
                _.map(messages, function (message) {
                  msgs[message.id] = message;
                })
                secondary.voicemail.messages  = angular.copy(msgs);
                secondary.voicemail.backup    = angular.copy(secondary.voicemail.messages)
              })
            } else {
              restService.getMessages('inbox').then(function (messages) {
                var msgs = {};
                _.map(messages, function (message) {
                  msgs[message.id] = message;
                })
                secondary.voicemail.messages  = angular.copy(msgs);
                secondary.voicemail.folder    = secondary.voicemail.folders[0];
                secondary.voicemail.backup    = angular.copy(secondary.voicemail.messages);
              }, function (er) {
                console.log(er);
              });
            }
          },

          changeFolder: function () {
            var folder = secondary.voicemail.folder.name;

            if (folder === 'all') {
              folder = 'messages';
            }

            secondary.voicemail.messages  = null;

            restService.getMessages(folder).then(function (messages) {
              var msgs = {};
              _.map(messages, function (message) {
                msgs[message.id] = message;
              })
              secondary.voicemail.messages  = angular.copy(msgs);
              secondary.voicemail.backup    = angular.copy(secondary.voicemail.messages);
            }, function (er) {
              console.log(er);
            });
          },

          getMessage: function (id, index) {
            var bId = angular.copy(id);
            var bIndex = angular.copy(index);
            secondary.voicemail.messages[bId]['loading'] = true;
            // secondary.voicemail.messages[index].href = CONFIG.baseRest + '/my/redirect/media/' + restService.user + '/inbox/' + id
            restService.getMessage(bId)
              .then(function (url) {
                // secondary.voicemail.messages[index].href   = url;
                secondary.voicemail.messages[bId].href = $sce.trustAsResourceUrl(CONFIG.baseRest +'/my/redirect/media/' + restService.cred.user + '/inbox/' + id);
                delete secondary.voicemail.messages[bId].loading;
                return restService.heardMessage(bId);
              })
              .then(function () {
                return restService.getHeardMessage(bId);
              })
              .then(function () {
                secondary.voicemail.messages[bId].heard  = true;
                delete secondary.voicemail.messages[bId].loading;
              })
          },

          delMessage: function (id, index) {
            var bId = angular.copy(id);
            var bIndex = angular.copy(index);
            secondary.voicemail.messages[bId]['loading'] = true;

            if (secondary.voicemail.folder.name === 'deleted') {
              restService.delMessage(bId).then(function () {
                delete secondary.voicemail.messages[bId];
              }, function (er) {
                console.log(er);
              });
            } else {
              restService.putMoveMsg(id, 'deleted').then(function () {
                delete secondary.voicemail.messages[bId];
              }, function (er) {
                console.log(er);
              });
            }

          },

          moveMessage: function (id, index) {
            var bId = angular.copy(id);
            var bIndex = angular.copy(index);
            var folder = angular.copy(secondary.voicemail.moveVoicemail[bId]);
            secondary.voicemail.messages[bId].loading = true;
            if (folder.name === 'deleted') {
              secondary.voicemail.messages[bId].href = '';
            }
            restService.putMoveMsg(id, folder.name).then(function () {
              secondary.voicemail.moveVoicemail = {};
              secondary.voicemail.messages[bId].loading = false;
              delete secondary.voicemail.messages[bId];
            }).catch(function (err) {
              console.log(err);
              secondary.voicemail.messages[bId].loading = false;
            })
          },

          changeSubject: function (index) {
            var subject   = angular.copy(secondary.voicemail.messages[index].subject);
            var id        = angular.copy(secondary.voicemail.messages[index].id);
            secondary.voicemail.messages[index].loading = true;
            restService.putSubjectMessage(id, subject).then(function () {
              secondary.voicemail.messages[index].loading = false;
            }).catch(function (err) {
              console.log(err);
              secondary.voicemail.messages[index].loading = false;
            })
          },

          cancelChangeSubject: function (index) {
            secondary.voicemail.messages[index] = angular.copy(secondary.voicemail.backup[index]);
            return;
          },

          isEmpty: function () {
            for (var key in secondary.voicemail.messages) {
              if(secondary.voicemail.messages.hasOwnProperty(key)) {
                return false
              }
            }
            return true
          },

          mp3Ie: function (audioFormat) {
            if (audioFormat === 'mp3') {
              return true;
            }

            if (window.navigator.userAgent.indexOf('MSIE ') !== -1 || window.navigator.userAgent.indexOf('Trident/') !== -1) {
              return false;
            }

            return true;
          }

        },

        conference: {

          rooms:        null,

          select:       null,

          active:       null,

          refresh:      null,

          status:       null,

          pin:          null,

          err:          null,

          btnDisabled:  false,

          participants: [],

          tooltip: {
            title: 'Refresh',
            checked: true
          },

          init: function () {
            if (!secondary.conference.err) {
              secondary.conference.rooms    = null;
              secondary.conference.err      = null;
            }
            secondary.conference.status   = 'Searching for conference rooms...';
            secondary.conference.refresh  = null;
            restService.getConfList()
              .then(function (data) {
                if (_.isEmpty(data.conferences)) {
                  secondary.conference.status   = 'No conference rooms found. ';
                  secondary.conference.err      = true;
                  return;
                } else {
                  secondary.conference.err      = null;
                }
                secondary.conference.rooms    = data.conferences;
                secondary.conference.select   = secondary.conference.rooms[0];
                secondary.conference.status   = 'Waiting for an active conference room...';
                secondary.conference.timers.start('confActive');
              })
              .catch(function (er) {
                secondary.conference.status   = 'Error!';
                console.log(er);
              });
          },

          changeRoom: function () {
            console.log(secondary.conference.select);
          },

          moveHere: function (item) {
            var entry     = angular.copy(item);
            var uniq      = _.find(secondary.conference.participants,
              function (part) {
                if (part.name === entry.name) {
                  return true;
                }
            });

            if (!_.isUndefined(uniq)) {
              return;
            }

            var confName  = angular.copy(secondary.conference.select.name);
            var intern;

            if (secondary.conference.active) {
              intern = entry.number;
              secondary.conference.participants.push(entry);
              restService.putConfInvite(confName, intern)
                .then(function (data) {
                  console.log(data);
                }, function (er) {
                  console.log(er);
                });
            } else {
              secondary.conference.participants.push(entry);
              return;
            }
          },

          remove: function (index) {
            secondary.conference.participants.splice(angular.copy(index), 1);

            return;
          },

          call: function () {
            var confName = angular.copy(secondary.conference.select.name);

            // if ((secondary.conference.pin !== null) && (secondary.conference.pin.length > 0)) {
            //   restService.getConfPin(confName, angular.copy(secondary.conference.pin))
            //     .then(function (data) {
            //       console.log(data);
            //     }, function (er) {
            //       console.log(er);
            //     })
            // };

            if (!secondary.conference.active) {
              secondary.conference.status  = 'Calling...';

              _.each(secondary.conference.participants, function (item) {
                restService.putConfInvite(confName, item.number || item.name)
                  .then(function () {

                  }, function (er) {
                    console.log(er);
                  });
              });
              secondary.conference.status       = 'Call in progress...';
              secondary.conference.btnDisabled  = true;
              secondary.conference.timers.start('confActive');
            } else {
              secondary.conference.status       = 'Kicking participants & hanging up...';
              secondary.conference.btnDisabled  = true;
              restService.getConfKick(confName, 'all')
                .then(function () {
                  secondary.conference.status       = 'Kicking participants & hanging up...';
                  secondary.conference.active       = false;
                  secondary.conference.btnDisabled  = false;
                  _.each(secondary.conference.participants, function (part) {
                    if (part.conf) {
                      part.conf.active = null;
                    }
                  });
                }, function (er) {
                  console.log(er);
                });
            }

            return;
          },

          mute: function (item) {
            var confName      = angular.copy(secondary.conference.select.name);
            var entry         = angular.copy(item);
            var getConfMute   = function (conf, id) {
              restService.getConfMute(conf, id)
                .then(function (data) {
                  console.log(data);
                }, function (er) {
                  console.log(er);
                });
            };
            var getConfUnmute = function (conf, id) {
              restService.getConfUnmute(conf, id)
                .then(function (data) {
                  console.log(data);
                }, function (er) {
                  console.log(er);
                });
            };

            if (entry.conf.canSpeak) {
              getConfMute(confName, entry.conf.id);
            } else {
              getConfUnmute(confName, entry.conf.id);
            }

          },

          deaf: function (item) {
            var confName      = angular.copy(secondary.conference.select.name);
            var entry         = angular.copy(item);
            var getConfDeaf   = function (conf, id) {
              restService.getConfDeaf(conf, id)
                .then(function (data) {
                  console.log(data);
                }, function (er) {
                  console.log(er);
                });
            };
            var getConfUndeaf = function (conf, id) {
              restService.getConfUndeaf(conf, id)
                .then(function (data) {
                  console.log(data);
                }, function (er) {
                  console.log(er);
                });
            };

            if (entry.conf.canHear) {
              getConfDeaf(confName, entry.conf.id);
            } else {
              getConfUndeaf(confName, entry.conf.id);
            }

          },

          keyPress: function (e) {
            if ( e.keyCode === 46 || e.keyCode === 8 ) {
            } else {
              if ((e.keyCode < 48 || e.keyCode > 57 ) && (secondary.conference.pin.length > 6)) {
                e.preventDefault();
              }
            }
          },

          kick: function (item) {
            var confName  = angular.copy(secondary.conference.select.name);
            var entry     = angular.copy(item);

            restService.getConfKick(confName, entry.conf.id)
              .then(function () {
                item.conf.active = null;
              }, function (er) {
                console.log(er);
              });
          },

          timers: {

            all: {},

            maxTimeouts: 20,
            currentTimeouts: 0,

            start: function (type) {
              var conf;
              var exists;
              var found;
              var interval;
              var backup = {};

              switch (type) {

                case 'confActive':
                  secondary.conference.refresh = null;
                  if (!timers.all.confActive) {
                    timers.all.confActive = [];
                  }

                  interval = $interval(function() {
                    restService.getConfPartList(secondary.conference.select.name)
                      .then(function (data) {

                        if (!_.isEmpty(backup)) {
                          _.each(_.filter(backup, function(obj){ return !_.findWhere(data.conference.members, obj); }), function (obj) {
                            _.find(secondary.conference.participants, function (part) {
                              if ((part.profile && part.profile.vCard && obj.name.toString() === part.profile.vCard['X-INTERN']) || (part.name === obj.imdId || part.imId === obj.name || part.imId === obj.imId)) {
                                part.conf.active = null;
                                return true;
                              }
                            })
                          })
                        }

                        backup = data.conference.members;

                        if (data !== null) {
                          secondary.conference.status  = 'Call in progress...';

                          _.each(data.conference.members, function (member) {
                            conf = {
                              'id':          member.id,
                              'canHear':     member.canHear,
                              'canSpeak':    member.canSpeak,
                              'muteDetect':  member.muteDetect,
                              'active':      true
                            };

                            exists = _.find(secondary.conference.participants, function (part) {
                              if ((part.number && part.number.toString() === member.name.toString()) || (part.name && part.name.toString() === member.name.toString())){
                                part.conf = conf;
                                return true;
                              } else {
                                part.conf.active = null;
                              }
                            });

                            if (!exists) {
                              found = _.find(restService.phonebook, function (item) {
                                if (member.name.toString() === item.number) {
                                  _.extend(item, { conf: conf });
                                  secondary.conference.participants.push(item);
                                  return true;
                                }
                              });

                              if (!found) {
                                secondary.conference.participants.push({
                                  name: member.imId || member.name,
                                  conf: conf
                                });
                              }
                            }

                          });
                          secondary.conference.active       = true;
                          secondary.conference.btnDisabled  = false;
                        } else {
                          secondary.conference.status       = 'Waiting for an active conference room...';
                          secondary.conference.active       = false;
                          secondary.conference.btnDisabled  = false;
                          _.each(secondary.conference.participants, function (part) {
                            if (part.conf) {
                              part.conf.active = null;
                            }
                          });
                          secondary.conference.timers.cancelAll();
                        }
                      })
                      .catch(function () {
                        // secondary.conference.timers.cancelAll(true);
                        // secondary.conference.btnDisabled  = false;
                        // secondary.conference.status       = 'Error ' + status;

                        secondary.conference.status       = 'Waiting for an active conference room...';
                        secondary.conference.active       = false;
                        secondary.conference.btnDisabled  = false;
                        _.each(secondary.conference.participants, function (part) {
                          if (part.conf) {
                            part.conf.active = null;
                          }
                        });
                        secondary.conference.timers.cancelAll();
                      });
                  }, 1500);

                  timers.all.confActive.push(interval);
                  break;

              }

            },

            cancelAll: function (force) {
              var stopTimers = function () {
                var keys = Object.keys(timers.all);

                _.each(keys, function (key) {
                  if (key.indexOf('conf') !== -1 ) {
                    _.each(timers.all[key], function (timer) {
                      $interval.cancel(timer);
                    });
                  }
                });

                secondary.conference.refresh                  = true;
                secondary.conference.timers.currentTimeouts   = 0;
                secondary.conference.status = 'No active conference rooms found. ';

              };

              if (force) {

                stopTimers();
                return;
              }

              if (secondary.conference.timers.currentTimeouts < secondary.conference.timers.maxTimeouts) {
                secondary.conference.timers.currentTimeouts++;
                return;
              } else {
                stopTimers();
                return;
              }

            }

          }

        },

        settings: {

          personalAttendant: {

            main:     null,
            showNew:  false,
            numbers:  ['1', '2', '3', '4', '5', '6', '7', '8', '9'],
            emptyArr: [],
            lang:     ['Default'],

            init:     function () {
              secondary.settings.errors.notAvailable = null;
              restService.getPersonalAttendant().then(function (data) {
                if (data.language === null) {
                  data.language = 'Default';
                }
                secondary.settings.personalAttendant.lang = ['Default'];
                secondary.settings.personalAttendant.main = data;
                secondary.settings.success = false;
                secondary.settings.warning = false;
                restService.getPersonalAttendantLang().then(function (data) {
                  secondary.settings.personalAttendant.lang = secondary.settings.personalAttendant.lang.concat(data);
                })
              })
              .catch(function () {
                secondary.settings.errors.notAvailable = true;
              });
            },

            rem: function (d, i, key) {
              if (d.length) {
                d.splice(i, 1);
              } else {
                delete d[key]
              }
            },

            add: function () {
              var obj = {
                no: '',
                key: ''
              }
              secondary.settings.personalAttendant.showNew = true;
              secondary.settings.personalAttendant.emptyArr.push(obj);
            },

            save: function () {
              var main = angular.copy(secondary.settings.personalAttendant.main);
              var arr = angular.copy(secondary.settings.personalAttendant.emptyArr);

              if (main.language === 'Default') {
                main.language = null;
              }

              secondary.settings.personalAttendant.showNew = false;

              if (arr.length > 0) {
                _.each(arr, function (obj) {
                  main.menu[obj.key] = obj.no;
                })
              }

              secondary.settings.personalAttendant.emptyArr  = [];
              secondary.settings.loading                     = true;
              restService.putPersonalAttendant(angular.copy(main)).then(function () {
                secondary.settings.success   = true;
                secondary.settings.loading   = false;
                secondary.settings.personalAttendant.init();
              }).catch(function (err) {
                secondary.settings.success   = false;
                secondary.settings.warning   = err.toString();
                secondary.settings.loading   = false;
              });
            },

            cancel: function () {
              secondary.settings.personalAttendant.main       = null;
              secondary.settings.personalAttendant.emptyArr   = [];
              secondary.settings.personalAttendant.init();

            }

          },

          fwd: {

            visible: 'setup',

            show: function (name) {
              secondary.settings.fwd.visible = name;
              switch (name) {
                case 'setup':
                  secondary.settings.fwd.setup.init();
                  break;
                case 'schedule':
                  secondary.settings.fwd.sched.init();
                  break;
              }
              return;
            },

            setup: {

              resp:             null,
              selected:         null,
              showEmpty:        false,
              custom:           [],
              defaultSchedules: [null],
              defaultTypes: [
                'At the same time',
                'If no response',
              ],
              default: {
                'expiration': '',
                'type': 'At the same time',
                'enabled': '',
                'number': ''
              },

              init: function () {
                secondary.settings.fwd.setup.resp               = null;
                secondary.settings.success                      = false;
                secondary.settings.warning                      = false;
                secondary.settings.fwd.setup.showEmpty          = false;
                secondary.settings.errors.notAvailable          = null;
                secondary.settings.fwd.setup.defaultSchedules   = [null];
                secondary.settings.fwd.visible                  = 'setup';
                restService.getForward().then(function (data) {
                  secondary.settings.fwd.setup.resp = data;
                  return restService.getSchedule();
                }).then(function (data) {
                  secondary.settings.fwd.sched.main = data;
                  _.each(data, function (obj) {
                    secondary.settings.fwd.setup.defaultSchedules.push(obj.scheduleId);
                    secondary.settings.fwd.setup.defaultSchedules = _.uniq(secondary.settings.fwd.setup.defaultSchedules);
                  });
                }).catch(function () {
                  secondary.settings.errors.notAvailable = true;
                });
              },

              add: function () {
                secondary.settings.fwd.setup.showEmpty = true;
                secondary.settings.fwd.setup.resp.rings.push(angular.copy(secondary.settings.fwd.setup.default));
              },

              rem: function (d, i) {
                d.splice(i, 1);
              },

              moveRing: function (from, to) {
                if (to === -1) {
                  return;
                }

                arrayMove(secondary.settings.fwd.setup.resp.rings, from, to);

                function arrayMove(arrayVar, from, to) {
                  arrayVar.splice(to, 0, arrayVar.splice(from, 1)[0]);
                }
              },

              save: function () {
                secondary.settings.success = false;
                secondary.settings.warning = false;
                secondary.settings.loading = true;
                restService.putForward(angular.copy(secondary.settings.fwd.setup.resp)).then(function (data) {
                  secondary.settings.loading = false;
                  secondary.settings.success = true;
                }, function (err) {
                  secondary.settings.loading = false;
                  secondary.settings.warning = true;
                })
              },

              cancel: function () {
                secondary.settings.fwd.setup.custom      = [];
                secondary.settings.fwd.setup.showEmpty   = false;
                secondary.settings.fwd.setup.init();
              }

            },

            sched: {

              backup:       null,
              showNew:      false,
              remArray:     [],
              main:         null,
              selected:     null,
              predefined:   [-2,-1,0,1,2,3,4,5,6,7],
              showNewEmpty: {
                'name':'',
                'description':'',
                'periods':[
                  {
                    'end': '',
                    'start': '',
                    'scheduledDay':''
                  }
                ],
                'scheduleId':''
              },

              init: function () {
                secondary.settings.errors.notAvailable = null;
                restService.getSchedule().then(function (data) {
                  secondary.settings.fwd.sched.convertTimeTo(data, 'normal');
                  secondary.settings.fwd.sched.backup    = angular.copy(data);
                  secondary.settings.fwd.sched.main      = data;
                  secondary.settings.fwd.sched.selected  = secondary.settings.fwd.sched.main[0];
                }).catch(function () {
                  secondary.settings.errors.notAvailable = true;
                });
              },

              add: function (to) {
                to.periods.push({
                  'end':'',
                  'start':'',
                  'scheduledDay':-2
                });
                return;
              },

              rem: function (to, index) {
                to.periods.splice(index, 1);
                return;
              },

              remSelected: function () {
                var selected = angular.copy(secondary.settings.fwd.sched.selected);
                secondary.settings.fwd.sched.remArray.push(selected.scheduleId);
                secondary.settings.fwd.sched.main = _.filter(angular.copy(secondary.settings.fwd.sched.main), function (obj) {
                  return (obj.name !== selected.name);
                })
                secondary.settings.fwd.sched.selected = secondary.settings.fwd.sched.main[0];
              },

              addNew: function () {
                var schedule = {};

                if (secondary.settings.fwd.sched.showNew) {
                  schedule = angular.copy(secondary.settings.fwd.sched.showNewEmpty);
                  secondary.settings.fwd.sched.main.push(schedule);
                  secondary.settings.fwd.sched.showNew = false;
                  secondary.settings.fwd.sched.convertTimeTo(schedule, 'sep');
                  postSchedule(schedule);
                } else {
                  secondary.settings.fwd.sched.showNew = true;
                }

                function postSchedule(schedule) {
                  restService.postSchedule(schedule).then(function (data) {
                    return restService.getSchedule();
                  }).then(function (data) {
                    _.find(data, function (o) {
                      if (schedule.name === o.name) {
                        schedule.scheduleId = o.scheduleId;
                        return true;
                      }
                    })
                    secondary.settings.fwd.sched.convertTimeTo(data, 'normal');
                    secondary.settings.fwd.sched.backup    = angular.copy(data);
                    secondary.settings.fwd.sched.main      = data;
                    secondary.settings.fwd.sched.selected  = secondary.settings.fwd.sched.main[0];
                  }).catch(function (err) {
                    console.log(err);
                  });
                }
              },

              hideNewEmpty: function () {
                secondary.settings.fwd.sched.showNew       = false;
                secondary.settings.fwd.sched.remArray      = [];
                secondary.settings.fwd.sched.showNewEmpty  = {
                  'name':'',
                  'description':'',
                  'periods':[
                    {
                      'end':'',
                      'start':'',
                      'scheduledDay':''
                    }
                  ]
                }
              },

              convertTimeTo: function (mainArr, type) {
                switch (type) {
                  case 'normal':

                    if (typeof mainArr === 'object' && mainArr.periods) {
                      _.each(mainArr.periods, function (time) {
                        time.end    = time.end.hrs + ':' + (time.end.min > -1 && time.end.min < 10 ? '0' + time.end.min : time.end.min);
                        time.start  = time.start.hrs + ':' + (time.start.min > -1 && time.start.min < 10 ? '0' + time.start.min : time.start.min);
                      })
                    } else {
                      _.each(mainArr, function (obj) {
                        _.each(obj.periods, function (time) {
                          time.end    = time.end.hrs + ':' + (time.end.min > -1 && time.end.min < 10 ? '0' + time.end.min : time.end.min);
                          time.start  = time.start.hrs + ':' + (time.start.min > -1 && time.start.min < 10 ? '0' + time.start.min : time.start.min);
                        })
                      });
                    }

                    return mainArr;
                  case 'sep':
                    if (typeof mainArr === 'object' && mainArr.periods) {
                      _.each(mainArr.periods, function (time) {
                        time.end = {
                          hrs: time.end.split(':')[0],
                          min: time.end.split(':')[1]
                        };
                        time.start = {
                          hrs: time.start.split(':')[0],
                          min: time.start.split(':')[1]
                        };
                      })
                    } else {
                      _.each(mainArr, function (obj) {
                        _.each(obj.periods, function (time) {
                          time.end = {
                            hrs: time.end.split(':')[0],
                            min: time.end.split(':')[1]
                          };
                          time.start = {
                            hrs: time.start.split(':')[0],
                            min: time.start.split(':')[1]
                          };
                        })
                      });
                    }

                    return mainArr;
                }
              },

              save: function () {
                var main    = angular.copy(secondary.settings.fwd.sched.main);
                var rem     = angular.copy(secondary.settings.fwd.sched.remArray);

                secondary.settings.fwd.sched.convertTimeTo(main, 'sep');

                secondary.settings.success = false;
                secondary.settings.warning = false;
                secondary.settings.loading = true;

                if (main.length > 0) {
                  _.each(main, function (obj) {
                    if (obj.scheduleId !== '') {
                      restService.putSchedule(obj.scheduleId, obj).then(function (data) {
                        secondary.settings.loading = false;
                        secondary.settings.success = true;
                      }, function (err) {
                        secondary.settings.loading = false;
                        secondary.settings.warning = true;
                        console.log(err);
                      })
                    } else {
                      restService.postSchedule(obj).then(function (data) {
                        secondary.settings.loading = false;
                        secondary.settings.success = true;
                        return restService.getSchedule();
                      }).then(function (data) {
                        _.find(data, function (o) {
                          if (obj.name === o.name) {
                            obj.scheduleId = o.scheduleId;
                            return true;
                          }
                        })
                      }).catch(function (err) {
                        secondary.settings.loading = false;
                        secondary.settings.warning = true;
                        console.log(err);
                      });
                    }
                  });
                }

                if (rem.length > 0) {
                  _.each(rem, function (sched) {
                    restService.delSchedule(sched).then(function (data) {
                      secondary.settings.loading = false;
                      secondary.settings.success = true;
                    }, function (err) {
                      secondary.settings.loading = false;
                      secondary.settings.warning = true;
                    })
                  })
                }
              },

              cancel: function () {
                secondary.settings.fwd.sched.showNew  = false;
                secondary.settings.fwd.sched.main     = null;
                secondary.settings.fwd.sched.init();
              }

            }
          },

          speed: {

            data: null,

            init: function () {
              secondary.settings.errors.notAvailable = null;
              restService.getSpeeddial().then(function (data) {
                secondary.settings.speed.data = data;
              }).catch(function () {
                secondary.settings.errors.notAvailable = true;
              })
            },

            removeEntry: function (index) {
              secondary.settings.speed.data.buttons.splice(index, 1);
              return;
            },

            addEntry: function () {
              var obj = {
                number: '',
                label: '',
                blf: false
              }
              secondary.settings.speed.data.buttons.push(obj);
              return;
            },

            move: function (from, to) {
              if (to === -1) {
                return;
              }
              arrayMove(secondary.settings.speed.data.buttons, from, to)
              function arrayMove(arrayVar, from, to) {
                arrayVar.splice(to, 0, arrayVar.splice(from, 1)[0]);
              }
            },

            save: function (updatePhones) {
              var data = angular.copy(secondary.settings.speed.data);
              if (updatePhones) {
                data.updatePhones = true;
              }
              secondary.settings.loading = true;
              secondary.settings.success = false;
              secondary.settings.warning = false;
              restService.putSpeeddial(data).then(function (data) {
                secondary.settings.success = true;
                restService.getSpeeddial().then(function (data) {
                  secondary.settings.loading = null;
                  secondary.settings.speed.data = data;
                }, function (err) {
                  secondary.settings.loading = null;
                  console.log(err);
                })
              }, function (err) {
                secondary.settings.warning = true;
                secondary.settings.loading = null;
                console.log(err);
              })
            },

            cancel: function () {
              secondary.settings.speed.init();
            }

          },

          user: {
            pass:     '',
            loading:  null,
            buddy:    null,
            success:  false,
            warning:  false,
            vm: {
              main: null,
              selected: null,
              pass: null,
              select: [
                {
                  name: 'Default system',
                  val: 'NONE'
                },
                {
                  name: 'Standard',
                  val: 'STANDARD'
                },
                {
                  name: 'Out of office',
                  val: 'OUT_OF_OFFICE'
                },
                {
                  name: 'Extended absence',
                  val: 'EXTENDED_ABSENCE'
                }
              ]
            },

            conf: {
              main: null,
              changeConf: function () {
                var name = angular.copy(secondary.settings.user.conf.selected.name);
                restService.getConfSettings(name).then(function (data) {
                  secondary.settings.user.conf.main = data.setting;
                }).catch(function (err) {
                  console.log(err);
                });
              }
            },

            init: function () {
              secondary.settings.errors.notAvailable   = null;
              secondary.settings.errors.myBuddy        = null;
              secondary.settings.errors.voicemail      = null;
              secondary.settings.errors.confBridge     = null;

              restService.getImBot().then(function (data) {
                secondary.settings.user.buddy = data;
              }).catch(function (err) {
                secondary.settings.errors.myBuddy = true;
              });

              restService.getVmPrefs().then(function (data) {
                secondary.settings.user.vm.main = data;
                setVmSelected();
              }).catch(function (err) {
                secondary.settings.errors.voicemail = true;
              });

              restService.getConfList().then(function (data) {
                if (data.conferences && data.conferences.length === 0) {
                  secondary.settings.errors.confBridge = true;
                  return;
                }
                secondary.settings.user.conf.select = data.conferences;
                secondary.settings.user.conf.selected = secondary.settings.user.conf.select[0];
                secondary.settings.user.conf.changeConf();
              }).catch(function (err) {
                secondary.settings.errors.confBridge = true;
              });

              function setVmSelected () {
                var i = 0;
                var greeting = angular.copy(secondary.settings.user.vm.main.greeting);
                for (i = 0; i < secondary.settings.user.vm.select.length; i++) {
                  if (greeting === secondary.settings.user.vm.select[i].val) {
                    secondary.settings.user.vm.selected = secondary.settings.user.vm.select[i];
                  }
                };
              }
            },

            save: function () {

              savePassword();
              saveImBot();
              saveVm();
              saveVmPass();
              saveConf();

              function savePassword() {
                var pass = angular.copy(secondary.settings.user.pass);
                if (pass.length === 0) {
                  return;
                }
                if (pass.length < 8) {
                  secondary.settings.user.errors.pass = true;
                  return
                }
                secondary.settings.user.loading = true;
                restService.putPassword(pass).then(function (data) {
                  secondary.settings.user.loading = null;
                }, function (err) {
                  console.log(err);
                })
              }

              function saveImBot() {
                var buddy = angular.copy(secondary.settings.user.buddy);
                secondary.settings.user.loading = true;
                restService.putImBot(buddy).then(function (data) {
                  restService.getImBot().then(function (data) {
                    secondary.settings.user.loading = null;
                    secondary.settings.user.buddy = data;
                  }, function (err) {
                    secondary.settings.user.loading = null;
                    console.log(err);
                  })
                }, function (err) {
                  secondary.settings.user.loading = null;
                  console.log(err);
                })
              }

              function saveVm() {
                if (secondary.settings.user.vm.main.email) {
                  secondary.settings.user.vm.main.emailAttachType      = 'YES';
                  secondary.settings.user.vm.main.emailFormat          = 'FULL';
                }
                if (secondary.settings.user.vm.main.altEmail) {
                  secondary.settings.user.vm.main.altEmailAttachType   = 'YES';
                  secondary.settings.user.vm.main.altEmailFormat       = 'FULL';
                }
                secondary.settings.user.vm.main.greeting = secondary.settings.user.vm.selected.val;

                restService.putVmPrefs(angular.copy(secondary.settings.user.vm.main)).catch(function (err) {
                  console.log(err);
                })
              }

              function saveConf() {
                var data = angular.copy(secondary.settings.user.conf.main);
                var conf = angular.copy(secondary.settings.user.conf.selected.name);

                restService.putConfSettings(conf, {setting: data}).catch(function (err) {
                  console.log(err);
                })
              }

              function saveVmPass() {
                if (secondary.settings.user.vm.pass === null) {
                  return
                } else {
                  var pass = angular.copy(secondary.settings.user.vm.pass);
                  restService.putVmPin(pass).catch(function (err) {
                    console.log(err);
                  })
                }
              }
            },

            cancel: function () {
              secondary.settings.user.init();
            },

            errors: {
              pass: null
            }
          },

          errors: {
            notAvailable: false,
            voicemail:    false,
            confBridge:   false,
            myBuddy:      false
          }
        },

        profile: {
          info: null,
          util: {
            success: null,
            failure: null,
            loading: null
          },
          init: function () {
            secondary.profile.info = null;
            secondary.profile.util.success = null;
            secondary.profile.util.failure = null;
            secondary.profile.util.loading = null;
            restService.getContactInfo().then(function (data) {
              secondary.profile.info          = angular.copy(secondary.profile.default);
              secondary.profile.info.profile  = util.deepExtend(angular.copy(secondary.profile.info.profile), data['contact-information']);
            }, function (err) {
              console.log(err);
            });
          },

          save: function () {
            var info = {'contact-information': util.deepRemoveEmpty(angular.copy(secondary.profile.info.profile))};
            secondary.profile.util.loading = true;
            restService.putContactInfo(info).then(function (data) {
              secondary.profile.util.loading = false;
              secondary.profile.util.success = true;
              setTimeout(function() {
                secondary.profile.util.success = false;
              }, 1000);
            }, function (err) {
              secondary.profile.util.loading = false;
              secondary.profile.util.failure = true;
              console.log(err);
            });
          },

          default: {
            name:     null,
            location: null,
            photoSrc: null,
            profile:  {
              alternateEmailAddress: '',
              alternateImId: '',
              assistantName: '',
              assistantPhoneNumber: '',
              cellPhoneNumber: '',
              companyName: '',
              didNumber: '',
              emailAddress: '',
              facebookName: '',
              faxNumber: '',
              firstName: '',
              homeAddress: {
                street:'',
                city:'',
                state:'',
                country:'',
                zip:''
              },
              homePhoneNumber: '',
              imDisplayName: '',
              imId: '',
              jobDept: '',
              jobTitle: '',
              lastName: '',
              linkedinName: '',
              location: '',
              officeAddress: {
                street:'',
                city:'',
                state:'',
                country:'',
                zip:'',
                officeDesignation:''
              },
              twiterName: '',
              useBranchAddress: '',
              xingName: ''
            }
          }

        }

      };

      var util = {

        openChat : null,

        populateContactList : function () {
          rosterList.main     = restService.phonebook;
          activityList.main   = restService.activityList;
          // restService.getCDRlog(20).then(function (data) {
          // })
          restService.getAllImages(rosterList.main);

          // keep alive every 10 minutes
          $interval(function () {
            restService.getKeepAlive();
          }, 600000);
          return;
        },

        deepRemoveEmpty : function (source) {
          var destination = {};

          for (var property in source) {
            if (!_.isEmpty(source[property])) {
              if ((source.hasOwnProperty(property)) && (typeof source[property] === 'object')) {
                destination[property] = source[property] || {};
                util.deepRemoveEmpty(source[property]);
              } else {
                destination[property] = source[property];
              }
            }

          }

          return destination;
        },

        deepExtend : function (destination, source) {
          for (var property in source) {
            if ((source[property]) && (typeof source[property] === 'object')) {
              destination[property] = destination[property] || {};
              util.deepExtend(destination[property], source[property]);
            } else {
              destination[property] = source[property];
            }
          }

          return destination;
        },

        changeView : function (view) {

          if (view.type) {
            secondary.template.main             = view;
            secondary.voicemail.messages        = [];
            secondary.conference.participants   = [];
            secondary.conference.active         = false;
            secondary.conference.timers.cancelAll(true);
            if ((secondary[view.fn]) && (secondary[view.fn].init)) {
              secondary[view.fn].init();
            }
            $rootScope.leftSideView = 'hide-me';
          } else {
            if (ui.root.oldTemplate.name !== view.name) {
              ui.root.oldTemplate = angular.copy(ui.root.template);
            }
            ui.root.template = view;
          }

          return;
        }
      };

      return {
        ui:           ui,
        secondary:    secondary,
        util:         util,
        rosterList:   rosterList,
        activityList: activityList,
        groupChat:    groupChat,
        search:       search
      };
    }
  ]);
})();
