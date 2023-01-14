//don't add taskId if the number is already in the list (if usr selects two ics files with identical content)
const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
  <html lang="de">
    <head>
      <meta charset='utf-8'>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Müll Erinnerung</title>
      <meta name="robots" content="noindex">
      <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate" />
      <meta http-equiv="Pragma" content="no-cache" />
      <meta http-equiv="Expires" content="0" />
      <link rel="stylesheet" href="https://tobiwern.github.io/TrashReminder/TrashReminder.css">
      <script src="https://tobiwern.github.io/TrashReminder/TrashReminder.js?random=<?php echo filemtime('https://tobiwern.github.io/TrashReminder/TrashReminder.js'); ?>"></script> 
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
        <div id='existingTasks'></div>
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
    <button class="button" onclick="closeConfig()">Beenden</button>
    <button class="button" onclick="requestTasks()">Lesen</button>
    <button class="button" onclick="deleteTasks()">L&ouml;schen</button>
    <button class="button" onclick="send(0)">Erinnerung einschalten</button>
    <button class="button" onclick="fireworks()">Feuerwerk</button>
  </div>
</body>
  </html>
)=====";
