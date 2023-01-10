/*
Bayden Klemens
02/04/2019
*/

#include "helper.h"

void free2DArray(char **infoArray){

    //deallcoate all memory in the 2d arrays
    for(int i = 0; i < 1000; i++){
        free(infoArray[i]);
    }

    free(infoArray);
}


Property* parseProperty(char *string){

    //allocate for the block
    Property *prop = malloc(sizeof(Property));

    char *token = NULL;

    if(string == NULL){
        free(prop);
        return NULL;
    }

    //get the name
    token = strtok(string, ":;");

    if(token == NULL){
        free(prop);
        return NULL;
    }

    strcpy(prop->propName, token);

    //get the description but realloc for its size
    token = strtok(NULL, "\0");

    if(token == NULL){
        free(prop);
        return NULL;
    }

    prop = realloc(prop, sizeof(Property) + strlen(token) + 50);

    strcpy(prop->propDescr, token);

    return prop;
}

DateTime parseDateTime(char *string){

    DateTime dt;
    char *token = NULL;
    strcpy(dt.date, "");
    strcpy(dt.time, "");

    token = strtok(string, "T");

    if(token != NULL && strlen(token) == 8 && isStringNum(token)){
        strcpy(dt.date, token);
    }

    token = strtok(NULL,"\0");

    if(token != NULL){
        if(token[strlen(token) - 1] == 'Z'){
            dt.UTC = true;
            token[strlen(token) - 1] = '\0';
        }else{
            dt.UTC = false;
        }

        if(strlen(token) == 6){
            if(isStringNum(token)){
                strcpy(dt.time, token);
            }
        }	
    }

    return dt;
}

bool checkDigits(char *string){

    int counter = 0;
    int decimal = 0;

    for(int i = 0; i < strlen(string); i++){
        if(isdigit(string[i])){
            counter++;
        }else if(string[i] == '.'){
            if(decimal == 0){
                counter++;
                decimal = 1;
            }else{
                return false;
            }
        }else{
            return false;
        }
    }

    if(counter == strlen(string)){
        return true;
    }

    return false;
}

bool checkDT(char *string){

    for(int i = 0; i < strlen(string); i++){
        if(string[i] == 'T'){
            return true;
        }
    }

    return false;
}

bool isStringNum(char *string){

    for(int i = 0; i < strlen(string); i++){

        if(!(isdigit(string[i]))){
            return false;
        }
    }

    return true;
}

/***************************************************************************************/

//write calendar to file

char* propListToString(List* list){
    ListIterator iter = createIterator(list);
    char* string;
        
    string = (char*)malloc(sizeof(char));
    strcpy(string, "");
    
    void* elem;
    while((elem = nextElement(&iter)) != NULL){

        Property *tempProp = (Property*)elem;
        char *currDescr = malloc((strlen(tempProp->propName) + strlen(tempProp->propDescr) + 50));
        sprintf(currDescr, "%s:%s\r\n", tempProp->propName, tempProp->propDescr);

        string = (char*)realloc(string, strlen(string)+50+strlen(currDescr));
        strcat(string, currDescr);

        free(currDescr);
    }
    
    return string;
}

char* alarmListToString(List* list){
    ListIterator iter = createIterator(list);
    char* string;
        
    string = (char*)malloc(sizeof(char));
    strcpy(string, "");
    
    void* elem;
    while((elem = nextElement(&iter)) != NULL){

        Alarm *tempAlarm = (Alarm*)elem;
        char *alarmProps = propListToString(tempAlarm->properties);
        char *currDescr = malloc(strlen(tempAlarm->action) + strlen(tempAlarm->trigger) + strlen(alarmProps) + 150);

        sprintf(currDescr, "BEGIN:VALARM\r\nACTION:%s\r\nTRIGGER:%s\r\n%sEND:VALARM\r\n", tempAlarm->action, tempAlarm->trigger, alarmProps);
        string = (char*)realloc(string, strlen(string)+50+strlen(currDescr));
        strcat(string, currDescr);
        
        free(currDescr);
        free(alarmProps);
    }
    
    return string;
}

