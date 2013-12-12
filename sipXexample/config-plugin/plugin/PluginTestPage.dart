import 'dart:html';
import 'package:sipxconfig/sipxconfig.dart';

// Api class helps load static data when back-end is down or still in development
// see source in sipxconfig.dart for more info.  
// NOTE: When test is false, you must add "--disable-web-security" launch 
var api = new Api(test : false);

main() {
  
  // Using classes is not strictly nec. but often convient way to organize your code  
  new PluginTest();
}

class PluginTest {
  var msg = new UserMessage(querySelector("#message"));
  DataLoader loader;

  PluginTest() {
    querySelector("#apply").onClick.listen(apply);
    loader = new DataLoader(this.msg, loadForm);    
    load();
  }
  
  apply(e) {    
    if (window.confirm(getString('confirm.foxMessage'))) {
      // push data to server
      // reload page
      msg.success(getString("msg.actionSuccess"));
    }
  }
  
  load() {
    var url = api.url("rest/example", "test.json");
    loader.load(url);
  }
  
  loadForm(text) {
    (querySelector("#fox") as InputElement).value = text;     
  }
}