import 'dart:html';
import 'dart:convert'; 
import 'dart:async';
import 'package:intl/intl.dart';
import 'package:sipxconfig/sipxconfig.dart';

var api = new Api(test : true);

main() {
  var backup = new BackupPage();
  var tabs = new Tabs(querySelector("#leftNavAbsolute"), ["local", "ftp"], backup.showContent);
  tabs.setPersistentStateId("backup");
}

class BackupPage {
  var msg = new UserMessage(querySelector("#message"));
  DataLoader loader;
  String type = 'local';
  SettingEditor settings;
  SettingEditor ftpSettings;
  var timeOfDayFormat = new DateFormat("jm");
  Timer refresh;
  bool isInProgress;
  
  BackupPage() {
    querySelector("#backup-now").onClick.listen(backupNow);
    querySelector("#apply").onClick.listen(apply);
    loader = new DataLoader(this.msg, loadForm);
    settings = new SettingEditor(querySelector("#settings"));
    ftpSettings = new SettingEditor(querySelector("#ftp-settings"));
    isInProgress = false;
    refresh = new Timer.periodic(new Duration(seconds: 30), (e) {
      if (isInProgress) {                                                                            
        load();
      }
    });
  }
  
  load() {
    var url = api.url("rest/backup/${type}", "backup-test.json");
    loader.load(url);    
  }
  
  loadForm(json) {
    var data = JSON.decode(json);
    Map <String, Object> backupSettings = getSetting(data['settings'], "backup");
    settings.render(backupSettings, 'backup/');
    Map <String, Object> ftpSettings = getSetting(data['settings'], "ftp");
    this.ftpSettings.render(ftpSettings);
    var archiveIds = new List<String>();
    var plan = data['backup'];
    if (plan != null) {    
      SelectElement limit = querySelector("#backup-limit");
      int count = plan['limitedCount'];
      if (count == null) {
        limit.value = null;
      } else {
        limit.value = count.toString();
      }
            
      int dow = 0;
      bool enabled = false;
      String timeOfDay = "";
      archiveIds = plan['definitionIds'];
      // backend uses array even though we only support a single schedule ATM
      List<Map> schedules = plan['schedules'];
      if (schedules != null && schedules.length > 0) {
        dow = schedules[0]['scheduledDay']['dayOfWeek'];
        enabled = schedules[0]['enabled'];
        
        Map<String, String> t = schedules[0]['timeOfDay'];
        if (t != null) {
          var dt = new DateTime(0, 1, 1, t['hrs'] as int, t['min'] as int);
          timeOfDay = timeOfDayFormat.format(dt);
        }
      }
      (querySelector("#dailyScheduledTime") as InputElement).value = timeOfDay;
      (querySelector("#dailyScheduledDay") as SelectElement).selectedIndex = dow;
      (querySelector("#dailyScheduleEnabled") as InputElement).checked = enabled;
    }

    UListElement archives = querySelector("#archives");
    archives.children.clear();
    Map<String, String> defs = data['definitions'];
    if (defs != null) {
      defs.forEach((defId, label) {
        var checked = archiveIds.contains(defId) ? "checked" : "";
        archives.appendHtml('''
<li>
  <input type="checkbox" name="definitionIds" value="${defId}" ${checked}/>
  ${label}
</li>
''');              
      });
    }  
    
    Map<String, List<String>> backups = data['backups'];
    if (backups != null) {
      UListElement listElem = querySelector("#backups");
      listElem.children.clear();
      backups.forEach((backupId, backupFiles) {
        var html = "<li>${backupId} - ";
        for (String f in backupFiles) {
          var decode = f.split('|');
          html += "<a href='${decode[1]}'>${decode[0]}</a> ";
        }
        listElem.appendHtml(html + "</li>");
      });
    }
    
    inProgress(true == data['inProgress']);
  }
  
  inProgress(bool inProg) {
    isInProgress = inProg;
    querySelector('#inprogress').hidden = ! isInProgress;
    for (var e in querySelectorAll('.action')) {    
      e.disabled = isInProgress;
    }
  }

  Map<String, Object> getSetting(Map<String, Object> settings, String path) {
    var selected = settings;
    for (var segment in path.split("/")) {
      selected = selected['value'][segment];  
    }
    return selected;    
  }
  
  parseForm() {
    var form = new Map<String, Object>();
    form['limitedCount'] = int.parse((querySelector("#backup-limit") as SelectElement).value);
    List<String> definitionIds = new List<String>();
    for (CheckboxInputElement c in querySelectorAll("input[name=definitionIds]")) {
      if (c.checked) {
        definitionIds.add(c.value);
      }
    }
    form['definitionIds'] = definitionIds;
    var timeStr = (querySelector("#dailyScheduledTime") as InputElement).value;
    DateTime timeOfDay = timeOfDayFormat.parse(timeStr);
    int dayOfWeek = int.parse((querySelector("#dailyScheduledDay") as SelectElement).value);
    var enabled = (querySelector("#dailyScheduleEnabled") as CheckboxInputElement).checked;
    form['schedules'] = [{
      "timeOfDay" : {
          "hrs" : timeOfDay.hour,
          "min" : timeOfDay.minute 
      },
      "scheduledDay" : {
        "dayOfWeek" : dayOfWeek
      },
      "enabled" : enabled
    }];

    return form;
  }
  
  backupNow(e) {
    postOrPut('POST', 'message.backupCompleted');
  }
  
  apply(e) {
    postOrPut('PUT', 'msg.actionSuccess');
  }
  
  postOrPut(String method, String successMessage) {
    var meta = new Map<String, Object>();
    meta['settings'] = settings.parseForm();
    meta['backup'] = parseForm();    
    HttpRequest req = new HttpRequest();
    req.open(method, api.url("rest/backup/${type}"));
    req.setRequestHeader("Content-Type", "application/json"); 
    req.send(JSON.encode(meta));
    req.onLoad.listen((e) {
      if (DataLoader.checkResponse(msg, req)) {
        msg.success(getString(successMessage));
        load();
      }
    });      
  }
  
  showContent(Tabs tabs, String selectedId) {    
    type = selectedId;
    tabs.showTabContent(selectedId);    
    load();
  }
}