char* eventListToString(List* list){
    ListIterator iter = createIterator(list);
    char* string;
        
    string = (char*)malloc(sizeof(char));
    strcpy(string, "");
    
    void* elem;
    while((elem = nextElement(&iter)) != NULL){

        Event *tempEvent = (Event*)elem;
        char *eventProps = propListToString(tempEvent->properties);
        char *eventAlarms = alarmListToString(tempEvent->alarms);
        char *creationDate = dateToString(tempEvent->creationDateTime);
        char *startDate = dateToString(tempEvent->startDateTime);

        char *currDescr = malloc(strlen(tempEvent->UID) + strlen(creationDate) + strlen(startDate) + strlen(eventProps) + strlen(eventAlarms) + 250);
        sprintf(currDescr, "BEGIN:VEVENT\r\nUID:%s\r\nDTSTAMP:%s\r\nDTSTART:%s\r\n%s%sEND:VEVENT\r\n",tempEvent->UID, creationDate, startDate, eventProps, eventAlarms);

        string = (char*)realloc(string, strlen(string)+100+strlen(currDescr));
        strcat(string, currDescr);
        
        free(currDescr);
        free(eventProps);
        free(eventAlarms);
        free(creationDate);
        free(startDate);
    }
    
    return string;
}

char *dateToString(DateTime dt){

    char *string = malloc(strlen(dt.date) + strlen(dt.time) + 10);

    if(dt.UTC == true){
        sprintf(string, "%sT%sZ", dt.date, dt.time);
    }else{
        sprintf(string, "%sT%s", dt.date, dt.time);
    }

    return string;
}

/***************************************************************************************/
//validate calendar

int oneOccurence(char *name, List *list){

    Node *temp = list->head;
    int count = 0;

    while(temp != NULL){	
        Property *listProp = (Property*)temp->data;
        if(strcmp(name, listProp->propName) == 0){
            count++;
            if(count > 1){
                return 1;
            }
        }
        temp = temp->next;	
    }

    return 0;
}

int validateProperty(Property *prop){

    if(prop == NULL){
        return 1;
    }

    if(strlen(prop->propName) <= 199 && strcmp(prop->propName, "") != 0){	
        if(strcmp(prop->propDescr, "") != 0){
            return 0;
        }
    }
    return 1;
}

int validateDateTime(DateTime dt){

    if(strcmp(dt.date, "") != 0 && strcmp(dt.time, "") != 0){
        if(strlen(dt.date) <= 8 && strlen(dt.time) <= 6){
            return 0;
        }
    }
    return 1;
}

ICalErrorCode validateAlarm(Alarm *alarm){

    if(alarm->properties != NULL){
        ListIterator iter = createIterator(alarm->properties);
        void* elem;
        int length = getLength(alarm->properties);
        int count = 0;

        if(strlen(alarm->action) <= 199 && strcmp(alarm->action, "AUDIO") == 0){
            if(strcmp(alarm->trigger, "") != 0 && alarm->trigger != NULL){
                //ensure that validation when there is no properties
                if(length == 0){
                    return OK;
                }
                //validate each property on the list
                while((elem = nextElement(&iter)) != NULL){
                    Property *prop = (Property*)elem;
                    if(validateProperty(prop) == 0){
                        if(validateAlarmProperty(prop, alarm->properties) == 0){
                            count++;
                            if(count == length){
                                return OK;
                            }
                        }
                    }
                }
            }
        }
    }
    return INV_ALARM;
}

int validateAlarmProperty(Property *prop, List *list){

    if(strcmp(prop->propName, "DURATION") == 0){
        if(oneOccurence(prop->propName, list) == 0){
            if(findProperty("REPEAT", list) == 0){
                if(oneOccurence("REPEAT", list) == 0){
                    return 0;
                }
            }
        }
    }

    if(strcmp(prop->propName, "REPEAT") == 0){
        if(oneOccurence(prop->propName, list) == 0){
            if(findProperty("DURATION", list) == 0){
                if(oneOccurence("DURATION", list) == 0){
                    return 0;
                }
            }
        }
    }

    if(strcmp(prop->propName, "ATTACH") == 0){
        if(oneOccurence(prop->propName, list) == 0){
            return 0;
        }
    }

    //action and trigger return 1 since they should only be static props

    return 1;
}


