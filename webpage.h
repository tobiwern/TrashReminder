// don't add taskId if the number is already in the list (if usr selects two ics files with identical content)
const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
  <html lang="de">
    <head>
      <meta charset='utf-8'>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Müll-Erinnerung</title>
      <meta name="robots" content="noindex">
      <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate" />
      <meta http-equiv="Pragma" content="no-cache" />
      <meta http-equiv="Expires" content="0" />
      <script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
      <link rel="stylesheet" href="https://tobiwern.github.io/TrashReminder/colorPicker/colorPick.css">
      <script src="https://tobiwern.github.io/TrashReminder/colorPicker/colorPick.js"></script>
      <link rel="stylesheet" href="https://tobiwern.github.io/TrashReminder/TrashReminder.css">
      <script src="https://tobiwern.github.io/TrashReminder/TrashReminder.js">"></script> 
    </head>  
    <body id='body' onload='createWebpage();requestSettingsFromESP();requestTasksFromESP(false);')>    
    </body>
  </html>
)=====";
