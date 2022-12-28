//https://www.abfall-kreis-tuebingen.de/online-abfuhrtermine/
//https://www.bogenschuetz-entsorgung.de/blaue-tonne-tuebingen/abfuhrtermine.html

#include <unordered_map>

std::unordered_map<int,String>epochTaskDict = {
{1672012800,"0,4"}, //dummy 2022.12.26 show
{1672099200,"2"}, //2022.12.27
{1672444800,"4"}, //2022.12.31
{1673049600,"1"},
{1673222400,"2"},
{1673568000,"0,4"}, //Bogenschütz
{1674172800,"1"},
{1674432000,"2"},
{1674777600,"4"},
{1675382400,"1"},
{1675641600,"2"},
{1675987200,"0,4"}, //Bogenschütz
{1676592000,"1"},
{1676851200,"2"},
{1677196800,"4"},
{1677801600,"1"},
{1678060800,"2"},
{1678406400,"0,4"}, //Bogenschütz
{1679011200,"1"},
{1679270400,"2"},
{1679616000,"4"},
{1680134400,"3"},
{1680220800,"1"},
{1680480000,"2"},
{1680912000,"0,4"}, //Bogenschütz
{1681516800,"1"},
{1681689600,"2"},
{1682035200,"4"},
{1682640000,"1"},
{1682985600,"2"},
{1683331200,"0,4"}, //Bogenschütz
{1683849600,"1"},
{1684108800,"2"},
{1684540800,"4"},
{1685059200,"1"},
{1685404800,"2"},
{1685750400,"0,4"}, //Bogenschütz
{1686355200,"1"},
{1686528000,"2"},
{1686873600,"4,1"},
{1687478400,"1"},
{1687737600,"2"},
{1688083200,"0,4,1"}, //Bogenschütz
{1688688000,"1"},
{1688947200,"2"},
{1689292800,"4,1"},
{1689897600,"1"},
{1690156800,"2"},
{1690502400,"0,4,1"}, //Bogenschütz
{1691107200,"1"},
{1691366400,"2"},
{1691712000,"4,1"},
{1692316800,"1"},
{1692576000,"2"},
{1692921600,"0,4,1"}, //Bogenschütz
{1693526400,"1"},
{1693785600,"2"},
{1694131200,"4,1"},
{1694736000,"1"},
{1694995200,"2"},
{1695340800,"0,4"}, //Bogenschütz
{1695945600,"1"},
{1696204800,"2"},
{1696636800,"4"},
{1697155200,"1"},
{1697414400,"2"},
{1697760000,"0,4"}, //Bogenschütz
{1698364800,"1"},
{1698624000,"2"},
{1698969600,"3"},
{1699056000,"4"},
{1699574400,"1"},
{1699833600,"2"},
{1700179200,"0,4"}, //Bogenschütz
{1700784000,"1"},
{1701043200,"2"},
{1701388800,"4"},
{1701993600,"1"},
{1702252800,"2"},
{1702598400,"0,4"}, //Bogenschütz
{1703203200,"1"},
{1703289600,"2"},
{1703894400,"4"},
{1671753600,"0,2,3"}, //dummy
{1671840000,"4,3"}, //dummy
{1671926400,"4,3"}, //dummy 2022.12.24 show
};
//                      0                                1                          2                            3                           4
const String task[] = {"Altpapier-Tonne in Hirrlingen", "Bioabfall in Hirrlingen", "Gelber Sack in Hirrlingen", "Häckselgut in Hirrlingen", "Restmüll in Hirrlingen"};
//                   blue 0    brown 1   yellow 2  green 3   white 4
const int color[] = {0x0000FF, 0xA52A2A, 0xFFFF00, 0x00FF00, 0xFFFFFF };
const int validIndex[] = {0,2,3,4};