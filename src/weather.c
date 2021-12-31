#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "bytes.h"
#include "readfile.h"

// Bleh
#include "dspweather.h"

struct zone_name_t {
    uint32_t id;
    const char* name;
};

static const struct zone_name_t zoneNames[] = {
    { 1, "Phanauet Channel" },
    { 2, "Carpenters' Landing" },
    { 3, "Manaclipper" },
    { 4, "Bibiki Bay" },
    { 5, "Uleguerand Range" },
    { 6, "Bearclaw Pinnacle" },
    { 7, "Attohwa Chasm" },
    { 8, "Boneyard Gully" },
    { 9, "Pso'Xja" },
    { 10, "The Shrouded Maw" },
    { 11, "Oldton Movalpolos" },
    { 12, "Newton Movalpolos" },
    { 13, "Mine Shaft #2716" },
    { 14, "Hall of Transference" },
    { 15, "Abyssea - Konschtat" },
    { 16, "Promyvion - Holla" },
    { 17, "Spire of Holla" },
    { 18, "Promyvion - Dem" },
    { 19, "Spire of Dem" },
    { 20, "Promyvion - Mea" },
    { 21, "Spire of Mea" },
    { 22, "Promyvion - Vahzl" },
    { 23, "Spire of Vahzl" },
    { 24, "Lufaise Meadows" },
    { 25, "Misareaux Coast" },
    { 26, "Tavnazian Safehold" },
    { 27, "Phomiuna Aqueducts" },
    { 28, "Sacrarium" },
    { 29, "Riverne - Site #B01" },
    { 30, "Riverne - Site #A01" },
    { 31, "Monarch Linn" },
    { 32, "Sealion's Den" },
    { 33, "Al'Taieu" },
    { 34, "Grand Palace of Hu'Xzoi" },
    { 35, "The Garden of Ru'Hmet" },
    { 36, "Empyreal Paradox" },
    { 37, "Temenos" },
    { 38, "Apollyon" },
    { 39, "Dynamis - Valkurm" },
    { 40, "Dynamis - Buburimu" },
    { 41, "Dynamis - Qufim" },
    { 42, "Dynamis - Tavnazia" },
    { 43, "Diorama Abdhaljs-Ghelsba" },
    { 44, "Abdhaljs Isle-Purgonorgo" },
    { 45, "Abyssea - Tahrongi" },
    { 46, "Open sea route to Al Zahbi" },
    { 47, "Open sea route to Mhaura" },
    { 48, "Al Zahbi" },
    { 50, "Aht Urhgan Whitegate" },
    { 51, "Wajaom Woodlands" },
    { 52, "Bhaflau Thickets" },
    { 53, "Nashmau" },
    { 54, "Arrapago Reef" },
    { 55, "Ilrusi Atoll" },
    { 56, "Periqia" },
    { 57, "Talacca Cove" },
    { 58, "Silver Sea route to Nashmau" },
    { 59, "Silver Sea route to Al Zahbi" },
    { 60, "The Ashu Talif" },
    { 61, "Mount Zhayolm" },
    { 62, "Halvung" },
    { 63, "Lebros Cavern" },
    { 64, "Navukgo Execution Chamber" },
    { 65, "Mamook" },
    { 66, "Mamool Ja Training Grounds" },
    { 67, "Jade Sepulcher" },
    { 68, "Aydeewa Subterrane" },
    { 69, "Leujaoam Sanctum" },
    { 70, "Chocobo Circuit" },
    { 71, "The Colosseum" },
    { 72, "Alzadaal Undersea Ruins" },
    { 73, "Zhayolm Remnants" },
    { 74, "Arrapago Remnants" },
    { 75, "Bhaflau Remnants" },
    { 76, "Silver Sea Remnants" },
    { 77, "Nyzul Isle" },
    { 78, "Hazhalm Testing Grounds" },
    { 79, "Caedarva Mire" },
    { 80, "Southern San d'Oria [S]" },
    { 81, "East Ronfaure [S]" },
    { 82, "Jugner Forest [S]" },
    { 83, "Vunkerl Inlet [S]" },
    { 84, "Batallia Downs [S]" },
    { 85, "La Vaule [S]" },
    { 86, "Everbloom Hollow" },
    { 87, "Bastok Markets [S]" },
    { 88, "North Gustaberg [S]" },
    { 89, "Grauberg [S]" },
    { 90, "Pashhow Marshlands [S]" },
    { 91, "Rolanberry Fields [S]" },
    { 92, "Beadeaux [S]" },
    { 93, "Ruhotz Silvermines" },
    { 94, "Windurst Waters [S]" },
    { 95, "West Sarutabaruta [S]" },
    { 96, "Fort Karugo-Narugo [S]" },
    { 97, "Meriphataud Mountains [S]" },
    { 98, "Sauromugue Champaign [S]" },
    { 99, "Castle Oztroja [S]" },
    { 100, "West Ronfaure" },
    { 101, "East Ronfaure" },
    { 102, "La Theine Plateau" },
    { 103, "Valkurm Dunes" },
    { 104, "Jugner Forest" },
    { 105, "Batallia Downs" },
    { 106, "North Gustaberg" },
    { 107, "South Gustaberg" },
    { 108, "Konschtat Highlands" },
    { 109, "Pashhow Marshlands" },
    { 110, "Rolanberry Fields" },
    { 111, "Beaucedine Glacier" },
    { 112, "Xarcabard" },
    { 113, "Cape Teriggan" },
    { 114, "Eastern Altepa Desert" },
    { 115, "West Sarutabaruta" },
    { 116, "East Sarutabaruta" },
    { 117, "Tahrongi Canyon" },
    { 118, "Buburimu Peninsula" },
    { 119, "Meriphataud Mountains" },
    { 120, "Sauromugue Champaign" },
    { 121, "The Sanctuary of Zi'Tah" },
    { 122, "Ro'Maeve" },
    { 123, "Yuhtunga Jungle" },
    { 124, "Yhoator Jungle" },
    { 125, "Western Altepa Desert" },
    { 126, "Qufim Island" },
    { 127, "Behemoth's Dominion" },
    { 128, "Valley of Sorrows" },
    { 129, "Ghoyu's Reverie" },
    { 130, "Ru'Aun Gardens" },
    { 131, "Mordion Gaol" },
    { 132, "Abyssea - La Theine" },
    { 134, "Dynamis - Beaucedine" },
    { 135, "Dynamis - Xarcabard" },
    { 136, "Beaucedine Glacier [S]" },
    { 137, "Xarcabard [S]" },
    { 138, "Castle Zvahl Baileys [S]" },
    { 139, "Horlais Peak" },
    { 140, "Ghelsba Outpost" },
    { 141, "Fort Ghelsba" },
    { 142, "Yughott Grotto" },
    { 143, "Palborough Mines" },
    { 144, "Waughroon Shrine" },
    { 145, "Giddeus" },
    { 146, "Balga's Dais" },
    { 147, "Beadeaux" },
    { 148, "Qulun Dome" },
    { 149, "Davoi" },
    { 150, "Monastic Cavern" },
    { 151, "Castle Oztroja" },
    { 152, "Altar Room" },
    { 153, "The Boyahda Tree" },
    { 154, "Dragon's Aery" },
    { 155, "Castle Zvahl Keep [S]" },
    { 156, "Throne Room [S]" },
    { 157, "Middle Delkfutt's Tower" },
    { 158, "Upper Delkfutt's Tower" },
    { 159, "Temple of Uggalepih" },
    { 160, "Den of Rancor" },
    { 161, "Castle Zvahl Baileys" },
    { 162, "Castle Zvahl Keep" },
    { 163, "Sacrificial Chamber" },
    { 164, "Garlaige Citadel [S]" },
    { 165, "Throne Room" },
    { 166, "Ranguemont Pass" },
    { 167, "Bostaunieux Oubliette" },
    { 168, "Chamber of Oracles" },
    { 169, "Toraimarai Canal" },
    { 170, "Full Moon Fountain" },
    { 171, "Crawlers' Nest [S]" },
    { 172, "Zeruhn Mines" },
    { 173, "Korroloka Tunnel" },
    { 174, "Kuftal Tunnel" },
    { 175, "The Eldieme Necropolis [S]" },
    { 176, "Sea Serpent Grotto" },
    { 177, "Ve'Lugannon Palace" },
    { 178, "The Shrine of Ru'Avitau" },
    { 179, "Stellar Fulcrum" },
    { 180, "La'Loff Amphitheater" },
    { 181, "The Celestial Nexus" },
    { 182, "Walk of Echoes" },
    { 183, "Maquette Abdhaljs-Legion" },
    { 184, "Lower Delkfutt's Tower" },
    { 185, "Dynamis - San d'Oria" },
    { 186, "Dynamis - Bastok" },
    { 187, "Dynamis - Windurst" },
    { 188, "Dynamis - Jeuno" },
    { 190, "King Ranperre's Tomb" },
    { 191, "Dangruf Wadi" },
    { 192, "Inner Horutoto Ruins" },
    { 193, "Ordelle's Caves" },
    { 194, "Outer Horutoto Ruins" },
    { 195, "The Eldieme Necropolis" },
    { 196, "Gusgen Mines" },
    { 197, "Crawlers' Nest" },
    { 198, "Maze of Shakhrami" },
    { 200, "Garlaige Citadel" },
    { 201, "Cloister of Gales" },
    { 202, "Cloister of Storms" },
    { 203, "Cloister of Frost" },
    { 204, "Fei'Yin" },
    { 205, "Ifrit's Cauldron" },
    { 206, "Qu'Bia Arena" },
    { 207, "Cloister of Flames" },
    { 208, "Quicksand Caves" },
    { 209, "Cloister of Tremors" },
    { 211, "Cloister of Tides" },
    { 212, "Gustav Tunnel" },
    { 213, "Labyrinth of Onzozo" },
    { 215, "Abyssea - Attohwa" },
    { 216, "Abyssea - Misareaux" },
    { 217, "Abyssea - Vunkerl" },
    { 218, "Abyssea - Altepa" },
    { 220, "Ship bound for Selbina" },
    { 221, "Ship bound for Mhaura" },
    { 222, "Provenance" },
    { 223, "San d'Oria-Jeuno Airship" },
    { 224, "Bastok-Jeuno Airship" },
    { 225, "Windurst-Jeuno Airship" },
    { 226, "Kazham-Jeuno Airship" },
    { 227, "Ship bound for Selbina" },
    { 228, "Ship bound for Mhaura" },
    { 230, "Southern San d'Oria" },
    { 231, "Northern San d'Oria" },
    { 232, "Port San d'Oria" },
    { 233, "Chateau d'Oraguille" },
    { 234, "Bastok Mines" },
    { 235, "Bastok Markets" },
    { 236, "Port Bastok" },
    { 237, "Metalworks" },
    { 238, "Windurst Waters" },
    { 239, "Windurst Walls" },
    { 240, "Port Windurst" },
    { 241, "Windurst Woods" },
    { 242, "Heavens Tower" },
    { 243, "Ru'Lude Gardens" },
    { 244, "Upper Jeuno" },
    { 245, "Lower Jeuno" },
    { 246, "Port Jeuno" },
    { 247, "Rabao" },
    { 248, "Selbina" },
    { 249, "Mhaura" },
    { 250, "Kazham" },
    { 251, "Hall of the Gods" },
    { 252, "Norg" },
    { 253, "Abyssea - Uleguerand" },
    { 254, "Abyssea - Grauberg" },
    { 255, "Abyssea - Empyreal Paradox" },
    { 256, "Western Adoulin" },
    { 257, "Eastern Adoulin" },
    { 258, "Rala Waterways" },
    { 259, "Rala Waterways [U]" },
    { 260, "Yahse Hunting Grounds" },
    { 261, "Ceizak Battlegrounds" },
    { 262, "Foret de Hennetiel" },
    { 263, "Yorcia Weald" },
    { 264, "Yorcia Weald [U]" },
    { 265, "Morimar Basalt Fields" },
    { 266, "Marjami Ravine" },
    { 267, "Kamihr Drifts" },
    { 268, "Sih Gates" },
    { 269, "Moh Gates" },
    { 270, "Cirdas Caverns" },
    { 271, "Cirdas Caverns [U]" },
    { 272, "Dho Gates" },
    { 273, "Woh Gates" },
    { 274, "Outer Ra'Kaznar" },
    { 275, "Outer Ra'Kaznar [U]" },
    { 276, "Ra'Kaznar Inner Court" },
    { 277, "Ra'Kaznar Turris" },
    { 280, "Mog Garden" },
    { 281, "Leafallia" },
    { 282, "Mount Kamihr" },
    { 284, "Celennia Memorial Library" },
    { 285, "Feretory" },
    { 288, "Escha - Zi'Tah" },
    { 289, "Escha - Ru'Aun" },
    { 290, "Desuetia - Empyreal Paradox" },
    { 291, "Reisenjima" },
    { 292, "Reisenjima Henge" },
    { 293, "Reisenjima Sanctorium" },
    { 294, "Dynamis - San d'Oria [D]" },
    { 295, "Dynamis - Bastok [D]" },
    { 296, "Dynamis - Windurst [D]" },
    { 297, "Dynamis - Jeuno [D]" },
};

