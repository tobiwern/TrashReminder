const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
  <html>
    <head>
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
          width: 350px;
          margin-left: auto;
          margin-right: auto;

        }

        td.value {
          white-space: nowrap;
          vertical-align: center;
          text-align: left;
        }

        td.description {
          vertical-align: center;
          text-align: right;
        }
      </style>
      <script>
    /*    function send(led_sts){
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              document.getElementById("state").innerHTML = this.responseText;
            }
          };
          xhttp.open("GET", "led_set?state="+led_sts, true);
          xhttp.send();
        }
    
        //setInterval(function(){getData();}, 2000);
        */

        function requestSettings() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              value = this.responseText;
              tokens = value.split(",");
              document.getElementById("start").value = tokens[0];
              document.getElementById("end").value = tokens[1];
            }
          };
          xhttp.open("GET", "settings", true);
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
          var value = document.getElementById(dropdown).value;
          var xhttp = new XMLHttpRequest();
          xhttp.open("GET", "set_" + dropdown+ "?value=" + value, true);
          xhttp.send();          
        }
        /// ICS/iCAL Processing
        function processFiles() {
            var dateDict = {};
            var items = [];
            console.log(items);
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
                            var epoch = new Date(date.substring(0, 4) + "-" + date.substring(4, 6) + "-" + date.substring(6, 8)).getTime()
                        } else if (line.search("SUMMARY") != -1) {
                            var item = line.split(":")[1];
                        } else if (line.search("END:VEVENT") != -1) {
                            if (!(epoch in dateDict)) { dateDict[epoch] = []; }
                            var arr = dateDict[epoch];
                            arr.push(item);
                            dateDict[epoch] = arr;
                            if (!items.includes(item)) {
                                items.push(item);
                                genCheckboxes(items);
                            }
                        }
                    }
                };
                reader.readAsText(file);
            }
        }

        function genCheckboxes(items) {
            console.log(items);
            var i = 0;
            var text = "<br>Bitte w&auml;hlen Sie die Abfallarten aus, an die Sie erinnert werden wollen:<br>";
            text += "<table><tr><td class=value>"            
            for (let i = 0; i < items.length; i++) {
                text += "<div><input type='checkbox' id='task" + i + "' name=task'" + i + "' checked>";
                text += "<label for='task" + i + "'>" + items[i] + "</label><div>";
            }
            text += "</td></tr></table>"            
            document.getElementById("tasks").innerHTML = text
        }
      </script>
    </head>  

   <body onload='requestSettings()')>
     <img src='https://github.com/tobiwern/TrashReminder/blob/main/pictures/TrashReminder.jpg?raw=true' alt='Trash Reminder' width='400' height='185'>
  <h1>M&uuml;ll-Erinnerung Einstellungen</h1>
  <form name='config'>
    <div class=frame>
      <h2>Zeitpunkt der Erinnerung</h2>
      <table>
        <tr>
          <td class=description><label for="start">Start der Erinnerung<br>(am Vortag der Abholung):</label></td>
          <td class=value><select id="start" name="start">
              <option value="10">10 Uhr</option>
              <option value="11">11 Uhr</option>
              <option value="12">12 Uhr</option>
              <option value="13">13 Uhr</option>
              <option value="14">14 Uhr</option>
              <option value="15">15 Uhr</option>
              <option value="16">16 Uhr</option>
              <option value="17">17 Uhr</option>
              <option value="18">18 Uhr</option>
              <option value="19">19 Uhr</option>
              <option value="19">20 Uhr</option>
              <option value="19">21 Uhr</option>
              <option value="19">22 Uhr</option>
              <option value="19">23 Uhr</option>
            </select></td>
        </tr>
        <tr>
          <td class=description><label class='fancy-input' for="end">Ende der Erinnerung<br>(am Tag der
              Abholung):</label></td>
          <td class=value><select id="end" name="end">
              <option value="6">6 Uhr</option>
              <option value="7">7 Uhr</option>
              <option value="8">8 Uhr</option>
              <option value="9">9 Uhr</option>
              <option value="10">10 Uhr</option>
              <option value="11">11 Uhr</option>
              <option value="12">12 Uhr</option>
              <option value="13">13 Uhr</option>
              <option value="14">14 Uhr</option>
              <option value="15">15 Uhr</option>
              <option value="16">16 Uhr</option>
              <option value="17">17 Uhr</option>
              <option value="18">18 Uhr</option>
              <option value="19">19 Uhr</option>
            </select></td>
        </tr>
      </table>
      <br>
    </div>
    <br>
    <div class=frame>
      <h2>Abfuhrtermine (ICS/ICAL)</h2>
      <table>
        <tr><td><label for="start">W&auml;hlen Sie eine oder mehrere bereits heruntergeladene ICS oder ICAL Dateien ihres Entsorgungsunternehmens aus:</label></td></tr>
        <tr><td><input type="file" name="files" id="files" accept=".ics" onchange="processFiles()" multiple></td></tr>
        <tr><td><div id='tasks'></div></td></tr>  
      </table>
      <br>
    </div>    
    

  </form>
  <br>
  <div>
    <button class="button" onclick="send(1)">Erinnerung ausschalten</button>
    <button class="button" onclick="send(0)">Erinnerung einschalten</button>
    <button class="button" onclick="send(2)">Feuerwerk</button>
  </div>
</body>
  </html>
)=====";