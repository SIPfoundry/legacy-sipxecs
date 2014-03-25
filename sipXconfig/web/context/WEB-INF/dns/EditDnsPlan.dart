/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
import 'dart:html';
import 'dart:convert';
import 'dart:math';
import 'package:sipxconfig/sipxconfig.dart';

var api = new Api(test : false);

main() {
  new DnsPlanEditor();
}

class DnsPlanEditor {
  var msg = new UserMessage(querySelector("#message"));
  DataLoader loader;
  int uid = 0;
  var groups = new List<Element>();
  Map <int,String> targetOptions;
  Map itemPrototype;
  Map<String, Object> groupPrototype;
  int dnsPlanId;

  DnsPlanEditor() {
    var planDom = querySelector("#plan-picker");
    querySelector("#ok").onClick.listen(ok);
    querySelector("#apply").onClick.listen(apply);
    querySelector("#cancel").onClick.listen(cancel);
    Location l = document.window.location;
    var params = Uri.parse(l.href).queryParameters;
    if (params['dnsPlanId'] != null) {
      dnsPlanId = int.parse(params['dnsPlanId']);
    }
    loader = new DataLoader(this.msg, loadForm);
    load();
    itemPrototype = {
      "targetType" : "BASIC",
      "targetId" : "ALL_OTHER_LOCATIONS",
      "percentage" : "0"
    };
    groupPrototype = {
       "targets" : [ {
            "targetType" : "BASIC",
            "targetId" : "ALL_OTHER_LOCATIONS",
            "percentage" : "100"
          }
       ]
    };
  }

  ok(e) {
    save(close);
  }

  apply(e) {
    save();
  }

  cancel(e) {
    close();
  }

  void save([onOk]) {
    var id = (dnsPlanId != null ? dnsPlanId : '');
    HttpRequest req = new HttpRequest();
    var meta = getPlanByScrapingForm();
    var method;
    if (dnsPlanId != null) {
      meta['id'] = dnsPlanId;
      method = 'PUT';
    } else {
      method = 'POST';
    }
    req.open(method, api.url("rest/dnsPlan/${id}"));
    req.setRequestHeader("Content-Type", "application/json");
    req.send(JSON.encode(meta));
    req.onLoadEnd.listen((e) {
      if (DataLoader.checkResponse(msg, req)) {
        if (dnsPlanId == null) {
          dnsPlanId = int.parse(req.responseText);
        }
        if (onOk != null) {
          msg.success("Save successful");
          onOk();
        }
      }
    });
  }

  Map<String, Object> getPlanByScrapingForm() {
    var form = querySelector("#edit-plan").querySelectorAll("input,select");
    var plan = new Map<String, Object>();
    plan['name'] = (form[0] as InputElement).value;
    var groups = new List<Map<String, Object>>();
    plan['groups'] = groups;
    List<Map<String, Object>> targets;
    Map<String, Object> target;
    for (HtmlElement i in form.sublist(1)) {
      if (i.id.startsWith("group-")) {
        var group = new Map<String, Object>();
        targets = new List();
        group['targets'] = targets;
        groups.add(group);
      } else if (i.id.startsWith("percentage-")) {
        var percentage = (i as NumberInputElement).value;
        try {
          targets.last['percentage'] = int.parse(percentage);
        } on FormatException {
          targets.last['percentage'] = 0;
        }
      } else if (i.id.startsWith("target-")) {
        SelectElement se = i;
        var target = new Map<String, Object>();
        var targetValue = se.value.split("-");
        target['targetType'] = targetValue[0];
        target['targetId'] = targetValue[1];
        targets.add(target);
      }
    }
    return plan;
  }

  close() {
    window.location.href = 'EditDns.html';
  }

  load() {
    var id = (dnsPlanId != null ? dnsPlanId : 'blank');
    var url = api.url("rest/dnsPlan/${id}", "edit-failover-test.json");
    loader.load(url);
  }

  loadForm(json) {
    var data = JSON.decode(json);
    targetOptions = data['targetCandidates'];
    Map<String, Object> plan = data['plan'];
    (querySelector("#name") as InputElement).value = plan['name'];
    List groups = plan['groups'];
    if (groups != null) {
      for (Map<String, Object> group in groups) {
        addGroup(null, group);
      }
    }
  }