static const char* weatherNames[] = {
    "fine patches",
    "sunshine",
    "clouds",
    "fog",
    "hot spells",
    "heat waves",
    "rain",
    "squalls",
    "dust storms",
    "sand storms",
    "winds",
    "gales",
    "snow",
    "blizzards",
    "thunder",
    "thunderstorms",
    "auroras",
    "stellar glare",
    "gloom",
    "darkness"
};

enum ELEMENT {
    NONE,
    FIRE,
    WATER,
    EARTH,
    WIND,
    ICE,
    THUNDER,
    LIGHT,
    DARK
};

static const enum ELEMENT weatherToElement[] = {
    NONE,
    NONE,
    NONE,
    NONE,
    FIRE,
    FIRE,
    WATER,
    WATER,
    EARTH,
    EARTH,
    WIND,
    WIND,
    ICE,
    ICE,
    THUNDER,
    THUNDER,
    LIGHT,
    LIGHT,
    DARK,
    DARK
};

static const char* ElementNames[] = {
    [NONE] = "No",
    [FIRE] = "Fire",
    [WATER] =  "Water",
    [EARTH] = "Earth",
    [WIND] = "Wind",
    [ICE] = "Ice",
    [THUNDER] = "Thunder",
    [LIGHT] = "Light",
    [DARK] = "Dark"
};

static const char* ElementalNames[] = {
    [NONE] = "<none>",
    [FIRE] = "Fire Elemental",
    [WATER] =  "Water Elemental",
    [EARTH] = "Earth Elemental",
    [WIND] = "Air Elemental",
    [ICE] = "Ice Elemental",
    [THUNDER] = "Thunder Elemental",
    [LIGHT] = "Light Elemental",
    [DARK] = "Dark Elemental"
};

struct zone_elementals_t {
    uint32_t id;
    /*const*/ enum ELEMENT elements[9];
};

