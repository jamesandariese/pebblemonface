Pebble.addEventListener("appmessage",
  function(e) {
    console.log("Received message: " + e.payload);
  }
);

function HTTPGETJSON(url) {
    var req = new XMLHttpRequest();
    req.open("GET", url, false);
    req.send(null);
    return JSON.parse(req.responseText);
}

function getOneMessage() {
  // This code is probably fine but I switched to synchronous because it's
  // going to be more durable.  That being said, this code probably works.
  // It had a bug which I found after preserving it in this horrible little
  // comment.
 var req = new XMLHttpRequest();
  req.open('GET', 'http://linode.strudelline.net:8088/pull?user=a', true);
  req.onerror = function(e) {
      console.log("error!");
      setTimeout(getOneMessage, 0);
  }
  
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        var json = JSON.parse(req.responseText);
        if (json.response == "timeout") {
          setTimeout(getOneMessage, 0);
	  return;
        }
        var title = json.response.title;
        var message = json.response.message;
        console.log("---");
        console.log(title);
        console.log(message);
        console.log("---");
        Pebble.showSimpleNotificationOnPebble(title, message);
      } else {
        console.log("Error");
      }
      setTimeout(getOneMessage, 0);
    }
  }
  req.send(null);
/*    json = HTTPGETJSON('http://linode.strudelline.net:8088/pull?user=a')
    var title = json.response.title;
    var message = json.response.message;
    if (json.response == "timeout") {
        setTimeout(getOneMessage, 0);
	return;
    }
    console.log("---");
    console.log(title);
    console.log(message);
    console.log("---");
    Pebble.showSimpleNotificationOnPebble(title, message);
    setTimeout(getOneMessage, 0);*/
 }

Pebble.addEventListener("ready",
    function(e) {
        console.log("Hello world! - Sent from your javascript application.");
        getOneMessage();
   }
);
