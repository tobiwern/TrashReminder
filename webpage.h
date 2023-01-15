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
    <body id='body' onload='requestSettingsFromESP();requestTasksFromESP();')>
    <img src='https://github.com/tobiwern/TrashReminder/blob/main/pictures/TrashReminder.jpg?raw=true'
        alt='Trash Reminder' width='400' height='185'>
    <h1>M&uuml;ll-Erinnerung Einstellungen</h1>
    <form name='config'>
        <div class=frame>
            <h2><div class='centeredHeight'><img src='https://github.com/tobiwern/TrashReminder/blob/main/pictures/clock.svg?raw=true'> Zeitpunkt der Erinnerung</div></h2>
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
            <br>
            <div id='messageTime'></div>
        </div>
        <br>
    </form>
    <div class=frame>
      <h2><div class='centeredHeight'><img src='https://github.com/tobiwern/TrashReminder/blob/main/pictures/truck.svg?raw=true'> Abfuhrtermine</div></h2>
      <h3><div class='centeredHeight'><img src='https://github.com/tobiwern/TrashReminder/blob/main/pictures/trash.svg?raw=true'> Abfallarten</div></h3>
      <div id='taskTypes'></div>
      <h3><div class='centeredHeight'><img src='https://github.com/tobiwern/TrashReminder/blob/main/pictures/watch.svg?raw=true'> Termine</div></h3>
      <div id='taskDates'></div>
      <h3><div class='centeredHeight'><img src='https://github.com/tobiwern/TrashReminder/blob/main/pictures/download.svg?raw=true'> Neue Abfuhrtermine (ICS/ICAL)</div></h3>
      <p>Falls sich Änderungen an den Abfuhrterminen ergeben haben oder Termine für das nächste Jahr gespeichert werden sollen, könnnen neue Abfuhrtermine auf die "Müll-Erinnerung" geladen werden. Hierbei werden die bestehenden Daten <b>überschrieben</b>!</p>
      <p>Die Abfuhrdaten werden üblicherweise durch das Entsorgungsunternehmen auf einer Webseite im ICS oder ICAL Format
      angeboten und müssen zuerst heruntergeladen werden.</p>
      <p>Beispiele für Unternehmen, bei denen ICS Dateien heruntergeladen werden können:
      <ul>
          <li><a href='https://www.abfall-kreis-tuebingen.de/online-abfuhrtermine/' target='_blank'>https://www.abfall-kreis-tuebingen.de/online-abfuhrtermine/</a></li>
          <li><a href='https://www.bogenschuetz-entsorgung.de/blaue-tonne-tuebingen/abfuhrtermine.html'  target='_blank'>https://www.bogenschuetz-entsorgung.de/blaue-tonne-tuebingen/abfuhrtermine.html</a></li>
      </ul>
      </p>
      <p>Sobald sie die ICS oder ICAL Datei auf Ihr Handy oder ihren Computer heruntergeladen haben, können Sie diese über den untenstehenden Button "Durchsuchen..." auswählen und auf die "Müll-Erinnerung" laden. 
          Es können auch mehrere Dateien ausgewählt werden, falls mehrere Unternehmen die Abfuhr übernehmen.</p>
      <hr>
      <table>
          <tr>
              <td><label for="start">Wählen Sie eine oder mehrere bereits heruntergeladene ICS oder ICAL Dateien ihres Entsorgungsunternehmens aus:</label><br><br></td>
          </tr>
          <tr>
              <td><input type="file" name="files" id="files" accept=".ics" onchange="processFiles()" multiple><br><br></td>
          </tr>
          <tr>
              <td>
                  <div id='tasks'></div>
              </td>
          </tr>
          <tr>
              <td>
                  <div id='message'></div>
              </td>
          </tr>
      </table>
      </div>
      <br>
      <div class=frame>
          <h2><div class='centeredHeight'><img src='https://github.com/tobiwern/TrashReminder/blob/main/pictures/github.svg?raw=true'> Bedienungsanleitung</div></h2>
          Mehr Infos zur "Müll-Erinnerung gibt es unter <a href='https://tobiwern.github.io/TrashReminder/' target='_blank'>https://tobiwern.github.io/TrashReminder/</a>
          <br><br>
      </div>
      <br>
      <div id='buttonMessage'></div>
      <div>
        <button class="button" onclick="closeConfig()">Beenden</button>
        <button class="button" onclick="requestTasksFromESP()">Lesen</button>
        <button class="button" onclick="deleteTasks()">L&ouml;schen</button>
        <button class="button" onclick="send(0)">Erinnerung einschalten</button>
        <button class="button" onclick="fireworks()">Feuerwerk</button>
      </div>
    </body>
  </html>
)=====";