// Elemental placement was hand-done but has errors.
// These values will be filled in using the next table.
static /*const*/ struct zone_elementals_t ZoneElementals[] = {
    { 1, { THUNDER, WATER, NONE } },
    { 2, { NONE } },
    { 3, { NONE } },
    { 4, { NONE } },
    { 5, { ICE, NONE } },
    { 6, { NONE } },
    { 7, { WIND, EARTH, NONE } },
    { 8, { NONE } },
    { 9, { NONE } },
    { 10, { NONE } },
    { 11, { EARTH, THUNDER, NONE } },
    { 12, { EARTH, THUNDER, NONE } },
    { 13, { NONE } },
    { 14, { NONE } },
    { 15, { NONE } },
    { 16, { NONE } },
    { 17, { NONE } },
    { 18, { NONE } },
    { 19, { NONE } },
    { 20, { NONE } },
    { 21, { NONE } },
    { 22, { NONE } },
    { 23, { NONE } },
    { 24, { WIND, DARK, THUNDER, NONE } },
    { 25, { WIND, DARK, THUNDER, NONE } },
    { 26, { NONE } },
    { 27, { WIND, DARK, THUNDER, NONE } },
    { 28, { WIND, DARK, THUNDER, NONE } },
    { 29, { WIND, THUNDER, NONE } },
    { 30, { WIND, THUNDER, NONE } },
    { 31, { NONE } },
    { 32, { NONE } },
    { 33, { NONE } },
    { 34, { NONE } },
    { 35, { NONE } },
    { 36, { NONE } },
    { 37, { NONE } },
    { 38, { NONE } },
    { 39, { NONE } },
    { 40, { NONE } },
    { 41, { NONE } },
    { 42, { NONE } },
    { 43, { NONE } },
    { 44, { NONE } },
    { 45, { NONE } },
    { 46, { THUNDER, WATER, NONE } },
    { 47, { THUNDER, WATER, NONE } },
    { 48, { NONE } },
    { 50, { NONE } },
    { 51, { WIND, NONE } },
    { 52, { WIND, NONE } },
    { 53, { NONE } },
    { 54, { DARK, ICE, NONE } },
    { 55, { NONE } },
    { 56, { NONE } },
    { 57, { NONE } },
    { 58, { WIND, THUNDER, NONE } },
    { 59, { WIND, THUNDER, NONE } },
    { 60, { NONE } },
    { 61, { EARTH, FIRE, NONE } },
    { 62, { EARTH, FIRE, NONE } },
    { 63, { NONE } },
    { 64, { NONE } },
    { 65, { WIND, NONE } },
    { 66, { NONE } },
    { 67, { NONE } },
    { 68, { WIND, NONE } },
    { 69, { NONE } },
    { 70, { NONE } },
    { 71, { NONE } },
    { 72, { NONE } },
    { 73, { NONE } },
    { 74, { NONE } },
    { 75, { NONE } },
    { 76, { NONE } },
    { 77, { NONE } },
    { 78, { NONE } },
    { 79, { DARK, THUNDER, WATER, NONE } },
    { 80, { NONE } },
    { 81, { NONE } },
    { 82, { THUNDER, WATER, NONE } },
    { 83, { WIND, THUNDER, NONE } },
    { 84, { EARTH, ICE, NONE } },
    { 85, { THUNDER, WATER, NONE } },
    { 86, { NONE } },
    { 87, { NONE } },
    { 88, { NONE } },
    { 89, { WIND, THUNDER, NONE } },
    { 90, { THUNDER, WATER, NONE } },
    { 91, { FIRE, WATER, NONE } },
    { 92, { THUNDER, WATER, NONE } },
    { 93, { NONE } },
    { 94, { NONE } },
    { 95, { NONE } },
    { 96, { WIND, EARTH, NONE } },
    { 97, { EARTH, FIRE, NONE } },
    { 98, { EARTH, NONE } },
    { 99, { EARTH, FIRE, NONE } },
    { 100, { NONE } },
    { 101, { NONE } },
    { 102, { WIND, WATER, NONE } },
    { 103, { EARTH, FIRE, NONE } },
    { 104, { THUNDER, WATER, NONE } },
    { 105, { EARTH, ICE, NONE } },
    { 106, { NONE } },
    { 107, { NONE } },
    { 108, { EARTH, THUNDER, NONE } },
    { 109, { THUNDER, WATER, NONE } },
    { 110, { FIRE, WATER, NONE } },
    { 111, { DARK, ICE, NONE } },
    { 112, { DARK, ICE, NONE } },
    { 113, { WIND, FIRE, NONE } },
    { 114, { EARTH, FIRE, NONE } },
    { 115, { NONE } },
    { 116, { NONE } },
    { 117, { WIND, EARTH, NONE } },
    { 118, { WIND, WATER, NONE } },
    { 119, { EARTH, FIRE, NONE } },
    { 120, { EARTH, THUNDER, NONE } },
    { 121, { NONE } },
    { 122, { THUNDER, WATER, NONE } },
    { 123, { NONE } },
    { 124, { FIRE, WATER, NONE } },
    { 125, { EARTH, FIRE, NONE } },
    { 126, { LIGHT, THUNDER, NONE } },
    { 127, { NONE } },
    { 128, { WIND, FIRE, NONE } },
    { 129, { NONE } },
    { 130, { NONE } },
    { 131, { NONE } },
    { 132, { NONE } },
    { 134, { NONE } },
    { 135, { NONE } },
    { 136, { DARK, ICE, NONE } },
    { 137, { DARK, ICE, NONE } },
    { 138, { DARK, ICE, NONE } },
    { 139, { NONE } },
    { 140, { NONE } },
    { 141, { NONE } },
    { 142, { NONE } },
    { 143, { NONE } },
    { 144, { NONE } },
    { 145, { NONE } },
    { 146, { NONE } },
    { 147, { THUNDER, WATER, NONE } },
    { 148, { NONE } },
    { 149, { NONE } },
    { 150, { THUNDER, WATER, NONE } },
    { 151, { EARTH, FIRE, NONE } },
    { 152, { NONE } },
    { 153, { THUNDER, WATER, NONE } },
    { 154, { NONE } },
    { 155, { DARK, ICE, NONE } },
    { 156, { NONE } },
    { 157, { NONE } },
    { 158, { NONE } },
    { 159, { FIRE, WATER, NONE } },
    { 160, { FIRE, WATER, NONE } },
    { 161, { DARK, ICE, NONE } },
    { 162, { NONE } },
    { 163, { NONE } },
    { 164, { EARTH, THUNDER, NONE } },
    { 165, { NONE } },
    { 166, { NONE } },
    { 167, { NONE } },
    { 168, { NONE } },
    { 169, { NONE } },
    { 170, { NONE } },
    { 171, { NONE } },
    { 172, { NONE } },
    { 173, { THUNDER, WATER, NONE } },
    { 174, { WIND, FIRE, NONE } },
    { 175, { EARTH, ICE, NONE } },
    { 176, { THUNDER, WATER, NONE } },
    { 177, { NONE } },
    { 178, { NONE } },
    { 179, { NONE } },
    { 180, { NONE } },
    { 181, { NONE } },
    { 182, { NONE } },
    { 183, { NONE } },
    { 184, { NONE } },
    { 185, { NONE } },
    { 186, { NONE } },
    { 187, { NONE } },
    { 188, { NONE } },
    { 190, { NONE } },
    { 191, { NONE } },
    { 192, { NONE } },
    { 193, { NONE } },
    { 194, { NONE } },
    { 195, { EARTH, ICE, NONE } },
    { 196, { EARTH, THUNDER, NONE } },
    { 197, { NONE } },
    { 198, { WIND, EARTH, NONE } },
    { 200, { EARTH, THUNDER, NONE } },
    { 201, { NONE } },
    { 202, { NONE } },
    { 203, { NONE } },
    { 204, { NONE } },
    { 205, { NONE } },
    { 206, { NONE } },
    { 207, { NONE } },
    { 208, { NONE } },
    { 209, { NONE } },
    { 211, { NONE } },
    { 212, { EARTH, FIRE, NONE } },
    { 213, { WIND, WATER, NONE } },
    { 215, { NONE } },
    { 216, { NONE } },
    { 217, { NONE } },
    { 218, { NONE } },
    { 220, { NONE } },
    { 221, { NONE } },
    { 222, { NONE } },
    { 223, { NONE } },
    { 224, { NONE } },
    { 225, { NONE } },
    { 226, { NONE } },
    { 227, { NONE } },
    { 228, { NONE } },
    { 230, { NONE } },
    { 231, { NONE } },
    { 232, { NONE } },
    { 233, { NONE } },
    { 234, { NONE } },
    { 235, { NONE } },
    { 236, { NONE } },
    { 237, { NONE } },
    { 238, { NONE } },
    { 239, { NONE } },
    { 240, { NONE } },
    { 241, { NONE } },
    { 242, { NONE } },
    { 243, { NONE } },
    { 244, { NONE } },
    { 245, { NONE } },
    { 246, { NONE } },
    { 247, { NONE } },
    { 248, { NONE } },
    { 249, { NONE } },
    { 250, { NONE } },
    { 251, { NONE } },
    { 252, { NONE } },
    { 253, { NONE } },
    { 254, { NONE } },
    { 255, { NONE } },
    { 256, { NONE } },
    { 257, { NONE } },
    { 258, { NONE } },
    { 259, { NONE } },
    { 260, { NONE } },
    { 261, { NONE } },
    { 262, { NONE } },
    { 263, { NONE } },
    { 264, { NONE } },
    { 265, { NONE } },
    { 266, { NONE } },
    { 267, { NONE } },
    { 268, { NONE } },
    { 269, { NONE } },
    { 270, { NONE } },
    { 271, { NONE } },
    { 272, { NONE } },
    { 273, { NONE } },
    { 274, { NONE } },
    { 275, { NONE } },
    { 276, { NONE } },
    { 277, { NONE } },
    { 280, { NONE } },
    { 281, { NONE } },
    { 282, { NONE } },
    { 284, { NONE } },
    { 285, { NONE } },
    { 288, { NONE } },
    { 289, { NONE } },
    { 290, { NONE } },
    { 291, { NONE } },
    { 292, { NONE } },
    { 293, { NONE } },
    { 294, { NONE } },
    { 295, { NONE } },
    { 296, { NONE } },
    { 297, { NONE } },
};

