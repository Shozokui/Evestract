#ifndef DATID_H
#define DATID_H

#define MAX_LANGUAGES 2 // JP, EN
#define MAX_LANGUAGES_OLD 4 // JP, EN, DE, FR

#define MAX_LOCALIZED_RESOURCES 109 // see BaseFileIds

int GetBaseFileId(int languageId, int index);

int GetEntityListFileId(int zoneId);
int GetEventMessageFileId(int languageId, int zoneId);
int GetEventFileId(int zoneId);

#endif
