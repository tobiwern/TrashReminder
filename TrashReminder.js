//setInterval(function(){getData();}, 2000);
var maxNumberOfEpochs;
var maxNumberOfTasksPerDay;
var maxNumberOfTaskIds;
function fireworks() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("message").innerHTML = this.responseText;
            setTimeout(function () { document.getElementById("message").innerHTML = ""; }, 2000);
        }
    };
    document.getElementById("settings").innerHTML = "";
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

function requestSettings() {
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
            maxNumberOfEpochs = tokens[2];
            maxNumberOfTasksPerDay = tokens[3];
            maxNumberOfTaskIds = tokens[4];
        }
    };
    xhttp.open("GET", "request_settings", true);
    xhttp.send();
}

function sendData(jsonText) { //send the jsonText to the ESP to be stored in LittleFS
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            response = this.responseText;
            document.getElementById("message").innerHTML = response;
            if (response.search("ERROR") != -1) {
                document.getElementById("message").style.color = "red";
            } else {
                document.getElementById("message").style.color = "green";
            }
        }
    };
    xhttp.open("GET", "send_tasks?value=" + jsonText, true);
    xhttp.send();
}

function requestTasks() { //send the ESP data to the webpage
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            response = this.responseText;
            if (response.search("ERROR") != -1) {
                document.getElementById("message").innerHTML = response;
                document.getElementById("message").style.color = "red";
                document.getElementById("settings").innerHTML = "";
            } else {
                refreshTaskTypesAndDates(response);
            }
        }
    };
    xhttp.open("GET", "request_tasks", true);
    xhttp.send();
}

function refreshTaskTypesAndDates(response) {
    const jsonObject = JSON.parse(response);
    initDataFromJson(jsonObject)
    refreshTaskTypes();
    refreshTaskDates();
    //    document.getElementById("tasks").innerHTML = response + "<br>";
}

function refreshTaskTypes() {
    var text = "Sie werden derzeit an folgende Abfallarten erinnert:<br><br>";
    text += "<table>";
    for (let i = 0; i < dataTasks.length; i++) {
        checked = (dataValidTaskIds.length == 0 || dataValidTaskIds.includes(i)) ? "checked" : "";
        text += "<tr>"
        text += "<td class='value'><div><input type='checkbox' class='taskType' onChange='refreshTaskDates()' id='taskType" + i + "' name=task" + i + "' " + checked + ">";
        text += "<label for='taskType" + i + "' id='taskTypel" + i + "'> " + dataTasks[i] + "</label><div></td>";
        text += "<td><button style='background-color: " + dataColors[i].replace("0x", "#") + ";border: 2px solid grey;padding: 10px 10px;display: inline-block;'></button></td>";
        text += "</tr>";
    }
    text += "</table>";
    document.getElementById("taskTypes").innerHTML = text + "<br>";
    //send updates to ESP
    const taskTypeCheckBoxes = document.getElementsByClassName("taskType");
    var validTaskIds = [];
    for (let i = 0; i < taskTypeCheckBoxes.length; i++) {
        checkBox = taskTypeCheckBoxes[i];
        if (checkBox.checked) {
            validTaskIds.push(i);
        }
    }
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            response = this.responseText;
            if (response.search("ERROR") != -1) {
                document.getElementById("message").innerHTML = response;
                document.getElementById("message").style.color = "red";
                document.getElementById("settings").innerHTML = "";
            } else {
                document.getElementById("message").innerHTML = response;
                document.getElementById("message").style.color = "green";
            }
        }
    };
    var jsonText = '{"validTaskIds:[' + validTaskIds.join(',') + ']}';
    xhttp.open("GET", "send_validTaskIds?value=" + jsonText, true);
    xhttp.send();
}

