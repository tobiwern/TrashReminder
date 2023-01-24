//setInterval(function(){getData();}, 2000);
var gMaxNumberOfEpochs;
var gMaxNumberOfTasksPerDay;
var gMaxNumberOfTaskIds;
var gHideDelayDefault = 3;
function fireworks() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            showMessage("I", "FEUERWERK!", "buttonMessage", gHideDelayDefault);
        }
    };
    xhttp.open("GET", "fireworks", true);
    xhttp.send();
}

function closeConfig() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "close", true);
    xhttp.send();
    document.getElementById("body").innerHTML = "<h1>Beendet - Bitte Fenster schließen!</h1>";
    window.scrollTo(0, 0);
    //  window.close(); //close the page
}

function requestSettingsFromESP() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            //populate options
            const dropdowns = ["start", "end"];
            dropdowns.forEach(function (item) {
                for (var i = 0; i <= 23; i++) {
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
            gMaxNumberOfEpochs = tokens[2];
            gMaxNumberOfTasksPerDay = tokens[3]; //tasks per day
            gMaxNumberOfTaskIds = tokens[4]; //different tasks
        }
    };
    xhttp.open("GET", "request_settings", true);
    xhttp.send();
}

function requestTasksFromESP(show = true) { //send the ESP data to the webpage
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4) {
            response = this.responseText;
            if (this.status == 200) {
                refreshTaskTypesAndDates(response);
            } else { //500
                showMessage("W", "Es sind noch keine Abholtermine auf der \"Müll-Erinnerung\" gespeichert! Bitte laden sie wie nachfolgend beschrieben die Abfuhrtermine herunter.", "taskDates");
                if (show) { showMessage("E", "Lesen der Daten fehlgeschlagen!", "buttonMessage", gHideDelayDefault); }
                document.getElementById("taskTypes").innerHTML = "";
            }
        }
    };
    xhttp.open("GET", "request_tasks", true);
    xhttp.send();
}

function sendTasksToESP(jsonText, currentData = false) { //send the jsonText to the ESP to be stored in LittleFS
    if (currentData) {
        receiver = "messageTaskTypes";
        message = "Neue Auswahl gespeichert.";
        hideDelay = 2;
    } else {
        receiver = "message";
        message = "Übertragen der Daten war erfolgreich und Abfuhrtermine werden oben angezeigt.";
        hideDelay = 5;
    }
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4) {
            if (this.status == 200) {
                showMessage("I", message, receiver, hideDelay);
                requestTasksFromESP(); //if storing the values on the ESP was successful => refresh the "current values" on the webpage
            } else { //500
                showMessage("E", "ERROR: Übertragen der Daten fehlgeschlagen!", receiver, gHideDelayDefault);
            }
        }
    };
    xhttp.open("GET", "send_tasks?value=" + jsonText, true);
    xhttp.send();
}

function sendValidTaskTypesToESP() {
    const taskTypeCheckBoxes = document.getElementsByClassName("taskType");
    var validTaskIds = [];
    for (let i = 0; i < taskTypeCheckBoxes.length; i++) {
        checkBox = taskTypeCheckBoxes[i];
        if (checkBox.checked) {
            validTaskIds.push(i);
        }
    }
    if (validTaskIds.length == 0) {
        showMessage("W", "Sie müssen mindestens eine Abfallart auswählen!", "messageTaskTypes");
        return;
    }
    gDataValidTaskIds = validTaskIds; //update in global Setup
    sendCurrentDataToESP(); //send updated data
}

