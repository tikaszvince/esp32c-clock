#include "Arduino.h"
#include <TzDbLookup.h>
#include <pgmspace.h>

#include "timezones.h"

struct TimezoneEntry {
  const char* city;
  const char* override;
};
struct ContinentGroup {
  const char* prefix;
  const TimezoneEntry* zones;
  int count;
};

template<int N>
static ContinentGroup makeGroup(const char* prefix, const TimezoneEntry (&zones)[N]) {
  return {prefix, zones, N};
}

static const TimezoneEntry asiaZones[] = {
  {"Baghdad",    nullptr},
  {"Bangkok",    nullptr},
  {"Beirut",     nullptr},
  {"Colombo",    nullptr},
  {"Dhaka",      nullptr},
  {"Dubai",      nullptr},
  {"Hong Kong",  "Hong_Kong"},
  {"Jakarta",    nullptr},
  {"Jerusalem",  nullptr},
  {"Kabul",      nullptr},
  {"Karachi",    nullptr},
  {"Kathmandu",  nullptr},
  {"Kolkata",    nullptr},
  {"Kuala Lumpur", "Kuala_Lumpur"},
  {"Kuwait",     nullptr},
  {"Makassar",   nullptr},
  {"Manila",     nullptr},
  {"Nicosia",    nullptr},
  {"Riyadh",     nullptr},
  {"Seoul",      nullptr},
  {"Shanghai",   nullptr},
  {"Singapore",  nullptr},
  {"Taipei",     nullptr},
  {"Tehran",     nullptr},
  {"Tokyo",      nullptr},
  {"Vladivostok",nullptr},
  {"Yakutsk",    nullptr},
  {"Yekaterinburg", nullptr},
};

static const TimezoneEntry europeZones[] = {
  {"Amsterdam",  nullptr},
  {"Athens",     nullptr},
  {"Belgrade",   nullptr},
  {"Berlin",     nullptr},
  {"Brussels",   nullptr},
  {"Bucharest",  nullptr},
  {"Budapest",   nullptr},
  {"Copenhagen", nullptr},
  {"Dublin",     nullptr},
  {"Helsinki",   nullptr},
  {"Kiev",       nullptr},
  {"Lisbon",     nullptr},
  {"London",     nullptr},
  {"Madrid",     nullptr},
  {"Moscow",     nullptr},
  {"Oslo",       nullptr},
  {"Paris",      nullptr},
  {"Prague",     nullptr},
  {"Riga",       nullptr},
  {"Rome",       nullptr},
  {"Sofia",      nullptr},
  {"Stockholm",  nullptr},
  {"Tallinn",    nullptr},
  {"Vienna",     nullptr},
  {"Vilnius",    nullptr},
  {"Warsaw",     nullptr},
  {"Zurich",     nullptr},
};

static const TimezoneEntry americaZones[] = {
  {"Anchorage",    nullptr},
  {"Bogota",       nullptr},
  {"Buenos Aires", "Argentina/Buenos_Aires"},
  {"Caracas",      nullptr},
  {"Chicago",      nullptr},
  {"Denver",       nullptr},
  {"Halifax",      nullptr},
  {"Havana",       nullptr},
  {"Houston",      "Chicago"},
  {"Lima",         nullptr},
  {"Los Angeles",  nullptr},
  {"Mexico City",  nullptr},
  {"Miami",        "New_York"},
  {"New York",     "New_York"},
  {"Phoenix",      nullptr},
  {"Santiago",     nullptr},
  {"Sao Paulo",    nullptr},
  {"Toronto",      nullptr},
  {"Vancouver",    nullptr},
  {"Winnipeg",     nullptr},
};

static const TimezoneEntry africaZones[] = {
  {"Abidjan",      nullptr},
  {"Accra",        nullptr},
  {"Addis Ababa",  "Addis_Ababa"},
  {"Algiers",      nullptr},
  {"Cairo",        nullptr},
  {"Casablanca",   nullptr},
  {"Dar es Salaam","Dar_es_Salaam"},
  {"Johannesburg", nullptr},
  {"Kampala",      nullptr},
  {"Khartoum",     nullptr},
  {"Lagos",        nullptr},
  {"Maputo",       nullptr},
  {"Nairobi",      nullptr},
  {"Tripoli",      nullptr},
  {"Tunis",        nullptr},
};

static const TimezoneEntry pacificZones[] = {
  {"Auckland",    nullptr},
  {"Fiji",        nullptr},
  {"Guam",        nullptr},
  {"Honolulu",    nullptr},
  {"Midway",      nullptr},
  {"Noumea",      nullptr},
  {"Pago Pago",   "Pago_Pago"},
  {"Port Moresby","Port_Moresby"},
  {"Tahiti",      nullptr},
  {"Tongatapu",   nullptr},
};

static const TimezoneEntry australiaZones[] = {
  {"Adelaide",  nullptr},
  {"Brisbane",  nullptr},
  {"Darwin",    nullptr},
  {"Hobart",    nullptr},
  {"Melbourne", nullptr},
  {"Perth",     nullptr},
  {"Sydney",    nullptr},
};

static const ContinentGroup continents[] = {
  makeGroup("Europe", europeZones),
  makeGroup("America", americaZones),
  makeGroup("Asia", asiaZones),
  makeGroup("Africa", africaZones),
  makeGroup("Pacific", pacificZones),
  makeGroup("Australia",australiaZones),
};

static void buildIana(char* buf, int bufSize, const char* continent, const char* city, const char* override) {
  if (override != nullptr) {
    snprintf(buf, bufSize, "%s/%s", continent, override);
  }
  else {
    // Copy city with spaces replaced by underscores
    snprintf(buf, bufSize, "%s/", continent);
    int pos = strlen(buf);
    for (int i = 0; city[i] != '\0' && pos < bufSize - 1; i++) {
      buf[pos++] = (city[i] == ' ') ? '_' : city[i];
    }
    buf[pos] = '\0';
  }
}

static void appendOptgroup(char* buf, int bufSize, int& pos, const char* label) {
  pos += snprintf(
    buf + pos,
    bufSize - pos,
    "<optgroup label=\"%s\">",
    label
  );
}

static void appendOption(char* buf, int bufSize, int& pos, const char* value, const char* label, bool selected) {
  pos += snprintf(
    buf + pos,
    bufSize - pos,
    "<option value=\"%s\"%s>%s</option>",
    value,
    selected ? " selected" : "",
    label
  );
}

void buildTimezoneSelect(
  const char* name,
  const char* currentPosix,
  char* buf,
  int bufSize
) {
  int pos = 0;

  pos += snprintf(buf + pos, bufSize - pos, "<label for=\"%s\">Timezone</label>", name);
  pos += snprintf(buf + pos, bufSize - pos, "<select name=\"%s\">", name);
  for (int c = 0; c < sizeof(continents) / sizeof(continents[0]); c++) {
    const ContinentGroup& group = continents[c];
    appendOptgroup(buf, bufSize, pos, group.prefix);

    for (int i = 0; i < group.count; i++) {
      char iana[48];
      buildIana(iana, sizeof(iana), group.prefix, group.zones[i].city, group.zones[i].override);
      const char* posix = TzDbLookup::getPosix(iana);
      if (posix == nullptr) {
        continue;
      }
      bool sel = strcmp(posix, currentPosix) == 0;
      appendOption(buf, bufSize, pos, posix, group.zones[i].city, sel);
    }
    pos += snprintf(buf + pos, bufSize - pos, "</optgroup>");
  }
  pos += snprintf(buf + pos, bufSize - pos, "</select>");
}
