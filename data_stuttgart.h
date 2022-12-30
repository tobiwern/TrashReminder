//C:/Users/tobiw/Downloads/stuttgart-gruenewaldstr25.ics
#include <unordered_map>
std::unordered_map<int,String>epochTaskDict = {
{1672358400,"3"}, //30.12.2022 ['Restmüll 02-wöchentl.']
{1672444800,"1"}, //31.12.2022 ['Biomüll 01-wöchentl.']
{1673222400,"1,0"}, //09.01.2023 ['Biomüll 01-wöchentl.', 'Altpapier 03-wöchentl.']
{1673308800,"2"}, //10.01.2023 ['Gelber Sack 03-wöchentl.']
{1673568000,"3"}, //13.01.2023 ['Restmüll 02-wöchentl.']
{1673654400,"1"}, //14.01.2023 ['Biomüll 01-wöchentl.']
{1674172800,"1"}, //20.01.2023 ['Biomüll 01-wöchentl.']
{1674691200,"3"}, //26.01.2023 ['Restmüll 02-wöchentl.']
{1674777600,"1,0"}, //27.01.2023 ['Biomüll 01-wöchentl.', 'Altpapier 03-wöchentl.']
};
const String task[] = {"Altpapier 03-wöchentl.", "Biomüll 01-wöchentl.", "Gelber Sack 03-wöchentl.", "Restmüll 02-wöchentl."};
const int color[] = {0x0000FF,0x00FF00,0xFFFF00,0xFFFFFF};
const int validIndex[] = {0,1,2,3};