function sendDropDownStateToESP(dropdown) {
    var value = parseInt(document.getElementById(dropdown).value);
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            response = this.responseText;
            showMessage("I", response, "messageTime", gHideDelayDefault);
        }
    };
    if (dropdown == "start") {
        var endValue = parseInt(document.getElementById("end").value)
        if (value < endValue) {
            var newValue = endValue + 1;
            alert("Um überlappende Ereignisse an Folgetagen zu vermeiden, sollte der \"Start der Erinnerung\" (" + value + ") nicht vor dem \"Ende der Erinnerung\" (" + endValue + ") liegen!\nSetze \"Start der Erinnerung\" auf minimal zulässigen Wert (" + newValue + ").");
            value = newValue
            document.getElementById(dropdown).value = value
        }
    } else {
        var startValue = parseInt(document.getElementById("start").value)
        if (startValue < value) {
            newValue = startValue - 1;
            alert("Um überlappende Ereignisse an Folgetagen zu vermeiden, sollte der \"Start der Erinnerung\" (" + startValue + ") nicht vor dem \"Ende der Erinnerung\" (" + value + ") liegen!\nSetze \"Ende der Erinnerung\" auf maximal zulässigen Wert (" + newValue + ").");
            value = newValue;
            document.getElementById(dropdown).value = value
        }
    }
    xhttp.open("GET", "set_" + dropdown + "?value=" + value, true);
    xhttp.send();
}

function deleteTasksOnESP() {
    const response = confirm("Wollen Sie wirklich alle Abfuhrtermine von der \"Müll-Erinnerung\" löschen?");
    if (!response) {
        showMessage("I", "Löschen der Daten abgebrochen.", "buttonMessage", gHideDelayDefault);
        return;
    }
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4) {
            if (this.status == 200) {
                showMessage("I", "Löschen der Daten war erfolgreich!", "buttonMessage", gHideDelayDefault);
                requestTasksFromESP(false); //if deleting the values on the ESP was successful => refresh the "current values" on the webpage
            } else { //500
                showMessage("E", "ERROR: Löschen der Daten fehlgeschlagen!", "buttonMessage", gHideDelayDefault);
            }
        }
    };
    xhttp.open("GET", "delete_tasks", true);
    xhttp.send();
}

/// WebPage Refresh Functions for CURRENT Data on ESP
//globally defined for form field callbacks
var gDataEpochTaskDict = {};
var gDataColors = [];
var gDataTasks = [];
var gDataValidTaskIds = [];

function initDataFromJson(jsonObject) {
    var epochTasks = jsonObject["epochTasks"];
    for (const epochTask of epochTasks) {
        for (var epoch in epochTask) { //translate into dict
            gDataEpochTaskDict[epoch] = epochTask[epoch];
        }
    }
    gDataColors = jsonObject["colors"];
    gDataTasks = jsonObject["tasks"];
    gDataValidTaskIds = jsonObject["validTaskIds"];
}

function sendCurrentDataToESP() { //send currently set data to ESP
    var entries = [];
    var epochs = Object.keys(gDataEpochTaskDict);
    for (epoch of epochs) {
        taskIds = gDataEpochTaskDict[epoch];
        entries.push('{"' + epoch + '":[' + taskIds.join(',') + ']}');
    }
    var jsonText = '{"tasks":["' + gDataTasks.join('","') + '"],"colors":["' + gDataColors.join('","') + '"],"validTaskIds":[' + gDataValidTaskIds.join(',') + '],"epochTasks":[' + entries.join(',') + ']}';
    console.log(jsonText);
    try {
        const obj = JSON.parse(jsonText); //just to check if valid JSON, ToDo: Show if there is an error!
    } catch (e) {
        showMessage("E", "Die Daten sind nicht korrekt als JSON formatiert. Bitte öffnen Sie ein GitHub Issue unter <a href='https://github.com/tobiwern/TrashReminder/issues' target='_blank'>https://github.com/tobiwern/TrashReminder/issues</a>", "message");
        return;
    }
    sendTasksToESP(jsonText, true);
}

function refreshTaskTypesAndDates(response) {
    const jsonObject = JSON.parse(response);
    /*    try {
            let jsonObject = JSON.parse(response);
        } catch (e) {
            showMessage("E", "Die Daten sind nicht korrekt als JSON formatiert. Bitte öffnen Sie ein <a href='https://github.com/tobiwern/TrashReminder/issues' target='_blank'>GitHub Issue</a>.<br>ERROR: " + e, "messageTaskTypes");
            document.getElementById("taskDates").innerHTML = response;
            return;
        }
    */
    document.getElementById("taskDates").style.color = "black";
    gDataEpochTaskDict = {}; //reset
    initDataFromJson(jsonObject);
    refreshTaskTypes();
    refreshTaskDates();
}