struct zone_elemental_data_t {
    const char* ZoneName;
    const char* ElementalName;
};

// As this table was generated from the NPC name data,
// this table can contain bad entries due to quest monsters, etc.
static const struct zone_elemental_data_t ZoneElementalData[] = {
    /* 1 */ { "Caedarva Mire","Thunder Elemental" },
    /* 1 */ { "Castle Zvahl Keep [S]","Dark Elemental" },
    /* 1 */ { "Castle Zvahl Keep [S]","Ice Elemental" },
//    /* 1 */ { "Dangruf Wadi","Fire Elemental" },
    /* 1 */ { "Fei'Yin","Dark Elemental" },
    /* 1 */ { "Fei'Yin","Ice Elemental" },
    /* 1 */ { "Fort Karugo-Narugo [S]","Air Elemental" },
    /* 1 */ { "Fort Karugo-Narugo [S]","Earth Elemental" },
    /* 1 */ { "Garlaige Citadel [S]","Earth Elemental" },
    /* 1 */ { "Garlaige Citadel [S]","Thunder Elemental" },
//    /* 1 */ { "Garlaige Citadel","Light Elemental" },
    /* 1 */ { "La Vaule [S]","Water Elemental" },
    /* 1 */ { "Lufaise Meadows","Dark Elemental" },
//    /* 1 */ { "Maze of Shakhrami","Dark Elemental" },
    /* 1 */ { "Open sea route to Al Zahbi","Thunder Elemental" },
    /* 1 */ { "Open sea route to Al Zahbi","Water Elemental" },
    /* 1 */ { "Open sea route to Mhaura","Thunder Elemental" },
    /* 1 */ { "Open sea route to Mhaura","Water Elemental" },
    /* 1 */ { "Ordelle's Caves","Water Elemental" },
//    /* 1 */ { "Outer Horutoto Ruins","Thunder Elemental" },
    /* 1 */ { "Phanauet Channel","Thunder Elemental" },
    /* 1 */ { "Phanauet Channel","Water Elemental" },
//    /* 1 */ { "Riverne - Site #A01","Earth Elemental" },
//    /* 1 */ { "Riverne - Site #A01","Water Elemental" },
//    /* 1 */ { "Riverne - Site #B01","Earth Elemental" },
//    /* 1 */ { "Riverne - Site #B01","Water Elemental" },
    /* 1 */ { "Sauromugue Champaign [S]","Earth Elemental" },
    /* 1 */ { "Silver Sea route to Al Zahbi","Air Elemental" },
    /* 1 */ { "Silver Sea route to Al Zahbi","Thunder Elemental" },
    /* 1 */ { "Silver Sea route to Nashmau","Air Elemental" },
    /* 1 */ { "Silver Sea route to Nashmau","Thunder Elemental" },
    /* 1 */ { "The Eldieme Necropolis [S]","Ice Elemental" },
    /* 2 */ { "Batallia Downs [S]","Ice Elemental" },
    /* 2 */ { "Beadeaux [S]","Water Elemental" },
    /* 2 */ { "Caedarva Mire","Dark Elemental" },
    /* 2 */ { "Crawlers' Nest [S]","Water Elemental" },
    /* 2 */ { "Gusgen Mines","Thunder Elemental" },
    /* 2 */ { "Gustav Tunnel","Fire Elemental" },
    /* 2 */ { "Korroloka Tunnel","Thunder Elemental" },
    /* 2 */ { "La Vaule [S]","Thunder Elemental" },
    /* 2 */ { "Lower Delkfutt's Tower","Light Elemental" },
    /* 2 */ { "Lower Delkfutt's Tower","Thunder Elemental" },
    /* 2 */ { "Misareaux Coast","Air Elemental" },
    /* 2 */ { "Misareaux Coast","Dark Elemental" },
    /* 2 */ { "Ordelle's Caves","Air Elemental" },
    /* 2 */ { "Pashhow Marshlands [S]","Thunder Elemental" },
    /* 2 */ { "Phomiuna Aqueducts","Air Elemental" },
    /* 2 */ { "Phomiuna Aqueducts","Thunder Elemental" },
    /* 2 */ { "Riverne - Site #B01","Thunder Elemental" },
    /* 2 */ { "Ro'Maeve","Thunder Elemental" },
    /* 2 */ { "Ro'Maeve","Water Elemental" },
    /* 2 */ { "Sacrarium","Air Elemental" },
    /* 2 */ { "Sacrarium","Dark Elemental" },
    /* 2 */ { "Sacrarium","Thunder Elemental" },
    /* 2 */ { "The Eldieme Necropolis [S]","Earth Elemental" },
    /* 2 */ { "Valley of Sorrows","Air Elemental" },
    /* 2 */ { "Valley of Sorrows","Fire Elemental" },
    /* 2 */ { "Yhoator Jungle","Fire Elemental" },
    /* 3 */ { "Batallia Downs [S]","Earth Elemental" },
    /* 3 */ { "Batallia Downs","Earth Elemental" },
    /* 3 */ { "Batallia Downs","Ice Elemental" },
    /* 3 */ { "Beadeaux [S]","Thunder Elemental" },
    /* 3 */ { "Bhaflau Thickets","Air Elemental" },
    /* 3 */ { "Buburimu Peninsula","Water Elemental" },
    /* 3 */ { "Castle Oztroja [S]","Earth Elemental" },
    /* 3 */ { "Castle Oztroja [S]","Fire Elemental" },
    /* 3 */ { "Crawlers' Nest [S]","Fire Elemental" },
    /* 3 */ { "Den of Rancor","Water Elemental" },
    /* 3 */ { "Garlaige Citadel","Earth Elemental" },
    /* 3 */ { "Garlaige Citadel","Thunder Elemental" },
    /* 3 */ { "Grauberg [S]","Thunder Elemental" },
    /* 3 */ { "Gusgen Mines","Earth Elemental" },
    /* 3 */ { "Gustav Tunnel","Earth Elemental" },
    /* 3 */ { "Konschtat Highlands","Earth Elemental" },
    /* 3 */ { "Konschtat Highlands","Thunder Elemental" },
    /* 3 */ { "La Theine Plateau","Water Elemental" },
    /* 3 */ { "Lufaise Meadows","Thunder Elemental" },
    /* 3 */ { "Meriphataud Mountains [S]","Earth Elemental" },
    /* 3 */ { "Meriphataud Mountains [S]","Fire Elemental" },
    /* 3 */ { "Meriphataud Mountains","Fire Elemental" },
    /* 3 */ { "Misareaux Coast","Thunder Elemental" },
    /* 3 */ { "Newton Movalpolos","Earth Elemental" },
    /* 3 */ { "Newton Movalpolos","Thunder Elemental" },
    /* 3 */ { "Pashhow Marshlands [S]","Water Elemental" },
    /* 3 */ { "Phomiuna Aqueducts","Dark Elemental" },
    /* 3 */ { "Riverne - Site #A01","Thunder Elemental" },
    /* 3 */ { "Rolanberry Fields [S]","Water Elemental" },
    /* 3 */ { "Rolanberry Fields","Fire Elemental" },
    /* 3 */ { "Rolanberry Fields","Water Elemental" },
    /* 3 */ { "Sauromugue Champaign","Earth Elemental" },
    /* 3 */ { "Sauromugue Champaign","Thunder Elemental" },
    /* 3 */ { "Temple of Uggalepih","Water Elemental" },
    /* 3 */ { "Valkurm Dunes","Earth Elemental" },
    /* 3 */ { "Vunkerl Inlet [S]","Air Elemental" },
    /* 3 */ { "Yhoator Jungle","Water Elemental" },
    /* 4 */ { "Arrapago Reef","Dark Elemental" },
    /* 4 */ { "Beadeaux","Thunder Elemental" },
    /* 4 */ { "Beadeaux","Water Elemental" },
    /* 4 */ { "Beaucedine Glacier","Dark Elemental" },
    /* 4 */ { "Cape Teriggan","Air Elemental" },
    /* 4 */ { "Cape Teriggan","Fire Elemental" },
    /* 4 */ { "Den of Rancor","Fire Elemental" },
    /* 4 */ { "Halvung","Earth Elemental" },
    /* 4 */ { "Jugner Forest [S]","Thunder Elemental" },
    /* 4 */ { "Jugner Forest [S]","Water Elemental" },
    /* 4 */ { "Jugner Forest","Thunder Elemental" },
    /* 4 */ { "Korroloka Tunnel","Water Elemental" },
    /* 4 */ { "Mamook","Air Elemental" },
    /* 4 */ { "Oldton Movalpolos","Thunder Elemental" },
    /* 4 */ { "Qufim Island","Thunder Elemental" },
    /* 4 */ { "Riverne - Site #A01","Air Elemental" },
    /* 4 */ { "Rolanberry Fields [S]","Fire Elemental" },
//    /* 4 */ { "Ru'Aun Gardens","Air Elemental" },
//    /* 4 */ { "Ru'Aun Gardens","Dark Elemental" },
//    /* 4 */ { "Ru'Aun Gardens","Earth Elemental" },
//    /* 4 */ { "Ru'Aun Gardens","Fire Elemental" },
//    /* 4 */ { "Ru'Aun Gardens","Ice Elemental" },
//    /* 4 */ { "Ru'Aun Gardens","Light Elemental" },
//    /* 4 */ { "Ru'Aun Gardens","Thunder Elemental" },
//    /* 4 */ { "Ru'Aun Gardens","Water Elemental" },
    /* 4 */ { "Tahrongi Canyon","Air Elemental" },
    /* 4 */ { "Tahrongi Canyon","Earth Elemental" },
    /* 4 */ { "The Sanctuary of Zi'Tah","Thunder Elemental" },
    /* 4 */ { "The Sanctuary of Zi'Tah","Water Elemental" },
//    /* 4 */ { "The Shrine of Ru'Avitau","Air Elemental" },
//    /* 4 */ { "The Shrine of Ru'Avitau","Dark Elemental" },
//    /* 4 */ { "The Shrine of Ru'Avitau","Earth Elemental" },
//    /* 4 */ { "The Shrine of Ru'Avitau","Fire Elemental" },
//    /* 4 */ { "The Shrine of Ru'Avitau","Ice Elemental" },
//    /* 4 */ { "The Shrine of Ru'Avitau","Thunder Elemental" },
//    /* 4 */ { "The Shrine of Ru'Avitau","Water Elemental" },
//    /* 4 */ { "Ve'Lugannon Palace","Fire Elemental" },
//    /* 4 */ { "Ve'Lugannon Palace","Thunder Elemental" },
    /* 4 */ { "Vunkerl Inlet [S]","Thunder Elemental" },
    /* 5 */ { "Arrapago Reef","Ice Elemental" },
    /* 5 */ { "Aydeewa Subterrane","Air Elemental" },
    /* 5 */ { "Behemoth's Dominion","Light Elemental" },
    /* 5 */ { "Behemoth's Dominion","Thunder Elemental" },
    /* 5 */ { "Castle Zvahl Baileys [S]","Dark Elemental" },
    /* 5 */ { "Castle Zvahl Baileys [S]","Ice Elemental" },
    /* 5 */ { "Castle Zvahl Baileys","Dark Elemental" },
    /* 5 */ { "Castle Zvahl Baileys","Ice Elemental" },
    /* 5 */ { "Grauberg [S]","Air Elemental" },
    /* 5 */ { "La Theine Plateau","Air Elemental" },
    /* 5 */ { "Lufaise Meadows","Air Elemental" },
    /* 5 */ { "Meriphataud Mountains","Earth Elemental" },
    /* 5 */ { "Oldton Movalpolos","Earth Elemental" },
    /* 5 */ { "Pashhow Marshlands","Water Elemental" },
    /* 5 */ { "Riverne - Site #B01","Air Elemental" },
    /* 5 */ { "Temple of Uggalepih","Fire Elemental" },
    /* 5 */ { "The Eldieme Necropolis","Earth Elemental" },
    /* 5 */ { "Upper Delkfutt's Tower","Light Elemental" },
    /* 5 */ { "Upper Delkfutt's Tower","Thunder Elemental" },
    /* 5 */ { "Xarcabard [S]","Dark Elemental" },
    /* 5 */ { "Xarcabard","Dark Elemental" },
    /* 6 */ { "Attohwa Chasm","Earth Elemental" },
    /* 6 */ { "Beaucedine Glacier [S]","Dark Elemental" },
    /* 6 */ { "Beaucedine Glacier","Ice Elemental" },
    /* 6 */ { "Caedarva Mire","Water Elemental" },
    /* 6 */ { "Carpenters' Landing","Thunder Elemental" },
    /* 6 */ { "Eastern Altepa Desert","Fire Elemental" },
    /* 6 */ { "Halvung","Fire Elemental" },
    /* 6 */ { "Jugner Forest","Water Elemental" },
    /* 6 */ { "Maze of Shakhrami","Air Elemental" },
    /* 6 */ { "Maze of Shakhrami","Earth Elemental" },
    /* 6 */ { "Middle Delkfutt's Tower","Light Elemental" },
    /* 6 */ { "Middle Delkfutt's Tower","Thunder Elemental" },
    /* 6 */ { "Pashhow Marshlands","Thunder Elemental" },
    /* 6 */ { "Reisenjima","Water Elemental" },
    /* 6 */ { "Sea Serpent Grotto","Thunder Elemental" },
    /* 6 */ { "The Eldieme Necropolis","Ice Elemental" },
//    /* 6 */ { "Ve'Lugannon Palace","Ice Elemental" },
//    /* 6 */ { "Ve'Lugannon Palace","Water Elemental" },
    /* 6 */ { "Wajaom Woodlands","Air Elemental" },
    /* 6 */ { "Western Altepa Desert","Earth Elemental" },
    /* 6 */ { "Western Altepa Desert","Fire Elemental" },
    /* 6 */ { "Xarcabard [S]","Ice Elemental" },
    /* 6 */ { "Yuhtunga Jungle","Fire Elemental" },
    /* 6 */ { "Yuhtunga Jungle","Water Elemental" },
    /* 7 */ { "Eastern Altepa Desert","Earth Elemental" },
    /* 7 */ { "Mount Zhayolm","Earth Elemental" },
    /* 7 */ { "Mount Zhayolm","Fire Elemental" },
    /* 7 */ { "Pso'Xja","Dark Elemental" },
    /* 8 */ { "Attohwa Chasm","Air Elemental" },
    /* 8 */ { "Beaucedine Glacier [S]","Ice Elemental" },
    /* 8 */ { "Castle Oztroja","Earth Elemental" },
    /* 8 */ { "Castle Oztroja","Fire Elemental" },
    /* 8 */ { "Crawlers' Nest","Fire Elemental" },
    /* 8 */ { "Labyrinth of Onzozo","Air Elemental" },
    /* 8 */ { "Qufim Island","Light Elemental" },
    /* 8 */ { "Valkurm Dunes","Fire Elemental" },
    /* 9 */ { "Buburimu Peninsula","Air Elemental" },
    /* 9 */ { "Crawlers' Nest","Water Elemental" },
    /* 9 */ { "Davoi","Thunder Elemental" },
    /* 9 */ { "Davoi","Water Elemental" },
    /* 9 */ { "Kuftal Tunnel","Air Elemental" },
    /* 9 */ { "Kuftal Tunnel","Fire Elemental" },
    /* 9 */ { "Labyrinth of Onzozo","Water Elemental" },
//    /* 9 */ { "Outer Ra'Kaznar [U]","Air Elemental" },
//    /* 9 */ { "Outer Ra'Kaznar [U]","Dark Elemental" },
//    /* 9 */ { "Outer Ra'Kaznar [U]","Earth Elemental" },
//    /* 9 */ { "Outer Ra'Kaznar [U]","Fire Elemental" },
//    /* 9 */ { "Outer Ra'Kaznar [U]","Ice Elemental" },
//    /* 9 */ { "Outer Ra'Kaznar [U]","Light Elemental" },
//    /* 9 */ { "Outer Ra'Kaznar [U]","Thunder Elemental" },
//    /* 9 */ { "Outer Ra'Kaznar [U]","Water Elemental" },
    /* 9 */ { "Sea Serpent Grotto","Water Elemental" },
    /* 10 */ { "Carpenters' Landing","Water Elemental" },
    /* 10 */ { "Pso'Xja","Ice Elemental" },
    /* 10 */ { "Reisenjima","Thunder Elemental" },
    /* 10 */ { "The Boyahda Tree","Thunder Elemental" },
    /* 10 */ { "Xarcabard","Ice Elemental" },
//    /* 12 */ { "Ve'Lugannon Palace","Air Elemental" },
//    /* 12 */ { "Ve'Lugannon Palace","Earth Elemental" },
    /* 17 */ { "Uleguerand Range","Ice Elemental" },
    /* 20 */ { "The Boyahda Tree","Water Elemental" }
};