function refreshTaskDates() {
    var text = Object.keys(dataEpochTaskDict).length + " Abholtermine sind derzeit verfügbar.<br><br>";
    var epochs = Object.keys(dataEpochTaskDict).sort();
    text += "<table id=epochTasks>"
    text += "<tr><th>Datum der Abholung</th><th>Müllart</th></tr>"
    const taskTypeCheckBoxes = document.getElementsByClassName("taskType");
    taskIdEnableValue = [];
    for (checkBox of taskTypeCheckBoxes) {
        taskIdEnableValue.push(checkBox.checked);
    }
    for (const epoch of epochs) {
        var dateTime = new Date(epoch * 1000);
        var taskIds = dataEpochTaskDict[epoch];
        selectedTaskIds = [];
        for (const taskId of taskIds) {
            if (taskIdEnableValue[taskId]) {
                selectedTaskIds.push(taskId);
            }
        }
        if (selectedTaskIds.length >= 1) {
            timeStamp = dateTime.toLocaleString("de", { weekday: "long" }) + ", " + ("00" + dateTime.getDate()).slice(-2) + "." + ("00" + dateTime.toLocaleString("de", { month: "numeric" })).slice(-2) + "." + dateTime.getFullYear();
            text += "<tr>"
            text += "<td class=description nowrap>" + timeStamp + "</td>";
            text += "<td>";
            for (const taskId of selectedTaskIds) {
                text += "<div class=taskType><button style='background-color: " + dataColors[taskId].replace("0x", "#") + ";border: 2px solid grey;padding: 10px 10px;display: inline-block;'></button>";
                text += " " + dataTasks[taskId] + "</div>";
            }
            text += "</td>";
            text += "</tr>";
        }
    }
    text += "</table>";
    document.getElementById("taskDates").innerHTML = text + "<br>";
}

//globally defined for form field callbacks
dataEpochTaskDict = {};
dataColors = [];
dataTasks = [];
dataValidTaskIds = [];

function initDataFromJson(jsonObject) {
    var epochTasks = jsonObject["epochTasks"];
    for (const epochTask of epochTasks) {
        for (var epoch in epochTask) { //translate into dict
            dataEpochTaskDict[epoch] = epochTask[epoch];
        }
    }
    dataColors = jsonObject["colors"];
    dataTasks = jsonObject["tasks"];
    dataValidTaskIds = jsonObject["validTaskIds"];
}

function deleteTasks() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            response = this.responseText;
            document.getElementById("message").innerHTML = response;
            if (response.search("ERROR") != -1) {
                document.getElementById("message").style.color = "red";
            } else {
                document.getElementById("message").style.color = "green";
                document.getElementById("settings").innerHTML = "";
            }
        }
    };
    xhttp.open("GET", "delete_tasks", true);
    xhttp.send();
}

document.addEventListener('DOMContentLoaded', function () {
    enableEventListener('start');
    enableEventListener('end');
});
function enableEventListener(dropdown) {
    document.getElementById(dropdown).addEventListener('change', function () { sendUpdate(dropdown); });
}
function sendUpdate(dropdown) {
    var value = parseInt(document.getElementById(dropdown).value);
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            response = this.responseText;
            document.getElementById("messageTime").innerHTML = "<br>" + response;
            document.getElementById("messageTime").style.color = "green";
            setTimeout(function () { document.getElementById("messageTime").innerHTML = ""; }, 2000);
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
                    var epoch = new Date(dateString).getTime() / 1000; //since ms => s
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
                        showCheckBoxes(items); //executed multiple times, however ok
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
    sendData(jsonText);
    //document.getElementById("output").innerHTML = jsonText;
    //            console.log(obj);
}

function showCheckBoxes(items) {
    var i = 0;
    var text = "<br><i>Es wurden " + Object.keys(dateDict).length + " Abholtermine in ";
    if (document.getElementById('files').files.length > 1) {
        text += "den Dateien gefunden.</i>";
    } else {
        text += "der Datei gefunden.</i>";
    }
    text += "<br><br>";
    text += "Bitte w&auml;hlen Sie die Abfallarten aus,<br>an die Sie erinnert werden wollen:<br><br>";
    text += genCheckBoxes(items, colors);
    text += "<br><button onclick='genJson()'>Abfuhrtermine speichern</button>";
    text += "<br><div id=output></div>";
    document.getElementById("tasks").innerHTML = text;
    document.getElementById("message").innerHTML = "";
    document.getElementById("settings").innerHTML = "";
}

function genCheckBoxes(items, colors, validTaskIds = []) {
    var text = "<table>";
    for (let i = 0; i < items.length; i++) {
        checked = (validTaskIds.length == 0 || validTaskIds.includes(i)) ? "checked" : "";
        text += "<tr>"
        text += "<td class=value><div><input type='checkbox' id='task" + i + "' name=task" + i + "' " + checked + ">";
        text += "<label for='task" + i + "' id='taskl" + i + "'> " + items[i] + "</label><div></td>";
        text += "<td><button style='background-color: " + colors[i].replace("0x", "#") + ";border: 2px solid grey;padding: 10px 10px;display: inline-block;'></button></td>";
        text += "</tr>";
    }
    text += "</table>";
    return (text);
}