function refreshTaskTypes() {
    var text = "Sie werden derzeit an folgende Abfallarten erinnert:<br><br>";
    text += "<table>";
    for (let i = 0; i < gDataTasks.length; i++) {
        checked = (gDataValidTaskIds.length == 0 || gDataValidTaskIds.includes(i)) ? "checked" : "";
        text += "<tr>"
        text += "<td class='value'><div><input type='checkbox' class='taskType' onChange='refreshTaskDates();sendValidTaskTypesToESP();' id='taskType" + i + "' name=task" + i + "' " + checked + ">";
        text += "<label for='taskType" + i + "' id='taskTypel" + i + "'> " + gDataTasks[i] + "</label><div></td>";
        text += "<td><button style='background-color: " + gDataColors[i].replace("0x", "#") + ";border: 2px solid grey;padding: 10px 10px;display: inline-block;'></button></td>";
        text += "</tr>";
    }
    text += "</table>";
    document.getElementById("taskTypes").innerHTML = text + "<br>";
}

function refreshTaskDates() { //show TaskDates on Webpage
    var text = Object.keys(gDataEpochTaskDict).length + " Abholtermine sind derzeit verfügbar.<br><br>";
    var epochs = Object.keys(gDataEpochTaskDict).sort();
    text += "<table id=epochTasks>"
    text += "<tr><th>Datum der Abholung</th><th>Müllart</th></tr>"
    const taskTypeCheckBoxes = document.getElementsByClassName("taskType");
    taskIdEnableValue = [];
    for (checkBox of taskTypeCheckBoxes) {
        taskIdEnableValue.push(checkBox.checked);
    }
    for (const epoch of epochs) {
        var dateTime = new Date(epoch * 1000);
        var taskIds = gDataEpochTaskDict[epoch];
        selectedTaskIds = [];
        for (const taskId of taskIds) {
            if (taskIdEnableValue[taskId]) {
                selectedTaskIds.push(taskId);
            }
        }
        if (selectedTaskIds.length >= 1) {
            text += "<tr>"
            text += "<td class=description nowrap>" + epochToDateString(epoch) + "</td>";
            text += "<td>";
            for (const taskId of selectedTaskIds) {
                text += "<div class=taskType><button style='background-color: " + gDataColors[taskId].replace("0x", "#") + ";border: 2px solid grey;padding: 10px 10px;display: inline-block;'></button>";
                text += " " + gDataTasks[taskId] + "</div>";
            }
            text += "</td>";
            text += "</tr>";
        }
    }
    text += "</table>";
    document.getElementById("taskDates").innerHTML = text + "<br>";
}

document.addEventListener('DOMContentLoaded', function () {
    enableEventListener('start');
    enableEventListener('end');
});
function enableEventListener(dropdown) {
    document.getElementById(dropdown).addEventListener('change', function () { sendDropDownStateToESP(dropdown); });
}
/// ICS/iCAL Processing ////////////////////////////////////////////////////////////////////
var gDebug = false;
var gTasks = [];
var gEpochTaskDict = {};                       //HÄCKSEL
var gColorDict = { 'PAPIER': '0x0000FF', 'BIO,CKSEL': '0x00FF00', 'GELB,WERT': '0xFFFF00', 'REST': '0xFFFFFF' }
var gColorDefault = '0xFFC0CB';
var gColors = [];
function processFiles() {
    gTasks = [];
    gEpochTaskDict = {};
    var files = document.getElementById('files').files;
    for (var fileIndex = 0; fileIndex < files.length; fileIndex++) {
        var file = files[fileIndex];
        var reader = new FileReader();
        reader.onload = function (progressEvent) {
            const text = this.result; //entire file
            var lines = text.split('\n');
            for (const line of lines) {
                if (line.search("DTSTART") != -1) {
                    var date = line.split(":")[1];
                    dateString = date.substring(0, 4) + "-" + date.substring(4, 6) + "-" + date.substring(6, 8);
                    var epoch = new Date(dateString).getTime() / 1000; //since ms => s
                } else if (line.search("SUMMARY") != -1) {
                    var task = line.split(":")[1];
                    task = task.replace("\\", "");
                    task = task.replace("\r", "");
                } else if (line.search("END:VEVENT") != -1) {
                    if (!(epoch in gEpochTaskDict)) { gEpochTaskDict[epoch] = { "tasks": [], "date": dateString }; }
                    var arr = gEpochTaskDict[epoch]["tasks"];
                    arr.push(task);
                    gEpochTaskDict[epoch][tasks] = arr;
                    if (!gTasks.includes(task)) {
                        gTasks.push(task);
                        gColors = getColors();
                    }
                }
            }
            showCheckBoxes(gTasks); //executed multiple times, however ok
            checkMaxNumberOfEntries();
        }; //on load
        reader.readAsText(file);
    }
}