static void UpdateZoneElementals(void) {

    for (uint32_t j = 0; j < sizeof(ZoneElementals)/sizeof(ZoneElementals[0]); j++) {
        struct zone_elementals_t* ele = &ZoneElementals[j];

        for (enum ELEMENT e = NONE; e <= DARK; e++) {
            ele->elements[e] = NONE;
        }
    }

    for (uint32_t i = 0; i < sizeof(ZoneElementalData)/sizeof(ZoneElementalData[0]); i++) {
        const struct zone_elemental_data_t* data = &ZoneElementalData[i];

        uint32_t zoneId = 0;

        for (uint32_t j = 0; j < sizeof(zoneNames)/sizeof(zoneNames[0]); j++) {
            const struct zone_name_t* zone = &zoneNames[j];

            if (strcmp(data->ZoneName, zone->name) == 0) {
                zoneId = zone->id;
                break;
            }
        }

        if (zoneId == 0) {
            fprintf(stderr, "ZONE \"%s\" NOT FOUND", data->ZoneName);
        }

        if (zoneId != 0) {
            for (uint32_t j = 0; j < sizeof(ZoneElementals)/sizeof(ZoneElementals[0]); j++) {
                struct zone_elementals_t* ele = &ZoneElementals[j];

                if (zoneId == ele->id) {
                    enum ELEMENT element = NONE;
                    for (enum ELEMENT e = NONE; e <= DARK; e++) {
                        if (strcmp(data->ElementalName, ElementalNames[e]) == 0) {
                            element = e;
                            break;
                        }
                    }

                    if (element == NONE) {
                        fprintf(stderr, "ZONE \"%s\" ELEMENTAL \"%s\" NOT FOUND\n",
                            data->ZoneName,
                            data->ElementalName);
                    }

                    if (element != NONE) {
                        ele->elements[element] = element;
                    }
                    break;
                }
            }
        }
    }
}