ICalErrorCode validateEvent(Event *event){

    bool validAlarmList = false;
    bool validpropList = false;

    if(strlen(event->UID) <= 199 && strcmp(event->UID, "") != 0){
        if(validateDateTime(event->creationDateTime) == 0 && validateDateTime(event->startDateTime) == 0){
            if(event->properties != NULL && event->alarms != NULL){

                //validate property list
                ListIterator alarmIterator = createIterator(event->alarms);
                ListIterator propIterator = createIterator(event->properties);
                void* elem;
                int count = 0;
                int length = getLength(event->properties);

                if(count == length){
                    validpropList = true;
                }

                while((elem = nextElement(&propIterator)) != NULL){
                    Property *prop = (Property*)elem;
                    if(validateProperty(prop) == 0){
                        if(validateEventProperty(prop, event->properties) == 0){
                            count++;
                            if(count == length){
                                validpropList = true;
                            }
                        }
                    }
                }

                if(validpropList == false){
                    return INV_EVENT;
                }

                //validate alarm list *********************************************
                count = 0;
                length = getLength(event->alarms);

                if(count == length){
                    validAlarmList = true;
                }

                while((elem = nextElement(&alarmIterator)) != NULL){
                    Alarm *alarm = (Alarm*)elem;
                    if(validateAlarm(alarm) == OK){
                        count++;
                        if(count == length){
                            validAlarmList = true;
                        }
                    }else{
                        return INV_ALARM;
                    }
                }

                //check that they have successful lists
                if(validAlarmList == true && validpropList == true){
                    return OK;
                }
            }
        }
    }

    return INV_EVENT;
}

int validateEventProperty(Property *prop, List *list){

    char *name = prop->propName;
    //if dtstamp,uid and dtstart occur here return 1

    //One Occurence
    //3.8.7 after creatded last mod and sequence
    if(strcmp(name, "CLASS") == 0 || strcmp(name, "DESCRIPTION") == 0 || strcmp(name, "GEO") == 0 || strcmp(name, "LOCATION") == 0 || strcmp(name, "ORGANIZER") == 0 || strcmp(name, "PRIORITY") == 0 || strcmp(name, "STATUS") == 0 || strcmp(name, "SUMMARY") == 0 || strcmp(name, "TRANSP") == 0 || strcmp(name, "URL") == 0 || strcmp(name, "RECURRENCE-ID") == 0 || strcmp(name, "LAST-MODIFIED") == 0 || strcmp(name, "CREATED") == 0 || strcmp(name, "SEQUENCE") == 0){
        if(oneOccurence(prop->propName, list) == 0){
            return 0;
        }
    }

    if(strcmp(name, "ATTACH") == 0 || strcmp(name, "ATTENDEE") == 0 || strcmp(name, "CATEGORIES") == 0 || strcmp(name, "COMMENT") == 0 || strcmp(name, "CONTACT") == 0 || strcmp(name, "EXDATE") == 0 || strcmp(name, "RELATED") == 0 || strcmp(name, "RESOURCES") == 0 || strcmp(name, "RDATE") == 0 || strcmp(name, "RRULE") == 0){
        return 0;
    }

    if(strcmp(name, "DTEND") == 0){
        if(findProperty("DURATION", list) != 0){
            return 0;
        }
    }

    if(strcmp(name, "DURATION") == 0){
        if(findProperty("DTEND", list) != 0){
            return 0;
        }
    }

    return 1;
}

int validateCalendarProperty(Property *prop, List *list){

    char *name = prop->propName;

    if(strcmp(name, "CALSCALE") == 0 || strcmp(name, "METHOD") == 0){
        if(oneOccurence(prop->propName, list) == 0){
            return 0;
        }
    }

    return 1;
}


/***************************************************************************************/
//JSON strings helper fucntions

//used to search
char* findPropertyDescription(List *list, char *propName){

    if (list == NULL){
        return NULL;
    }

    ListIterator itr = createIterator(list);

    void* data = nextElement(&itr);
    while (data != NULL){

        Property *propData = (Property*)data;

        if (strcmp(propData->propName, propName) == 0){
            return propData->propDescr;
        }

        data = nextElement(&itr);
    }

    return "";
}

int findProperty(char *propName, List *list){

    ListIterator itr = createIterator(list);

    void* data = nextElement(&itr);
    while (data != NULL){

        Property *propData = (Property*)data;

        if (strcmp(propData->propName, propName) == 0){
            return 0;
        }

        data = nextElement(&itr);
    }

    return 1;
}

char *constToString(const char *string){

    char *newString = malloc(strlen(string) + 10);
    strcpy(newString, "");

    for(int i = 0; i < strlen(string); i++){

        char c = string[i];
        newString[i] = c; 
    }

    return newString;
}











