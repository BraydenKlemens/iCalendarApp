
/*
Bayden Klemens
02/04/2019
*/

#ifndef HELPER_H
#define HELPER_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include "CalendarParser.h"
#include "LinkedListAPI.h"

void free2DArray(char **infoArray);

Property* parseProperty(char *string);

DateTime parseDateTime(char *string);

bool checkDigits(char *string);

bool checkDT(char *string);

bool isStringNum(char *string);

char *propListToString(List *list);

char* alarmListToString(List* list);

char* eventListToString(List* list);

char *dateToString(DateTime dt);

char* findPropertyDescription(List *list, char *propName);

//char *addEscapeChar(char *string);

char *constToString(const char *string);

//validation

ICalErrorCode validateAlarm(Alarm *alarm);

int validateAlarmProperty(Property *prop, List *list);

ICalErrorCode validateEvent(Event *event);

int validateEventProperty(Property *prop, List *list);

int validateProperty(Property *prop);

int validateDateTime(DateTime dt);

int oneOccurence(char *name, List *list);

int findProperty(char *propName, List *list);

int validateCalendarProperty(Property *prop, List *list);


#endif