#include <time.h>

#define VTIME_BASEDATE          1009810800              // unix epoch - 1009810800 = se epoch (in earth seconds)
#define VTIME_YEAR                      518400                  // 360 * GameDay
#define VTIME_MONTH                     43200                   // 30 * GameDay
#define VTIME_WEEK                      11520                   // 8 * GameDay
#define VTIME_DAY                       1440                    // 24 hours * GameHour
#define VTIME_HOUR                      60                              // 60 minutes

static uint32_t GetVanaTime(time_t t)
{
    //all functions/variables for in game time should be derived from this
    return (uint32_t)t - VTIME_BASEDATE;
}

static void PrintVanaClock(int vanaTime)
{
    int m_vanaDate  = (uint32_t)(vanaTime / 60.0 * 25) + 886 * VTIME_YEAR; //convert vana time (from SE epoch in earth s

    int m_vYear = (uint32_t)( m_vanaDate / VTIME_YEAR);
    int m_vMon  = (uint32_t)((m_vanaDate / VTIME_MONTH) % 12) + 1;
    int m_vDate = (uint32_t)((m_vanaDate / VTIME_DAY) % 30 ) + 1;
    int m_vDay  = (uint32_t)((m_vanaDate % VTIME_WEEK)  / VTIME_DAY);
    int m_vHour = (uint32_t)((m_vanaDate % VTIME_DAY)   / VTIME_HOUR);
    int m_vMin  = (uint32_t)( m_vanaDate % VTIME_HOUR);

    printf("%d/%d/%d %d %d:%02d\n", m_vYear, m_vMon, m_vDate, m_vDay, m_vHour, m_vMin);
}

static int FormatVanaDate(char* tmp, size_t size, int vanaTime)
{
    int m_vYear = (uint32_t)( vanaTime / VTIME_YEAR);
    int m_vMon  = (uint32_t)((vanaTime / VTIME_MONTH) % 12) + 1;
    int m_vDate = (uint32_t)((vanaTime / VTIME_DAY) % 30 ) + 1;
    //int m_vDay  = (uint32_t)((m_vanaDate % VTIME_WEEK)  / VTIME_DAY);
    //int m_vHour = (uint32_t)((m_vanaDate % VTIME_DAY)   / VTIME_HOUR);
    //int m_vMin  = (uint32_t)( m_vanaDate % VTIME_HOUR);

    return snprintf(tmp, size, "%d/%d/%d", m_vYear, m_vMon, m_vDate);
}

