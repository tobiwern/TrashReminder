const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
  <html lang="de">
    <head>
      <meta charset='utf-8'>
      <style type="text/css">
        .button {
          background-color: #4CAF50;
          /* Green */
          border: none;
          color: white;
          padding: 15px 32px;
          text-align: center;
          text-decoration: none;
          display: inline-block;
          font-size: 16px;
        }

        body {
          text-align: center;
        }

        table {
          width: 80%;
          text-align: center;
          margin-left: auto;
          margin-right: auto;
        }

        div.frame {
          border: 2px solid  #4CAF50;
          border-radius: 8px;
          padding: 5px;
          max-width: 350px;
          margin-left: auto;
          margin-right: auto;

        }
        div.output {
          width: 300px;
          margin: auto;
        }
        td.value {
          white-space: nowrap;
          vertical-align: center;
          text-align: left;
          padding-left: 5px;
        }

        td.description {
          vertical-align: center;
          text-align: right;
          padding-right: 5px;
        }
      </style>
      <script>
        //setInterval(function(){getData();}, 2000);

        function fireworks() { 
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              document.getElementById("message").innerHTML = this.responseText; 
              setTimeout( function(){document.getElementById("message").innerHTML ="";}, 2000);             
            }
          };
          document.getElementById("settings").innerHTML = "";
          xhttp.open("GET", "fireworks", true);
          xhttp.send();
        } 

        function closeSettings(){
          var xhttp = new XMLHttpRequest();
          xhttp.open("GET", "close", true);
          xhttp.send();
          document.getElementById("body").innerHTML ="<h1>Beendet - Bitte Fenster schließen!</h1>";
        //  window.close(); //close the page
        }

        function requestSettings(){
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              //populate options
              const dropdowns = ["start", "end"];
              dropdowns.forEach(function (item){
                for(var i = 0; i <= 23; i++) {
                    var el = document.createElement("option");
                    el.textContent = i + " Uhr";
                    el.value = i;
                    document.getElementById(item).appendChild(el);
                }              
              });
              value = this.responseText;
              tokens = value.split(",");
              document.getElementById("start").value = tokens[0];
              document.getElementById("end").value = tokens[1];
            }
          };
          xhttp.open("GET", "request_settings", true);
          xhttp.send();
        }
       
        function sendSettings(jsonText) { //send the jsonText to the ESP to be stored in LittleFS
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              response = this.responseText;
              document.getElementById("message").innerHTML = response;
              if(response.search("ERROR") != -1){
                document.getElementById("message").style.color = "red";
              } else {
                document.getElementById("message").style.color = "green";
              }
            }
          };
          xhttp.open("GET", "send_settings?value=" + jsonText, true);
          xhttp.send();
        }

        function readSettings() { //send the ESP data to the webpage
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              response = this.responseText;
              if(response.search("ERROR") != -1){
                document.getElementById("message").innerHTML = response;
                document.getElementById("message").style.color = "red";
                document.getElementById("settings").innerHTML = "";
              } else {  
                document.getElementById("settings").innerHTML = response + "<br>";
              }            
            }
          };
          xhttp.open("GET", "read_settings", true);
          xhttp.send();
        }

        function deleteSettings() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              response = this.responseText;
              document.getElementById("message").innerHTML = response;
              if(response.search("ERROR") != -1){
                document.getElementById("message").style.color = "red";
              } else {
                document.getElementById("message").style.color = "green";
                document.getElementById("settings").innerHTML = "";
              }
            }
          };
          xhttp.open("GET", "delete_settings", true);
          xhttp.send();
        }

        document.addEventListener('DOMContentLoaded', function () {
    	    enableEventListener('start');
          enableEventListener('end');
        });
        function enableEventListener(dropdown) {
          document.getElementById(dropdown).addEventListener('change', function(){sendUpdate(dropdown);});
        }
	      function sendUpdate(dropdown) {
          var value = parseInt(document.getElementById(dropdown).value);
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              response = this.responseText;
              document.getElementById("messageTime").innerHTML = "<br>" + response;
              document.getElementById("messageTime").style.color = "green";
              setTimeout( function(){document.getElementById("messageTime").innerHTML ="";}, 2000);
            }
          };
          if(dropdown == "start"){
            var endValue = parseInt(document.getElementById("end").value)
            if(value < endValue){
              var newValue = endValue+1;
              alert("Um überlappende Ereignisse an Folgetagen zu vermeiden, sollte der \"Start der Erinnerung\" (" + value + ") nicht vor dem \"Ende der Erinnerung\" (" + endValue + ") liegen!\nSetze \"Start der Erinnerung\" auf minimal zulässigen Wert (" + newValue + ").");
              value = newValue
              document.getElementById(dropdown).value = value
            }
          } else {
            var startValue = parseInt(document.getElementById("start").value)
            if(startValue < value){
              newValue = startValue-1;
              alert("Um überlappende Ereignisse an Folgetagen zu vermeiden, sollte der \"Start der Erinnerung\" (" + startValue + ") nicht vor dem \"Ende der Erinnerung\" (" + value + ") liegen!\nSetze \"Ende der Erinnerung\" auf maximal zulässigen Wert (" + newValue + ").");
              value = newValue;
              document.getElementById(dropdown).value = value
            }
          }
          xhttp.open("GET", "set_" + dropdown+ "?value=" + value, true);
          xhttp.send();          
        }
        /// ICS/iCAL Processing ////////////////////////////////////////////////////////////////////
        var debug = false;
        var items = [];
        var dateDict = {};                       //HÄCKSEL
        var colorDict = { 'PAPIER': '0x0000FF', 'BIO,CKSEL': '0x00FF00', 'GELB,WERT': '0xFFFF00', 'REST': '0xFFFFFF' }
        var colorDefault = '0xFFC0CB';
        var colors = [];
        function processFiles() {
            items = [];
            dateDict = {};
            var files = document.getElementById('files').files;
            for (var fileIndex = 0; fileIndex < files.length; fileIndex++) {
                var file = files[fileIndex];
                var reader = new FileReader();
                reader.onload = function (progressEvent) {
                    const text = this.result; //entire file
                    var lines = text.split('\n');
                    for (var i = 0; i < lines.length; i++) {
                        line = lines[i];
                        if (line.search("DTSTART") != -1) {
                            var date = line.split(":")[1];
                            dateString = date.substring(0, 4) + "-" + date.substring(4, 6) + "-" + date.substring(6, 8);
                            var epoch = new Date(dateString).getTime()/1000; //since ms => s
                        } else if (line.search("SUMMARY") != -1) {
                            var item = line.split(":")[1];
                            item = item.replace("\\", "");
                            item = item.replace("\r", "");
                        } else if (line.search("END:VEVENT") != -1) {
                            if (!(epoch in dateDict)) { dateDict[epoch] = { "tasks": [], "date": dateString }; }
                            var arr = dateDict[epoch]["tasks"];
                            arr.push(item);
                            dateDict[epoch][tasks] = arr;
                            if (!items.includes(item)) {
                                items.push(item);
                                colors = getColors();
                                genCheckboxes(items); //executed multiple times, however ok
                            }
                        }
                    }
                }; //on load
                reader.readAsText(file);
            }
        }

        function getValidTaskIds() {
            var validTaskIds = [];
            for (var i = 0; i < items.length; i++) {
                var label = document.getElementById("taskl" + i).innerText;
                if (document.getElementById("task" + i).checked) {
                    validTaskIds.push(i);
                }
            }
            return (validTaskIds);
        }

        function getTaskIds(tasks) {
            var taskIds = [];
            for (var i = 0; i < tasks.length; i++) {
                taskIds.push(items.indexOf(tasks[i]));
            }
            return (taskIds);
        }

        function getColors() {
            var colors = [];
            for (var i = 0; i < items.length; i++) {
                var item = items[i];
                var color = getMatchingColor(item);
                if (color) {
                    colors.push(color);
                } else {
                    alert("Failed to auto-assign color for entry " + item + ".<br>Assigning default color (pink).");
                    colors.push(colorDefault);
                }
            }
            return (colors);
        }

        function getMatchingColor(item) {
            var keys = Object.keys(colorDict).sort();
            for (var i = 0; i < keys.length; i++) {
                var key = keys[i];
                var entries = key.split(",");
                for (var j = 0; j < entries.length; j++) {
                    var entry = entries[j];
                    if (item.toUpperCase().search(entry) != -1) {
                        return (colorDict[key]);
                    }
                }
            }
            return (false);
        }

        function genJson() {
            validTaskIds = getValidTaskIds();
            console.log(validTaskIds);
            var entries = [];
            keys = Object.keys(dateDict).sort();
            for (var i = 0; i < keys.length; i++) {
                var epoch = keys[i];
                var tasks = dateDict[epoch]["tasks"];
                var taskIds = getTaskIds(tasks);
                var date = dateDict[epoch]["date"];
                if (debug) {
                    entries.push('{"' + epoch + '"' + ":" + '{"date":"' + date + '","tasks":["' + tasks.join('","') + '"],"taskIds":[' + taskIds.join(',') + ']}}');
                } else {
                    entries.push('{"' + epoch + '":[' + taskIds.join(',') + ']}');
                }
            }
            
            var jsonText = '{"tasks":["' + items.join('","') + '"],"colors":["' + colors.join('","') + '"],"validTaskIds":[' + validTaskIds.join(',') + '],"epochTasks":[' + entries.join(',') + ']}';
            console.log(jsonText);
            const obj = JSON.parse(jsonText); //just to check if valid JSON
            sendSettings(jsonText);
            //document.getElementById("output").innerHTML = jsonText;
            //            console.log(obj);
        }

        function genCheckboxes(items) {
            var i = 0;
            var text = "<br>Bitte w&auml;hlen Sie die Abfallarten aus,<br>an die Sie erinnert werden wollen:<br>";
            text += "<table>"
            for (let i = 0; i < items.length; i++) {
                text += "<tr>"
                text += "<td class=value><div><input type='checkbox' id='task" + i + "' name=task'" + i + "' checked>";
                text += "<label for='task" + i + "' id='taskl" + i + "'>" + items[i] + "</label><div></td>";
                text += "<td><button style='background-color: " + colors[i].replace("0x","#") + ";border: 2px solid grey;padding: 10px 10px;display: inline-block;'></button></td>";
                text += "</tr>";
            }
            text += "</table>";
            text += "<br><button onclick='genJson()'>Abfuhrtermine speichern</button>";
            text += "<br><div id=output></div>";
            document.getElementById("tasks").innerHTML = text;
            document.getElementById("message").innerHTML = "Es wurden " + Object.keys(dateDict).length + " Abholtermine in der Datei gefunden.";
            document.getElementById("settings").innerHTML = "";
        }
      </script>
    </head>  

   <body id='body' onload='requestSettings()')>
     <img src='https://github.com/tobiwern/TrashReminder/blob/main/pictures/TrashReminder.jpg?raw=true' alt='Trash Reminder' width='400' height='185'>
  <h1>M&uuml;ll-Erinnerung Einstellungen</h1>
  <form name='config'>
    <div class=frame>
      <h2>Zeitpunkt der Erinnerung</h2>
      <table>
        <tr>
          <td class=description><label for="start">Start der Erinnerung<br>(am Vortag der Abholung):</label></td>
          <td class=value><select id="start" name="start"></select></td>
        </tr>
        <tr>
          <td class=description><label class='fancy-input' for="end">Ende der Erinnerung<br>(am Tag der Abholung):</label></td>
          <td class=value><select id="end" name="end"></select></td>
        </tr>
      </table>
      <div id='messageTime'></div>
      <br>
    </div>
    <br>
    </form>
    <div class=frame>
      <h2>Abfuhrtermine (ICS/ICAL)</h2>
      <table>
        <tr><td><label for="start">W&auml;hlen Sie eine oder mehrere bereits heruntergeladene ICS oder ICAL Dateien ihres Entsorgungsunternehmens aus:</label></td></tr>
        <tr><td><input type="file" name="files" id="files" accept=".ics" onchange="processFiles()" multiple></td></tr>
        <tr><td><div id='tasks'></div></td></tr>
        <tr><td><div id='message'></div></td></tr>          
      </table>
      <br>
    </div>    
  <br>
  <div id='settings'></div>
  <div>
    <button class="button" onclick="closeSettings()">Beenden</button>
    <button class="button" onclick="readSettings()">Lesen</button>
    <button class="button" onclick="deleteSettings()">L&ouml;schen</button>
    <button class="button" onclick="send(0)">Erinnerung einschalten</button>
    <button class="button" onclick="fireworks()">Feuerwerk</button>
  </div>
</body>
  </html>
)=====";