function getValidTaskIds() {
    var validTaskIds = [];
    for (var i = 0; i < gTasks.length; i++) {
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
        taskIds.push(gTasks.indexOf(tasks[i]));
    }
    return (taskIds);
}

function getColors() {
    var colors = [];
    for (const task of gTasks) {
        var color = getMatchingColor(task);
        if (color) {
            colors.push(color);
        } else {
            alert("Failed to auto-assign color for entry " + task + ".<br>Assigning default color (pink).");
            colors.push(gColorDefault);
        }
    }
    return (colors);
}

function getMatchingColor(task) {
    var keys = Object.keys(gColorDict).sort();
    for (const key of keys) {
        var entries = key.split(",");
        for (entry of entries) {
            if (task.toUpperCase().search(entry) != -1) {
                return (gColorDict[key]);
            }
        }
    }
    return (false);
}

function genJson() {
    validTaskIds = getValidTaskIds();
    console.log(validTaskIds);
    var entries = [];
    var epochs = Object.keys(gEpochTaskDict);
    for (epoch of epochs) {
        var tasks = gEpochTaskDict[epoch]["tasks"];
        var taskIds = getTaskIds(tasks);
        var date = gEpochTaskDict[epoch]["date"];
        if (gDebug) {
            entries.push('{"' + epoch + '"' + ":" + '{"date":"' + date + '","tasks":["' + tasks.join('","') + '"],"taskIds":[' + taskIds.join(',') + ']}}');
        } else {
            entries.push('{"' + epoch + '":[' + taskIds.join(',') + ']}');
        }
    }

    var jsonText = '{"tasks":["' + gTasks.join('","') + '"],"colors":["' + gColors.join('","') + '"],"validTaskIds":[' + validTaskIds.join(',') + '],"epochTasks":[' + entries.join(',') + ']}';
    console.log(jsonText);
    try {
        const obj = JSON.parse(jsonText); //just to check if valid JSON, ToDo: Show if there is an error!
    } catch (e) {
        showMessage("E", "Die Daten sind nicht korrekt als JSON formatiert. Bitte öffnen Sie ein GitHub Issue unter <a href='https://github.com/tobiwern/TrashReminder/issues' target='_blank'>https://github.com/tobiwern/TrashReminder/issues</a>", "message");
        return;
    }
    sendTasksToESP(jsonText);
}

