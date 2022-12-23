# Müll-Erinnerung ("Trash Reminder")

**Schon mal vergessen den Mülleimer rauszustellen?**

`Müll-Erinnerung` erinnert Dich daran, indem es eine farbig blinkende Erinnerung anzeigt.

![bild](./pictures/TrashReminder.jpg)

Je nach Müllart blinkt das Mülleimermodell in einer unterschiedlichen Farbe:
- `weiß`: Restmüll
- `blau`: Papier
- `grün`: Biomüll
- `gelb`: Gelber Sack oder Wertstofftonne

Wird also zum Beispiel `Restmüll` abgeholt, blinkt das Mülleimermodell `weiß`.

Sollten am selben Tag **zwei** unterschiedliche Müllarten abgeholt werden, wechselt das Licht zwischen den entsprechenden Farben hin und her. Also zum Beispiel zwischen `weiß` und `grün`, wenn sowohl `Restmüll` als auch `Biomüll` am selben Tag abgeholt werden.

## Zeitpunkt der Erinnerung
Die Erinnerung erfolgt am **Vortag von 15 Uhr bis 8 Uhr morgens**. 

## Ausschalten der Erinnerung
Wenn der Müll rausgestellt wurde, kann die Erinnerung ausgeschaltet werden indem das **Mülleimermodell kurz angehoben** wird. Damit erlischt das Licht bis zum nächsten Abholungstermin.

Wurde das Mülleimermodell versehentlich angehoben (aber der Müll nicht rausgestellt) kann man den Stecker der Stromversorgung einfach kurz aus- und wieder einstecken. Liegt eine Erinnerung vor und ist man im Erinnerungszeitraum (15 - 8 Uhr) blinkt das Mülleimermodell wieder wie zuvor.  

## Einrichten der `Müll-Erinnerung`
Da das aktuelle Datum und Uhrzeit aus dem Internet ermittelt werden, muss die `Müll-Erinnerung` mit dem WLAN verbunden werden.

Wenn die `Müll-Erinnerung` zum ersten mal ans Netzteil angeschlossen wird, leuchetet das Mülleimermodell dauerhaft `rot` um mitzuteilen, dass die einmalige Einrichtung durchgeführt werden muss.

Hierzu muss man sich über das Smartphone mit dem WLAN Netzwerk `TrashReminder` verbinden und im folgenden Dialog
- Heimnetzwerk (SSID) auswählen und das
- Passwort für dieses Netzwerk eingeben.

Jetzt verbindet sich die `Müll-Erinnerung` mit dem ausgewählten Netzwerk und quittiert die erfolgreiche Verbindung mit einem `Feuerwerk` im Mülleimermodell.

Diese Einrichtung muss nur einmalig durchgeführt werden - von nun an holt sich die `Müll-Erinnerung` die aktuelle Uhrzeit aus dem Internet und gleicht diese mit eventuell vorliegenden Müllterminen ab, welche dann durch farbiges Blinken des Mülleimermodells mitgeteilt wird.

## Unterbrechung der WLAN Verbindung
Sollte es zu einer Verbindungsstörung zum Internet kommen, signalisiert die `Müll-Erinnerung` dies durch ein schnelles rotes Blinken. Sie müssen nichts weiter unternehmen - sobald die Verbindung wieder hergestellt werden kann wechselt die `Müll-Erinnerung` in den regulären Modus.

## Gültigkeit der Abholtermine
Die eingespeicherten Termine gelten nur für das Jahr 2023 und sind für einen bestimmten Ort (z.B. Hirrlingen) vorprogrammiert.
Ob die Termine auch für das Jahr 2024 bereitgestellt werden steht noch in den Sternen, aber man braucht ja auch nächstes Jahr wieder ein Weihnachtsgeschenk ;-)

Schöne Weihnachten und viel Spaß mit der `Müll-Erinnerung`!

## Easter-Egg
Findest Du die versteckte Funktion?

## Autor

Tobias Werner, Erfindungen aller Art