static uint32_t dehex(char s) {
    if (isdigit(s)) {
        return s - '0';
    }

    if (isupper(s)) {
        return 10 + s - 'A';
    }

    return 10 + s - 'a';
}

static uint32_t crapola(const char* s) {

    uint32_t a = dehex(s[0]);
    uint32_t b = dehex(s[1]);
    uint32_t c = dehex(s[2]);
    uint32_t d = dehex(s[3]);

    return (a << 4) + b + (c << 12) + (d << 8);
}

int main(int argc, char* argv[]) {

    if (argc != 5 && argc != 6 && argc != 7) {
        fprintf(stderr, "usage: %s <region0.dat> <weather0.dat> <region100.dat> <weather100.dat>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    uint32_t region0Len;
    uint32_t weather0Len;
    uint32_t region100Len;
    uint32_t weather100Len;

    // ROM/27/77.DAT
    uint8_t* region0 = ReadFile(argv[1], &region0Len);

    // ROM/27/78.DAT
    uint8_t* weather0 = ReadFile(argv[2], &weather0Len);

    // Zones 100+ were moved to new DATs to support additonal zones
    // ROM/188/66.DAT
    uint8_t* region100 = ReadFile(argv[3], &region100Len);

    // ROM/188/67.DAT
    uint8_t* weather100 = ReadFile(argv[4], &weather100Len);

    uint32_t requestedZoneId = 0;

    if (argc >= 6) {
        requestedZoneId = atoi(argv[5]);
    }

    uint32_t useDsp = 0;

    if (argc >= 7 && strcmp(argv[6], "dsp") == 0) {
        useDsp = 1;
    }

    // Overwrite the error-ridden elemental table.
    UpdateZoneElementals();

#if 0
    // Weather NPC behaves as such:
    time_t t = time(NULL);
    uint32_t vanaTime = GetVanaTime(t);
    PrintVanaClock(vanaTime);

    uint32_t weatherIdx = (vanaTime / 3456) % 0x870;

    for (uint32_t i = 0; i < sizeof(zoneNames)/sizeof(zoneNames[0]); i++) {
        const struct zone_name_t* zone = &zoneNames[i];

        uint32_t region = 0;
        uint32_t offset = 0;
        uint8_t* ptr = NULL;

        if (zone->id >= 100) {
            region = region100[zone->id - 100];
            offset = weatherIdx * 3 + region * 0x1950;
            ptr = ptr8(weather100, offset);
        } else {
            region = region0[zone->id];
            offset = weatherIdx * 3 + region * 0x1950;
            ptr = ptr8(weather0, offset);
        }

       printf("%3u %32s: today: %13s, chance: %13s, slight: %13s; tomorrow: %13s, chance: %13s, slight: %13s; day after: %13s, chance: %13s, slight: %13s\n",
            zone->id,
            zone->name,
            ptr[0] != 0xff ? weatherNames[ptr[0]] : "n/a",
            ptr[1] != 0xff ? weatherNames[ptr[1]] : "n/a",
            ptr[2] != 0xff ? weatherNames[ptr[2]] : "n/a",
            ptr[3] != 0xff ? weatherNames[ptr[3]] : "n/a",
            ptr[4] != 0xff ? weatherNames[ptr[4]] : "n/a",
            ptr[5] != 0xff ? weatherNames[ptr[5]] : "n/a",
            ptr[6] != 0xff ? weatherNames[ptr[6]] : "n/a",
            ptr[7] != 0xff ? weatherNames[ptr[7]] : "n/a",
            ptr[8] != 0xff ? weatherNames[ptr[8]] : "n/a");
    }
#endif

#if 0
    for (uint32_t i = 0; i < 299; i++) {
        uint32_t region = 0;
        uint32_t offset = 0;
        uint8_t* ptr = NULL;

        if (i >= 100) {
            region = region100[i - 100];
            offset = region * 0x1950;
            ptr = ptr8(weather100, offset);
        } else {
            region = region0[i];
            offset = region * 0x1950;
            ptr = ptr8(weather0, offset);
        }

        printf("INSERT INTO `zone_weather` VALUES (%u,0x", i);

        uint32_t prevWeather = 0;

        for (uint32_t j = 0; j < 0x870; j++) {

            uint32_t a = ptr[0] != 0xff ? ptr[0] : 0;
            uint32_t b = ptr[1] != 0xff ? ptr[1] : a;
            uint32_t c = ptr[2] != 0xff ? ptr[2] : b;

            uint16_t x = (a << 10) | (b << 5) | (c << 0);

            if (x == prevWeather) {
                x = 0;
            } else {
                prevWeather = x;
            }

            printf("%02X%02X", x & 0xff, (x >> 8) & 0xff);
            ptr += 3;
        }
        printf(");\n");
    }
#endif

#if 1
    for (uint32_t i = 0; (useDsp == 0) && i < sizeof(zoneNames)/sizeof(zoneNames[0]); i++) {
        const struct zone_name_t* zone = &zoneNames[i];

        char tmpbuf[1024];
        FILE* txtfp;

        if (requestedZoneId != 0) {
            if (zone->id != requestedZoneId) {
                continue;
            }

            txtfp = stdout;
        } else {
            snprintf(tmpbuf, sizeof(tmpbuf), "Weather - %u - %s (Retail).txt", zone->id, zone->name);

            txtfp = fopen(tmpbuf, "wt");
        }

        fprintf(txtfp, "%-15s %-15s %-15s %-15s\n", "Date",
                "Normal",
                "Common",
                "Rare"
                );

        uint32_t WeatherTally[9];
        memset(WeatherTally, 0, sizeof(WeatherTally));

        // Six years of weather data
        for (uint32_t vanaTime = 1332 * 518400; vanaTime < 1338 * 518400; vanaTime += VTIME_DAY) {
            uint32_t weatherIdx = (vanaTime / 1440) % 0x870;

            uint32_t region = 0;
            uint32_t offset = 0;
            uint8_t* ptr = NULL;

            if (zone->id >= 100) {
                region = region100[zone->id - 100];
                offset = weatherIdx * 3 + region * 0x1950;
                ptr = ptr8(weather100, offset);
            } else {
                region = region0[zone->id];
                offset = weatherIdx * 3 + region * 0x1950;
                ptr = ptr8(weather0, offset);
            }

            FormatVanaDate(tmpbuf, sizeof(tmpbuf), vanaTime);

            fprintf(txtfp, "%-15s %-15s %-15s %-15s\n", tmpbuf,
                    ptr[0] != 0xff ? weatherNames[ptr[0]] : "",
                    ptr[1] != 0xff ? weatherNames[ptr[1]] : "",
                    ptr[2] != 0xff ? weatherNames[ptr[2]] : "");

            enum ELEMENT WeatherElement50 = weatherToElement[ptr[0]];
            enum ELEMENT WeatherElement35 = ptr[1] != 0xff ? weatherToElement[ptr[1]] : NONE;
            enum ELEMENT WeatherElement15 = ptr[2] != 0xff ? weatherToElement[ptr[2]] : NONE;

            // 50/35/15 estimate from DSP.
            WeatherTally[WeatherElement50] += 50;
            if (ptr[1] != 0xff) {
                WeatherTally[WeatherElement35] += 35;
            } else {
                WeatherTally[WeatherElement50] += 35;
            }
            if (ptr[2] != 0xff) {
                WeatherTally[WeatherElement15] += 15;
            } else {
                WeatherTally[WeatherElement50] += 15;
            }
        }

        fprintf(txtfp, "\n# Weather Statistics:\n");
        for (enum ELEMENT e = NONE; e <= DARK; e++) {
            if (WeatherTally[e] > 0) {
                double pct = (double) WeatherTally[e] / (double) (360 * 6);

                if (pct >= 100.0) {
                    fprintf(txtfp, "# %s Weather: 100%%\n", ElementNames[e]);
                } else if (pct >= 0.1) {
                    fprintf(txtfp, "# %s Weather: %.1f%%\n", ElementNames[e], pct);
                } else {
                    fprintf(txtfp, "# %s Weather: <0.1%%\n", ElementNames[e]);
                }
            } else {
                fprintf(txtfp, "# %s Weather: 0%%\n", ElementNames[e]);
            }
        }

        for (uint32_t j = 0; i < sizeof(ZoneElementals)/sizeof(ZoneElementals[0]); j++) {
            const struct zone_elementals_t* ele = &ZoneElementals[j];

            if (ele->id == zone->id) {
                uint32_t hasElementals = 0;

                for (enum ELEMENT e = NONE; e <= DARK; e++) {
                    if (ele->elements[e] != NONE) {
                        hasElementals = 1;
                        break;
                    }
                }

                if (hasElementals != 0) {
                    fprintf(txtfp, "#\n# Elementals Info:\n");
                    for (enum ELEMENT e = NONE; e <= DARK; e++) {
                        if (ele->elements[e] != NONE) {
                            fprintf(txtfp, "# %s: %s\n",
                                ElementalNames[e],
                                WeatherTally[e] > 0 ? "YES" : "NO");
                        }
                    }
                }
                break;
            }
        }

        if (txtfp != stdout) {
            fclose(txtfp);
        }
    }
#endif

#if 0
    for (uint32_t i = 0; i < sizeof(zoneNames)/sizeof(zoneNames[0]); i++) {
        const struct zone_name_t* zone = &zoneNames[i];

        char tmpbuf[1024];

        snprintf(tmpbuf, sizeof(tmpbuf), "csv/Weather - %u - %s (Retail).csv", zone->id, zone->name);

        FILE* csvfp = fopen(tmpbuf, "wt");

        fprintf(csvfp, "%s,%s,%s,%s\n", "date",
                "normal",
                "common",
                "rare"
                );

        // Six years of weather data
        for (uint32_t vanaTime = 1332 * 518400; vanaTime < 1338 * 518400; vanaTime += VTIME_DAY) {
            uint32_t weatherIdx = (vanaTime / 1440) % 0x870;

            uint32_t region = 0;
            uint32_t offset = 0;
            uint8_t* ptr = NULL;

            if (zone->id >= 100) {
                region = region100[zone->id - 100];
                offset = weatherIdx * 3 + region * 0x1950;
                ptr = ptr8(weather100, offset);
            } else {
                region = region0[zone->id];
                offset = weatherIdx * 3 + region * 0x1950;
                ptr = ptr8(weather0, offset);
            }

            FormatVanaDate(tmpbuf, sizeof(tmpbuf), vanaTime);

            fprintf(csvfp, "%s,%s,%s,%s\n", tmpbuf,
                    ptr[0] != 0xff ? weatherNames[ptr[0]] : "",
                    ptr[1] != 0xff ? weatherNames[ptr[1]] : "",
                    ptr[2] != 0xff ? weatherNames[ptr[2]] : "");
        }

        fclose(csvfp);
    }
#endif

#if 1
    for (uint32_t i = 0; (useDsp == 1) && i < sizeof(zoneNames)/sizeof(zoneNames[0]); i++) {
        const struct zone_name_t* zone = &zoneNames[i];

        char tmpbuf[1024];
        FILE* txtfp;

        if (requestedZoneId != 0) {
            if (zone->id != requestedZoneId) {
                continue;
            }

            txtfp = stdout;
        } else {
            snprintf(tmpbuf, sizeof(tmpbuf), "Weather - %u - %s (DSP).txt", zone->id, zone->name);

            txtfp = fopen(tmpbuf, "wt");
        }

        fprintf(txtfp, "%-15s %-15s %-15s %-15s\n", "Date",
                "Normal",
                "Common",
                "Rare"
                );

        uint32_t WeatherTally[9];
        memset(WeatherTally, 0, sizeof(WeatherTally));

        // Six years of weather data
        uint32_t prevWeather = 0;

        for (uint32_t vanaTime = 1332 * 518400; vanaTime < 1338 * 518400; vanaTime += VTIME_DAY) {
            uint32_t weatherIdx = (vanaTime / 1440) % 0x870;

            uint32_t region = 0;
            uint32_t offset = 0;
            const char* ptr = DSPWeather[zone->id] + weatherIdx * 4;

            uint32_t x = crapola(ptr);

            // Decode the DSP weather
            if (x > 0) {
                prevWeather = x;
            } else {
                x = prevWeather;
            }

            FormatVanaDate(tmpbuf, sizeof(tmpbuf), vanaTime);

            uint32_t a = (x >> 10) & 0x1f;
            uint32_t b = (x >> 5) & 0x1f;
            uint32_t c = (x >> 0) & 0x1f;

            fprintf(txtfp, "%-15s %-15s %-15s %-15s\n", tmpbuf,
                    a != 0 ? weatherNames[a] : "",
                    b != 0 ? weatherNames[b] : "",
                    c != 0 ? weatherNames[c] : "");

            enum ELEMENT WeatherElement50 = weatherToElement[a];
            enum ELEMENT WeatherElement35 = b != 0 ? weatherToElement[b] : NONE;
            enum ELEMENT WeatherElement15 = c != 0 ? weatherToElement[c] : NONE;

            // 50/35/15 estimate from DSP.
            WeatherTally[WeatherElement50] += 50;
            if (b != 0) {
                WeatherTally[WeatherElement35] += 35;
            } else {
                WeatherTally[WeatherElement50] += 35;
            }
            if (c != 0) {
                WeatherTally[WeatherElement15] += 15;
            } else {
                WeatherTally[WeatherElement50] += 15;
            }
        }

        fprintf(txtfp, "\n# Weather Statistics:\n");
        for (enum ELEMENT e = NONE; e <= DARK; e++) {
            if (WeatherTally[e] > 0) {
                double pct = (double) WeatherTally[e] / (double) (360 * 6);

                if (pct >= 100.0) {
                    fprintf(txtfp, "# %s Weather: 100%%\n", ElementNames[e]);
                } else if (pct >= 0.1) {
                    fprintf(txtfp, "# %s Weather: %.1f%%\n", ElementNames[e], pct);
                } else {
                    fprintf(txtfp, "# %s Weather: <0.1%%\n", ElementNames[e]);
                }
            } else {
                fprintf(txtfp, "# %s Weather: 0%%\n", ElementNames[e]);
            }
        }

        for (uint32_t j = 0; i < sizeof(ZoneElementals)/sizeof(ZoneElementals[0]); j++) {
            const struct zone_elementals_t* ele = &ZoneElementals[j];

            if (ele->id == zone->id) {
                uint32_t hasElementals = 0;

                for (enum ELEMENT e = NONE; e <= DARK; e++) {
                    if (ele->elements[e] != NONE) {
                        hasElementals = 1;
                        break;
                    }
                }

                if (hasElementals != 0) {
                    fprintf(txtfp, "#\n# Elementals Info:\n");
                    for (enum ELEMENT e = NONE; e <= DARK; e++) {
                        if (ele->elements[e] != NONE) {
                            fprintf(txtfp, "# %s: %s\n",
                                ElementalNames[e],
                                WeatherTally[e] > 0 ? "YES" : "NO");
                        }
                    }
                }
                break;
            }
        }

        if (txtfp != stdout) {
            fclose(txtfp);
        }
    }
#endif

    return 0;
}
