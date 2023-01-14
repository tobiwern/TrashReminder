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
        <div id='messageTime'></div>
        <br>
      </div>
      <br>
    </form>
    <div class=frame>
      <h2>Abfuhrtermine</h2>
      <h3>Abfallarten</h3>
      <div id='taskTypes'></div>
      <h3>Termine</h3>
      <div id='taskDates'></div>
      <h3>Neue Abfuhrtermine (ICS/ICAL)</h3>
      Sie könnnen neue Abfuhrtermine auf die \"Müll-Erinnerung\" laden, falls sich Änderungen ergeben haben oder um diese für ein neues Jahr zu definieren.
      Hierbei werden die bestehenden Daten <b>überschrieben</b>! 
      Die Abfuhrdaten werden üblicherweise durch das Abfuhrunternehmen auf einer Webseite im ICS oder ICAL Format angeboten und müssen zuerst heruntergeladen werden.
      Es können auch mehrere Dateien ausgewählt werden, falls mehrere Unternehmen die Abfuhr übernehmen.
      Beispiele für Unternehmen, bei denen ICS Dateien heruntergeladen werden können
      <ul>
        <li><a href='https://www.abfall-kreis-tuebingen.de/online-abfuhrtermine/'>https://www.abfall-kreis-tuebingen.de/online-abfuhrtermine/</a>
        <li><a href='https://www.bogenschuetz-entsorgung.de/blaue-tonne-tuebingen/abfuhrtermine.html'>https://www.bogenschuetz-entsorgung.de/blaue-tonne-tuebingen/abfuhrtermine.html</a>
      </ul>

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