  void addGroup(Element sibling, Map<String, Object> group) {
    var isFirst = (groups.length == 0);
    var label = getString(isFirst ? 'label.primaryPlan' : 'label.alternativePlan');
    var removeId = "remove-${++uid}";
    var addId = "add-${++uid}";
    var eGroup = new Element.html('''
<table>
  <tbody>
    <tr>
      <td colspan="3">
        <span>${label}<span>
        <input type="hidden" id="group-${++uid}"/>
      </td>
      <td></td>
      <td>
        <button class="subtle" id="${removeId}">-</button>
        <button class="subtle" id="${addId}">+</button>
      </td>
    </tr>
  </tbody>
</table>
''').querySelector("tr");

    ButtonElement remove = eGroup.querySelector("#${removeId}");
    remove.disabled = isFirst;
    remove.onClick.listen((_) {
      removeGroup(eGroup);
    });
    eGroup.querySelector("#${addId}").onClick.listen((_) {
      var nextSibling = lastItemInGroup(eGroup);
      addGroup(nextSibling, groupPrototype);
    });
    if (sibling == null) {
      var tbody = querySelector("#plan-picker");
      tbody.children.add(eGroup);
    } else {
      sibling.insertAdjacentElement("afterEnd", eGroup);
    }
    groups.add(eGroup);

    var juggler = new PercentageJuggler();
    Element lastRow = eGroup;
    for (Map<String, Object> target in group['targets']) {
      lastRow = addTarget(lastRow, juggler, target);
    }
  }

  List<Element> removeGroup(Element group) {
    while (true) {
      Element nextRow = group.nextElementSibling;
      if (nextRow == null || groups.contains(nextRow)) {
        break;
      }
      nextRow.remove();
    }
    group.remove();
    groups.remove(group);
  }

  Element lastItemInGroup(Element group) {
    Element item = group;
    while (true) {
      Element nextRow = item.nextElementSibling;
      if (nextRow == null || groups.contains(nextRow)) {
        return item;
      }
      item = nextRow;
    }
  }

  Element addTarget(Element sibling, PercentageJuggler juggler, Map<String, Object> target) {
    bool isFirst = juggler.size() == 0;
    var percentageId = "percentage-${++uid}";
    var itemId = "item-${++uid}";
    var removeId = "remove-${++uid}";
    var addId = "add-${++uid}";
    var eItem = new Element.html('''
<table>
  <tbody>
    <tr id="${itemId}">
      <td></td>
      <td>
        <select id="target-${++uid}">
        </select>
      </td>
      <td>
        <input id='${percentageId}' type='number' min='1' max='99' size='2' value='${target['percentage']}'/> %
      </td>
      <td>
         <button class="subtle" id="${removeId}">-</button>
         <button class="subtle" id="${addId}">+</button>
      </td>
      <td></td>
    </tr>
  </tbody>
</table>
''').querySelector('tr');
    SelectElement services = eItem.querySelector("select");
    addTargetOptions(services, targetOptions, target['targetId'].toString(), target['targetType'], 'BASIC');
    InputElement percentage = eItem.querySelector("#${percentageId}");
    ButtonElement remove = eItem.querySelector("#${removeId}");
    remove.disabled = isFirst;
    remove.onClick.listen((_) {
      juggler.remove(percentage);
      eItem.remove();
    });
    (eItem.querySelector("#${addId}") as ButtonElement).onClick.listen((_) {
      addTarget(eItem, juggler, itemPrototype);
    });

    sibling.insertAdjacentElement("afterEnd", eItem);
    juggler.add(percentage);
    return eItem;
  }

  addTargetOptions(Element select, Map options, String currentValue, String currentTargetType, String targetType) {
    options.forEach((targetId, targetValue) {
      var label = (targetType == 'BASIC' ? getString('select.${targetId}') : targetValue);
      if (targetValue is Map) {
        var separator = new OptGroupElement();
        separator.label = label;
        select.append(separator);
        // RECURSIVE !
        addTargetOptions(separator, (targetValue as Map), currentValue, currentTargetType, targetId);
      } else {
        var selected = (currentValue == targetId) && (currentTargetType == targetType);
        var optionId = "${targetType}-${targetId}";
        select.append(new OptionElement(data: label, value: optionId, selected: selected));
      }
    });
  }
}

class PercentageJuggler {
  var listeners = new Map<InputElement,Object>();
  InputElement last;

  add(InputElement percentage) {
    listeners[percentage] = percentage.onClick.listen(juggle);
    decideWhoIsLast();
  }

  int size() {
    return listeners.length;
  }

  remove(InputElement percentage) {
    listeners.remove(percentage).cancel;
    decideWhoIsLast();
  }

  decideWhoIsLast() {
    int y = 0;
    Element candidate;
    // we use parent because y offset can be 0 immediately
    // after adding element to dom.
    for (InputElement e in listeners.keys) {
      if (e.parent.offsetTop >= y) {
        candidate = e;
        y = e.parent.offsetTop;
      }
    }
    last = candidate;
    for (InputElement e in listeners.keys) {
      e.readOnly = (e == last);
    }
  }

  juggle([Event e]) {
    if (e != null) {
      // avoid recursion or at least redundant call
      if (e.target == last) {
        return;
      }
    }
    int total = 0;
    for (InputElement e in listeners.keys) {
      if (e != last) {
        total += int.parse(e.value);
      }
    }
    // we don't show negative numbers because disconcerting to users
    // allow total > 100 as it should work in backend implementation
    last.value = max(0, 100 - total).toString();
  }
}