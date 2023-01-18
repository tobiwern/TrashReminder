#coding=utf-8
from icalendar import Calendar, Event, vDatetime
import datetime
from pytz import UTC # timezone
import re


#files = ["C:/Users/tobiw/Downloads/Abfuhrkalender-Hirrlingen-2023.ics","C:/Users/tobiw/Downloads/Abfuhrkalender-Hirrlingen-2022.ics", "C:/Users/tobiw/Downloads/Abfallkalender Hirrlingen.ics"]
#files = ["C:/Users/tobiw/Downloads/allestrassenweilimschoenbuch.ics"]
#files = ["C:/Users/tobiw/Downloads/stuttgart-gruenewaldstr25.ics"]
files = ["C:/Users/tobiw/Downloads/panoramastrboeblingen.ics"]
#files = ["C:/Users/tobiw/Downloads/allestrassenebhausen.ics"]
#validEntries = ['Altpapier-Tonne in Hirrlingen', 'Bioabfall in Hirrlingen', 'Gelber Sack in Hirrlingen', 'Häckselgut in Hirrlingen', 'Restmüll in Hirrlingen']
#validEntries = ['Altpapier-Tonne in Hirrlingen', 'Gelber Sack in Hirrlingen', 'Häckselgut in Hirrlingen', 'Restmüll in Hirrlingen']
validEntries = []
colorDict = {'PAPIER':'0x0000FF', 'BIO,HÄCKSEL':'0x00FF00', 'GELB,WERT':'0xFFFF00', 'REST':'0xFFFFFF'}
dateDict = {}
items = set()
for file in files:
    with open(file,'rb') as g:
        gcal = Calendar.from_ical(g.read())
        for component in gcal.walk():
        #   print(component)
            if component.name == "VEVENT":
                date = str(component.get('dtstart').dt)[0:10]
                tokens = date.split("-")
             #   epoche = int((component.get('dtstart').dt-datetime.date(1970,1,1)).total_seconds())
                epoche = int((datetime.date(int(tokens[0]),int(tokens[1]),int(tokens[2]))-datetime.date(1970,1,1)).total_seconds())
                #print(epoche)
                #date1 = datetime.date(1970,1,1)
                item = str(component.get('summary'))
                if((not validEntries) or (item in validEntries)):
                    items.add(item)
                    dateDict.setdefault(epoche, []).append(item) 
#print(dateDict)

items = list(sorted(items))
for file in files:
    print("//{}".format(file))

print("#include <unordered_map>")
print("std::unordered_map<int,String>epochTaskDict = {")
for entry in sorted(dateDict.keys()):
    values = []
    for item in dateDict[entry]:
        i = items.index(item)
        values.append(str(i))
    print("{{{},\"{}\"}}, //{} {}".format(entry,",".join(values),datetime.datetime.fromtimestamp(entry).strftime('%d.%m.%Y'), dateDict[entry]))
print("};")
print("const String task[] = {{\"{}\"}};".format("\", \"".join(items)))

def getMatchingColor(item):
    for key in colorDict.keys():
        for entry in key.split(","):
            if(re.search(entry, item.upper())):
                return(colorDict[key])
    print("WARNING: No matching color detected for item {}.".format(item))

colors = []
for item in items:
    colors.append(getMatchingColor(item))    

print("const int color[] = {{{}}};".format(",".join(colors)))
print("const int validIndex[] = {{{}}};".format(",".join("%d" %i for i in range(0,len(items))))) #keine Altpapiertonne, kein Bioabfall, (aber Papier, Pappe Kartonagen von Bogenschütz)

print("Number of entries " + str(len(dateDict)))