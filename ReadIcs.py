#coding=utf-8
from icalendar import Calendar, Event, vDatetime
import datetime
from pytz import UTC # timezone

file = "C:/Users/tobiw/Downloads/Abfuhrkalender-Hirrlingen-2023.ics"
file = "C:/Users/tobiw/Downloads/allestrassenweilimschoenbuch.ics"
file = "C:/Users/tobiw/Downloads/allestrassenebhausen.ics"
#validEntries = ['Altpapier-Tonne in Hirrlingen', 'Bioabfall in Hirrlingen', 'Gelber Sack in Hirrlingen', 'Häckselgut in Hirrlingen', 'Restmüll in Hirrlingen']
#validEntries = ['Altpapier-Tonne in Hirrlingen', 'Gelber Sack in Hirrlingen', 'Häckselgut in Hirrlingen', 'Restmüll in Hirrlingen']
validEntries = []
colors = ['gelb', 'grün', 'weiß']
dateDict = {}
items = set()
with open(file,'rb') as g:
    gcal = Calendar.from_ical(g.read())
    for component in gcal.walk():
    #   print(component)
        if component.name == "VEVENT":
            date = str(component.get('dtstart').dt)
            print(date)
            epoche = int((component.get('dtstart').dt-datetime.date(1970,1,1)).total_seconds())
            print(epoche)
            #date1 = datetime.date(1970,1,1)
            item = str(component.get('summary'))
            if((not validEntries) or (item in validEntries)):
                items.add(item)
                dateDict.setdefault(epoche, []).append(item) 
print(dateDict)
#for entry in dateDict.values:
#    if(len(entry)>1):
#        print(entry)

items = list(sorted(items))
#print("std::unordered_map<int,int>epochTaskDict = {")
print("#include <unordered_map>")
print("std::unordered_map<int,String>epochTaskDict = {")
for entry in dateDict:
    values = []
    for item in dateDict[entry]:
        i = items.index(item)
        values.append(str(i))
#    print("{{{},{}}},".format(entry,",".join(values)))
    print("{{{},\"{}\"}},".format(entry,",".join(values)))
print("};")
print("const String task[] = {{\"{}\"}};".format("\", \"".join(items)))

print(len(dateDict))