function checkMaxNumberOfEntries() {
    var text = "";
    if (gTasks.length > gMaxNumberOfTaskIds) {
        text += "Anzahl der Abfallarten ist " + gTasks.length + ". Es werden maximal " + gMaxNumberOfTaskIds + " <b>unterschiedliche Abfallarten</b> unterstützt!<br>";
    }
    var epochs = Object.keys(gEpochTaskDict);
    if (epochs.length > gMaxNumberOfEpochs) {
        text += "Anzahl der Abholtermine ist " + epochs.length + ". Es werden maximal " + gMaxNumberOfEpochs + " <b>Abholtermine</b> unterstützt!<br>";
    }
    for (const epoch of epochs) {
        var tasks = gEpochTaskDict[epoch]["tasks"];
        var taskIds = getTaskIds(tasks);
        if (taskIds.length > gMaxNumberOfTasksPerDay) {
            text += "Anzahl der Abfallarten pro Tag am " + epochToDateString(epoch, "short") + " ist " + taskIds.length + ". Es werden maximal " + gMaxNumberOfTasksPerDay + " unterschiedliche <b>Abfallarten pro Tag</b> unterstützt!<br>";
        }
    }
    if (text != "") {
        text += "Die darüber hinausgehenden Einträge werden nicht verarbeitet.<br>Bitte öffnen Sie ein <a href='https://github.com/tobiwern/TrashReminder/issues' target='_blank'>GitHub Issue</a>!";
        showMessage("W", text, "message");
    }
}

function showCheckBoxes(items) {
    var i = 0;
    var text = "<i>Es wurden " + Object.keys(gEpochTaskDict).length + " Abholtermine in ";
    if (document.getElementById('files').files.length > 1) {
        text += "den Dateien gefunden.</i>";
    } else {
        text += "der Datei gefunden.</i>";
    }
    text += "<br><br>";
    text += "Bitte w&auml;hlen Sie die Abfallarten aus,<br>an die Sie erinnert werden wollen:<br><br>";
    text += genCheckBoxes(items, gColors);
    text += "<br><button onclick='genJson()'>Abfuhrtermine speichern</button><br><br>";
    document.getElementById("tasks").innerHTML = text;
    document.getElementById("message").innerHTML = "";
}

function genCheckBoxes(tasks, colors, validTaskIds = []) {
    var text = "<table>";
    for (let i = 0; i < tasks.length; i++) {
        checked = (validTaskIds.length == 0 || validTaskIds.includes(i)) ? "checked" : "";
        text += "<tr>"
        text += "<td class=value><div><input type='checkbox' id='task" + i + "' name=task" + i + "' " + checked + ">";
        text += "<label for='task" + i + "' id='taskl" + i + "'> " + tasks[i] + "</label><div></td>";
        text += "<td><button style='background-color: " + colors[i].replace("0x", "#") + ";border: 2px solid grey;padding: 10px 10px;display: inline-block;'></button></td>";
        text += "</tr>";
    }
    text += "</table>";
    return (text);
}

function send(number) {//debug
    showMessage("E", "Die Daten sind nicht korrekt als JSON formatiert. Bitte öffnen Sie ein GitHub Issue unter <a href='https://github.com/tobiwern/TrashReminder/issues' target='_blank'>https://github.com/tobiwern/TrashReminder/issues</a>", "message");
}

/// Utility Functions
function epochToDateString(epoch, dateType = "long") {
    var dateTime = new Date(epoch * 1000);
    var timeStamp = "";
    if (dateType == "long") {
        timeStamp += dateTime.toLocaleString("de", { weekday: "long" }) + ", ";
    }
    timeStamp += ("00" + dateTime.getDate()).slice(-2) + "." + ("00" + dateTime.toLocaleString("de", { month: "numeric" })).slice(-2) + "." + dateTime.getFullYear();
    return (timeStamp);
}

let timeoutID;
function showMessage(msgType, message, receiver = "buttonMessage", hideDelayInSec = 0) {
    document.getElementById(receiver).innerHTML = message + "<br><br>";
    switch (msgType) {
        case "D":
            document.getElementById(receiver).style.color = "orange";
            break;
        case "W":
            document.getElementById(receiver).style.color = "orange";
            break;
        case "E":
            document.getElementById(receiver).style.color = "red";
            break;
        case "I":
            document.getElementById(receiver).style.color = "green";
            break;
        default:
            document.getElementById(receiver).style.color = "black";
    }
    if (hideDelayInSec != 0) {
        timeoutId = setTimeout(function () { document.getElementById(receiver).innerHTML = ""; }, hideDelayInSec * 1000);
    } else {
        clearTimeout(timeoutID);
    }
}
