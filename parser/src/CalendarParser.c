/*
Bayden Klemens
02/04/2019
*/

#include "CalendarParser.h"
#include "helper.h"

/** Function to create a Calendar object based on the contents of an iCalendar file.
 *@pre File name cannot be an empty string or NULL.  File name must have the .ics extension.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid calendar has been created, its address was stored in the variable obj, and OK was returned
        or 
        An error occurred, the calendar was not created, all temporary memory was freed, obj was set to NULL, and the 
        appropriate error code was returned
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param fileName - a string containing the name of the iCalendar file
 *@param a double pointer to a Calendar struct that needs to be allocated
**/
ICalErrorCode createCalendar(char* fileName, Calendar** obj){

    //filename doesnt exist
    if(fileName == NULL){
        obj = NULL;
        return INV_FILE;
    }

    //does file exist
    if(access(fileName, F_OK) == -1){
        obj = NULL;
        return INV_FILE;
    }

    //count num lines for allocation
    int letter = 0;
    int numLines = 0;

    FILE *fp;
    fp = fopen(fileName, "r");
    if(fp != NULL){
        while(!(feof(fp))){
            letter = fgetc(fp);
            if(letter == '\n'){
                numLines++;
            }
        }
        fclose(fp);
        
        //empty string file
        if(numLines == 0){
            return INV_FILE;
        }
    }else{
        fclose(fp);
        obj = NULL;
        return INV_FILE;
    }

    //double pointer cannot be NULL
    if(obj == NULL){
        obj = NULL;
        return OTHER_ERROR;
    }

    //vars
    char buffer[3000];
    char **infoArray = NULL;
    char **tempArray = NULL;
    char *token = NULL;
    int tempArraySize = 0;
    int infoArraySize = 0;

    //FILE STUFF
    fp = fopen(fileName, "r");

    //file cant be opened, filename is empty, or doesnt have the ics extension
    if(fp != NULL && strcmp(fileName, "") != 0) {
        if(strcmp(strrchr(fileName, '.'), ".ics") != 0){
            fclose(fp);
            return INV_FILE;
        }
    }else {
        fclose(fp);
        obj = NULL;
        return INV_FILE;
    }

    //make calendar
    Calendar *cal = malloc(sizeof(Calendar));

    //allocate memory for the 2d array
    infoArray = malloc(sizeof(char*)*1000);
    tempArray = malloc(sizeof(char*)*1000);


    //make room for full length lines 1000+ bytes
    for(int i  = 0; i < 1000; i++){
        infoArray[i] = malloc(sizeof(char) * 2000);
        tempArray[i] = malloc(sizeof(char) * 2000);
    }

    //Put all information into a 2d array
    while(fgets(buffer, 3000, fp) != NULL){
        strcpy(tempArray[tempArraySize], buffer);
        tempArraySize++;
    }

    fclose(fp);

    //check for invalid line endings
    for (int i = 0; i < tempArraySize; i++){
        if(strstr(tempArray[i],"\r\n") == NULL){
            free2DArray(tempArray);
            free2DArray(infoArray);
            free(cal);
            obj =  NULL;
            cal = NULL;
            return INV_FILE;
        }
    }

    //get rid of new lines and returns - check line of text length
    for(int i  = 0; i < tempArraySize; i++){

        if(tempArray[i][strlen(tempArray[i]) - 1] == '\n'){
            tempArray[i][strlen(tempArray[i]) - 1] = '\0';
        }
        if(tempArray[i][strlen(tempArray[i]) - 1] == '\r'){
            tempArray[i][strlen(tempArray[i]) - 1] = '\0';
        }
    }

    //unfoldlines TODO - DUMP INTO NEW ARRAY 
    for(int i = 0; i < tempArraySize; i++){

        if(tempArray[i][0] == ' ' || tempArray[i][0] == '\t'){

            if(i != 0){
                int counter = 1;
                char *fold = malloc(sizeof(char)*1000);
                memcpy(fold, &tempArray[i][1], strlen(tempArray[i]));
                
                if(fold != NULL){
                    int num = strlen(fold);
                    fold[num] = '\0';
                }

                //dont append to a comment
                while(tempArray[i - counter][0] == ';' || tempArray[i - counter][0] == ' ' || tempArray[i - counter][0] == '\t'){
                    counter++;
                }
                strcat(tempArray[i - counter], fold);
                free(fold);
            }
        }
    }


    //NEW ARRAY WITHOUT COMMENTS OR NULL LINES
    for(int i = 0; i < tempArraySize; i++){
        if(tempArray[i][0] != ' ' && tempArray[i][0] != ';' && tempArray[i][0] != '\0' && tempArray[i][0] != '\t' && tempArray[i][0] != '\r'){
            strcpy(infoArray[infoArraySize], tempArray[i]);
            infoArraySize++;
        }
    }

    //free temp array no longer need it
    free2DArray(tempArray);

    //MALLLOCS BEFORE PARSE 
    // cal and infoArray

    //PARSE ***********************************************************************************

    //flag integers
    int i = 0;
    int alarmFlag = 0;
    int calFlag = 0;
    int verFlag = 0;
    int prodFlag = 0;
    int uidFlag = 0;
    int createDateFlag = 0;
    int startDateFlag = 0;
    int eventFlag = 0;
    int actionFlag = 0;
    int triggerFlag = 0;
    char *tempInfo = malloc(sizeof(char)*5000);

    //init scope
    cal->properties = NULL;
    cal->events = NULL;

    //parse information in 2D array and store in structs
    while(i < infoArraySize){

        token = strtok(infoArray[i], ":;");

        //dont parse antything until VCAL starts
        if(strcmp(token, "BEGIN") == 0){

            token = strtok(NULL, "\0");

            if(token == NULL){
                free2DArray(infoArray);
                free(tempInfo);
                deleteCalendar(cal);
                return INV_CAL;
            }

            if(strcmp(token, "VCALENDAR") == 0){

                calFlag = 1;
                i++;

                if(cal->properties == NULL){
                    cal->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
                }
                if(cal->events == NULL){
                    cal->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
                }

                strcpy(cal->prodID, "");

                //loop to find components of the calendar struct
                while(calFlag == 1 && i < infoArraySize){

                    strcpy(tempInfo, infoArray[i]);
                    token = strtok(infoArray[i], ":;");

                    //parse the version of the ICAL
                    if(strcmp(token, "VERSION") == 0){
                        //missing or not a number
                        token = strtok(NULL, "\0");
                        
                        if(token == NULL){
                            free2DArray(infoArray);
                            free(tempInfo);
                            deleteCalendar(cal);
                            cal = NULL; 
                            obj = NULL;
                            return INV_VER;
                        }else if(checkDigits(token) == false){
                            //free
                            free2DArray(infoArray);
                            free(tempInfo);
                            deleteCalendar(cal);
                            cal = NULL; 
                            obj = NULL;
                            return INV_VER;
                        }else{
                            if(verFlag == 0){
                                verFlag = 1;
                                cal->version = atof(token);
                                i++;
                                if(i >= infoArraySize){
                                    free2DArray(infoArray);
                                    free(tempInfo);
                                    deleteCalendar(cal);
                                    cal = NULL; 
                                    obj = NULL;
                                    return INV_CAL;
                                }
                            }else{
                                free2DArray(infoArray);
                                free(tempInfo);
                                deleteCalendar(cal);
                                cal = NULL; 
                                obj = NULL;
                                return DUP_VER;
                            }
                        }
                    //parse product ID
                    }else if(strcmp(token, "PRODID") == 0){

                        token = strtok(NULL, "\0");
                        
                        if(token == NULL){
                            free2DArray(infoArray);
                            free(tempInfo);
                            deleteCalendar(cal);
                            cal = NULL; 
                            obj = NULL;
                            return INV_PRODID;
                        }else{
                            if(prodFlag == 0){
                                prodFlag = 1;
                                strcpy(cal->prodID, token);
                                i++;
                                if(i >= infoArraySize){
                                    free2DArray(infoArray);
                                    free(tempInfo);
                                    deleteCalendar(cal);
                                    cal = NULL; 
                                    obj = NULL;
                                    return INV_CAL;
                                }
                            }else{
                                free2DArray(infoArray);
                                free(tempInfo);
                                deleteCalendar(cal);
                                cal = NULL; 
                                obj = NULL;
                                return DUP_PRODID;
                            }
                        }
                    }else if(strcmp(token, "BEGIN") == 0){
                        //get all information from an event
                        
                        token = strtok(NULL, "\0");

                        if(token == NULL){
                            free2DArray(infoArray);
                            free(tempInfo);
                            deleteCalendar(cal);
                            obj = NULL;
                            cal = NULL;
                            return INV_CAL;
                        }

                        if(strcmp(token, "VEVENT") == 0){
                            //parse everything inside the event until it ends
                            eventFlag = 1;
                            i++;
                            Event *event = malloc(sizeof(Event));

                            event->properties = NULL;
                            event->alarms = NULL;

                            if(event->properties == NULL){
                                event->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
                            }
                            if(event->alarms == NULL){
                                event->alarms =  initializeList(&printAlarm, &deleteAlarm, &compareAlarms);
                            }

                            strcpy(event->UID,"");


                            //set event flag back to zero after exiting loop
                            while(eventFlag == 1 && i < infoArraySize){

                                strcpy(tempInfo, infoArray[i]);
                                token = strtok(infoArray[i],":;");

                                //this is where we will handel events, properties, alarms
                                //will get next line untill closing tag has been found
                                //will not exit loop unless all event properties has been filled
                                //other wise invalid cal  at large and we hae to stop em
                                // for a dancer jackson brown

                                if(strcmp(token, "UID") == 0){

                                    if(uidFlag == 0){
                                        uidFlag = 1;

                                        token = strtok(NULL, "\0");
                                        
                                        if(token != NULL){
                                            strcpy(event->UID, token);
                                            i++;
                                            if(i >= infoArraySize){
                                                free2DArray(infoArray);
                                                free(tempInfo);
                                                deleteCalendar(cal);
                                                deleteEvent((void*)event);
                                                cal = NULL; 
                                                obj = NULL;
                                                return INV_EVENT;
                                            }
                                        }else{

                                            free2DArray(infoArray);
                                            free(tempInfo);
                                            deleteCalendar(cal);
                                            deleteEvent((void*)event);
                                            cal = NULL; 
                                            obj = NULL;
                                            return INV_EVENT;
                                        }
                                    }else{
                                        free2DArray(infoArray);
                                        free(tempInfo);
                                        deleteCalendar(cal);
                                        deleteEvent((void*)event);
                                        cal = NULL; 
                                        obj = NULL;
                                        return INV_EVENT;
                                    }
                                }else if(strcmp(token, "DTSTAMP") == 0){

                                    if(createDateFlag == 0){
                                        createDateFlag = 1;

                                        token = strtok(NULL, "\0");

                                        if(token != NULL && token[0] != 'T'){
                                            if(checkDT(token) == true){
                                                DateTime dt = parseDateTime(token);
                                                if(strcmp(dt.time, "") != 0 && strcmp(dt.date, "") != 0){
                                                    event->creationDateTime = dt;
                                                    i++;
                                                }else{
                                                    free2DArray(infoArray);
                                                    free(tempInfo);
                                                    deleteCalendar(cal);
                                                    deleteEvent((void*)event);
                                                    cal = NULL; 
                                                    obj = NULL;
                                                    return INV_DT;
                                                }
                                                if(i >= infoArraySize){
                                                    free2DArray(infoArray);
                                                    free(tempInfo);
                                                    deleteCalendar(cal);
                                                    deleteEvent((void*)event);
                                                    cal = NULL; 
                                                    obj = NULL;
                                                    return INV_EVENT;
                                                }
                                            }else{
                                                free2DArray(infoArray);
                                                free(tempInfo);
                                                deleteCalendar(cal);
                                                deleteEvent((void*)event);
                                                cal = NULL; 
                                                obj = NULL;
                                                return INV_DT;
                                            }
                                        }else{
                                            free2DArray(infoArray);
                                            free(tempInfo);
                                            deleteCalendar(cal);
                                            deleteEvent((void*)event);
                                            cal = NULL; 
                                            obj = NULL;
                                            return INV_DT;
                                        }
                                    }else{
                                        free2DArray(infoArray);
                                        free(tempInfo);
                                        deleteCalendar(cal);
                                        deleteEvent((void*)event);
                                        cal = NULL; 
                                        obj = NULL;
                                        return INV_EVENT;
                                    }
                                }else if(strcmp(token, "DTSTART") == 0){

                                    if(startDateFlag == 0){
                                        startDateFlag = 1;

                                        token = strtok(NULL, "\0");

                                        if(token != NULL && token[0] != 'T'){
                                            if(checkDT(token) == true){
                                                DateTime dt = parseDateTime(token);
                                                if(strcmp(dt.time, "") != 0 && strcmp(dt.date, "") != 0){
                                                    event->startDateTime = dt;
                                                    i++;
                                                }else{
                                                    free2DArray(infoArray);
                                                    free(tempInfo);
                                                    deleteCalendar(cal);
                                                    deleteEvent((void*)event);
                                                    cal = NULL; 
                                                    obj = NULL;
                                                    return INV_DT;
                                                }	
                                                if(i >= infoArraySize){
                                                    free2DArray(infoArray);
                                                    free(tempInfo);
                                                    deleteCalendar(cal);
                                                    deleteEvent((void*)event);
                                                    cal = NULL; 
                                                    obj = NULL;
                                                    return INV_EVENT;
                                                }
                                            }else{
                                                free2DArray(infoArray);
                                                free(tempInfo);
                                                deleteCalendar(cal);
                                                deleteEvent((void*)event);
                                                cal = NULL; 
                                                obj = NULL;
                                                return INV_DT;
                                            }
                                        }else{
                                            free2DArray(infoArray);
                                            free(tempInfo);
                                            deleteCalendar(cal);
                                            deleteEvent((void*)event);
                                            cal = NULL; 
                                            obj = NULL;
                                            return INV_DT;
                                        }
                                    }else{
                                        free2DArray(infoArray);
                                        free(tempInfo);
                                        deleteCalendar(cal);
                                        deleteEvent((void*)event);
                                        cal = NULL; 
                                        obj = NULL;
                                        return INV_EVENT;
                                    }
                                }else if(strcmp(token, "BEGIN") == 0){

                                    token = strtok(NULL, "\0");

                                    if(token == NULL){
                                        free2DArray(infoArray);
                                        free(tempInfo);
                                        deleteCalendar(cal);
                                        deleteEvent((void*)event);
                                        cal = NULL; 
                                        obj = NULL;
                                        return INV_CAL;
                                    }

                                    if(strcmp(token, "VALARM") == 0){

                                        alarmFlag = 1;
                                        i++;
                                        Alarm *alarm = malloc(sizeof(Alarm));
                                        alarm->trigger = malloc(sizeof(char)*100);

                                        alarm->properties = NULL;

                                        if(alarm->properties == NULL){
                                            alarm->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
                                        }

                                        strcpy(alarm->trigger, "");
                                        strcpy(alarm->action, "");								

                                        while(alarmFlag == 1 && i < infoArraySize){

                                            strcpy(tempInfo, infoArray[i]);
                                            token = strtok(infoArray[i],":;");

                                            if(strcmp(token,"ACTION") == 0){

                                                if(actionFlag == 0){
                                                    actionFlag = 1;
                                                    token = strtok(NULL, "\0");

                                                    if(token != NULL){
                                                        strcpy(alarm->action, token);
                                                        i++;
                                                        //no end tag
                                                        if(i >= infoArraySize){
                                                            free2DArray(infoArray);
                                                            free(tempInfo);
                                                            deleteCalendar(cal);
                                                            deleteEvent((void*)event);
                                                            deleteAlarm((void*)alarm);
                                                            cal = NULL; 
                                                            obj = NULL;
                                                            return INV_ALARM;
                                                        }
                                                    }else{
                                                        free2DArray(infoArray);
                                                        free(tempInfo);
                                                        deleteCalendar(cal);
                                                        deleteEvent((void*)event);
                                                        deleteAlarm((void*)alarm);
                                                        cal = NULL; 
                                                        obj = NULL;
                                                        return INV_ALARM;
                                                    }
                                                }else{
                                                    free2DArray(infoArray);
                                                    free(tempInfo);
                                                    deleteCalendar(cal);
                                                    deleteEvent((void*)event);
                                                    deleteAlarm((void*)alarm);
                                                    cal = NULL; 
                                                    obj = NULL;
                                                    return INV_ALARM;
                                                }
                                            }else if(strcmp(token, "TRIGGER") == 0){

                                                if(triggerFlag == 0){
                                                    triggerFlag = 1;

                                                    token = strtok(NULL, "\0");

                                                    if(token != NULL){
                                                        alarm->trigger = realloc(alarm->trigger, sizeof(char)*(strlen(token) + 100));
                                                        strcpy(alarm->trigger, token);
                                                        i++;
                                                        if(i >= infoArraySize){
                                                            free2DArray(infoArray);
                                                            free(tempInfo);
                                                            deleteCalendar(cal);
                                                            deleteEvent((void*)event);
                                                            deleteAlarm((void*)alarm);
                                                            cal = NULL; 
                                                            obj = NULL;
                                                            return INV_ALARM;
                                                        }
                                                    }else{
                                                        free2DArray(infoArray);
                                                        free(tempInfo);
                                                        deleteCalendar(cal);
                                                        deleteEvent((void*)event);
                                                        deleteAlarm((void*)alarm);
                                                        cal = NULL; 
                                                        obj = NULL;
                                                        return INV_ALARM;
                                                    }
                                                }else{
                                                    free2DArray(infoArray);
                                                    free(tempInfo);
                                                    deleteCalendar(cal);
                                                    deleteEvent((void*)event);
                                                    deleteAlarm((void*)alarm);
                                                    cal = NULL; 
                                                    obj = NULL;
                                                    return INV_ALARM;
                                                }
                                            }else if(strcmp(token, "BEGIN") == 0){
                                                //maybe properties
                                                //free shit
                                                free2DArray(infoArray);
                                                free(tempInfo);
                                                deleteCalendar(cal);
                                                deleteEvent((void*)event);
                                                deleteAlarm((void*)alarm);
                                                cal = NULL; 
                                                obj = NULL;
                                                return INV_ALARM;
                                            }else if(strcmp(token, "END") == 0){
                                                //end of an alarm
                                                token = strtok(NULL, "\0");

                                                if(token == NULL){
                                                    free2DArray(infoArray);
                                                    free(tempInfo);
                                                    deleteCalendar(cal);
                                                    deleteEvent((void*)event);
                                                    deleteAlarm((void*)alarm);
                                                    cal = NULL; 
                                                    obj = NULL;
                                                    return INV_CAL;
                                                }

                                                if(strcmp(token,"VALARM") == 0){
                                                    //valid event? add to list
                                                    if(strcmp(alarm->action, "") != 0 && strcmp(alarm->trigger, "") != 0 && alarm->properties != NULL){
                                                        insertBack(event->alarms, (void*)alarm);
                                                        alarmFlag = 0;
                                                        triggerFlag = 0;
                                                        actionFlag = 0;
                                                        i++;
                                                        if(i >= infoArraySize){
                                                            free2DArray(infoArray);
                                                            free(tempInfo);
                                                            deleteCalendar(cal);
                                                            deleteEvent((void*)event);
                                                            cal = NULL; 
                                                            obj = NULL;
                                                            return INV_EVENT;
                                                        }
                                                    }else{
                                                        free2DArray(infoArray);
                                                        free(tempInfo);
                                                        deleteCalendar(cal);
                                                        deleteEvent((void*)event);
                                                        deleteAlarm((void*)alarm);
                                                        cal = NULL; 
                                                        obj = NULL;
                                                        return INV_ALARM;
                                                    }
                                                }else{
                                                    free2DArray(infoArray);
                                                    free(tempInfo);
                                                    deleteCalendar(cal);
                                                    deleteEvent((void*)event);
                                                    deleteAlarm((void*)alarm);
                                                    cal = NULL; 
                                                    obj = NULL;
                                                    //could be a property list add
                                                    //check if it is any of the valid ones - return inv_cal
                                                    //else add prop list
                                                    return INV_ALARM;
                                                }
                                            }else{
                                                //we got a property - event
                                                Property *prop = parseProperty(tempInfo);
                                                if(prop != NULL){
                                                    insertBack(alarm->properties, (void*)prop);
                                                    i++;
                                                    if(i >= infoArraySize){
                                                        free2DArray(infoArray);
                                                        free(tempInfo);
                                                        deleteCalendar(cal);
                                                        deleteEvent((void*)event);
                                                        deleteAlarm((void*)alarm);
                                                        cal = NULL; 
                                                        obj = NULL;
                                                        return INV_ALARM;
                                                    }
                                                }else{
                                                    free2DArray(infoArray);
                                                    free(tempInfo);
                                                    deleteCalendar(cal);
                                                    deleteEvent((void*)event);
                                                    deleteAlarm((void*)alarm);
                                                    cal = NULL; 
                                                    obj = NULL;
                                                    return INV_ALARM;
                                                }
                                            }
                                            //end of loop
                                        }
                                    }else{
                                        // if it does not begin a valarm inside the event
                                        //may be a property
                                        free2DArray(infoArray);
                                        free(tempInfo);
                                        deleteCalendar(cal);
                                        deleteEvent((void*)event);
                                        cal = NULL; 
                                        obj = NULL;
                                        return INV_EVENT;
                                    }
                                }else if(strcmp(token, "END") == 0){
                                    //end of an event
                                    token = strtok(NULL, "\0");

                                    if(token == NULL){
                                        free2DArray(infoArray);
                                        free(tempInfo);
                                        deleteCalendar(cal);
                                        deleteEvent((void*)event);
                                        cal = NULL; 
                                        obj = NULL;
                                        return INV_CAL;
                                    }

                                    if(strcmp(token, "VEVENT") == 0){

                                        if(strcmp(event->UID, "") != 0 && createDateFlag == 1 && startDateFlag == 1 && event->properties != NULL && event->alarms != NULL){
                                            insertBack(cal->events, (void*)event);
                                            eventFlag = 0;
                                            startDateFlag = 0;
                                            createDateFlag = 0;
                                            uidFlag = 0;
                                            i++;
                                        }else{
                                            free2DArray(infoArray);
                                            free(tempInfo);
                                            deleteCalendar(cal);
                                            deleteEvent((void*)event);
                                            cal = NULL; 
                                            obj = NULL;
                                            return INV_EVENT;
                                        }
                                    }else{
                                        //maybe a property
                                        free2DArray(infoArray);
                                        free(tempInfo);
                                        deleteCalendar(cal);
                                        deleteEvent((void*)event);
                                        cal = NULL; 
                                        obj = NULL;
                                        return INV_EVENT;
                                    }	
                                }else{
                                    //then create event property
                                    Property *prop = parseProperty(tempInfo);
                                    if(prop != NULL){
                                        insertBack(event->properties, (void*)prop);
                                        i++;
                                        if(i >= infoArraySize){
                                            free2DArray(infoArray);
                                            free(tempInfo);
                                            deleteCalendar(cal);
                                            deleteEvent((void*)event);
                                            cal = NULL; 
                                            obj = NULL;
                                            return INV_EVENT;
                                        }
                                    }else{
                                        free2DArray(infoArray);
                                        free(tempInfo);
                                        deleteCalendar(cal);
                                        deleteEvent((void*)event);
                                        cal = NULL; 
                                        obj = NULL;
                                        return INV_EVENT;
                                    }
                                }
                                //end of loop
                            }
                        }else{
                            //if not a VEVENT
                            //might be a property for calendar
                            free2DArray(infoArray);
                            free(tempInfo);
                            deleteCalendar(cal);
                            cal = NULL; 
                            obj = NULL;
                            return INV_CAL;
                        }
                        //check ending tags after VCAL flag
                    }else if(strcmp(token, "END") == 0){

                        token = strtok(NULL, "\0");

                        if(token == NULL){
                            free2DArray(infoArray);
                            free(tempInfo);
                            deleteCalendar(cal);
                            cal = NULL; 
                            obj = NULL;
                            return INV_CAL;
                        }

                        if(strcmp(token, "VCALENDAR") == 0){

                            //end Vcalendar
                            if(calFlag == 1){
                                if(verFlag == 1 && prodFlag == 1 && strcmp(cal->prodID, "") != 0 && cal->events != NULL && cal->properties != NULL && cal->events->length > 0){
                                    calFlag = 0;
                                    verFlag = 0;
                                    prodFlag = 0;
                                    i++;
                                    *obj = cal;
                                    free2DArray(infoArray);
                                    free(tempInfo);
                                    return OK;
                                }
                            }else{
                                free2DArray(infoArray);
                                free(tempInfo);
                                deleteCalendar(cal);
                                cal = NULL; 
                                obj = NULL;
                                return INV_CAL;
                            }
                        }else{
                            //add to calendar prop list unless its one of the other ones
                            free2DArray(infoArray);
                            free(tempInfo);
                            deleteCalendar(cal);
                            cal = NULL; 
                            obj = NULL;
                            return INV_CAL;
                        }
                    }else{
                        //means there is a property
                        Property *prop = parseProperty(tempInfo);
                        if(prop != NULL){
                            insertBack(cal->properties, (void*)prop);
                            i++;
                            if(i >= infoArraySize){
                                free2DArray(infoArray);
                                free(tempInfo);
                                deleteCalendar(cal);
                                cal = NULL; 
                                obj = NULL;
                                return INV_CAL;
                            }
                        }else{
                            free2DArray(infoArray);
                            free(tempInfo);
                            deleteCalendar(cal);
                            cal = NULL; 
                            obj = NULL;
                            return INV_CAL;
                        }
                    }
                    //end of loop
                }
            }else{
                i++;
                if(i > infoArraySize){
                    free2DArray(infoArray);
                    free(tempInfo);
                    deleteCalendar(cal);
                    cal = NULL; 
                    obj = NULL;
                    return INV_CAL;
                }
            }
        }else{
            i++;
            if(i >= infoArraySize){
                free2DArray(infoArray);
                free(tempInfo);
                deleteCalendar(cal);
                cal = NULL; 
                obj = NULL;
                return INV_CAL;
            }
        }
    }

    //remember to do this
    if(calFlag == 1){
        free2DArray(infoArray);
        free(tempInfo);
        deleteCalendar(cal);
        cal = NULL; 
        obj = NULL;
        return INV_CAL;
    }
    //check to make sure all brackets have been closed

    free2DArray(infoArray);
    free(tempInfo);
    free(cal);

    return INV_CAL;
}


/** Function to delete all calendar content and free all the memory.
 *@pre Calendar object exists, is not null, and has not been freed
 *@post Calendar object had been freed
 *@return none
 *@param obj - a pointer to a Calendar struct
**/
void deleteCalendar(Calendar* obj){

    if(obj == NULL){
        return;
    }

    if(obj->events != NULL){
        freeList(obj->events);
    }

    if(obj->properties != NULL){
        freeList(obj->properties);
    }

    if(obj != NULL){
        free(obj);
    }
}


/** Function to create a string representation of a Calendar object.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a string representing the Calndar contents has been created
 *@return a string contaning a humanly readable representation of a Calendar object
 *@param obj - a pointer to a Calendar struct
**/
char* printCalendar(const Calendar* obj){

    char *string = NULL;

    if(obj == NULL){
        return NULL;
    }

    char *objEvents = toString(obj->events);
    char *objProps = toString(obj->properties);

    string = malloc(strlen(obj->prodID) + strlen(objEvents) +  strlen(objProps) + 150);

    sprintf(string, "Calendar:\nVersion: %f\nProdID: %s\nCalEvents: %s\nCalProperties: %s", obj->version, obj->prodID, objEvents, objProps);

    free(objEvents);
    free(objProps);

    return string;
}


/** Function to "convert" the ICalErrorCode into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code by indexing into 
          the descr array using rhe error code enum value as an index
 *@param err - an error code
**/
char* printError(ICalErrorCode err){

    char *error = malloc(sizeof(char)*50);

    if(err == OK){
        strcpy(error, "OK");
    }else if(err == INV_FILE){
        strcpy(error, "INV_FILE");
    }else if(err == INV_CAL){
        strcpy(error, "INV_CAL");
    }else if(err == INV_VER){
        strcpy(error, "INV_VER");
    }else if(err == DUP_VER){
        strcpy(error, "DUP_VER");
    }else if(err == INV_PRODID){
        strcpy(error, "INV_PRODID");
    }else if(err == DUP_PRODID){
        strcpy(error, "DUP_PRODID");
    }else if(err == INV_EVENT){
        strcpy(error, "INV_EVENT");
    }else if(err == INV_DT){
        strcpy(error, "INV_DT");
    }else if(err == INV_ALARM){
        strcpy(error, "INV_ALARM");
    }else if(err == WRITE_ERROR){
        strcpy(error, "WRITE_ERROR");
    }else if(err == OTHER_ERROR){
        strcpy(error, "OTHER_ERROR");
    }else{
        strcpy(error,"INVALID_ERROR_CODE");
    }

    return error;
}

//EVENTS
void deleteEvent(void* toBeDeleted){

    Event *event = (Event*)toBeDeleted;

    if(toBeDeleted == NULL){
        return;
    }

    DateTime *creation = &(event->creationDateTime);
    DateTime *start = &(event->startDateTime);

    deleteDate((void*)creation);
    deleteDate((void*)start);

    if(event->properties != NULL){
        freeList(event->properties);
    }

    if(event->alarms != NULL){
        freeList(event->alarms);
    }

    if(event != NULL){
        free(event);
    }
}

int compareEvents(const void* first, const void* second){

    Event *tempEvent1 = NULL;
    Event *tempEvent2 = NULL;

    if(first == NULL || second == NULL){
        return 1;
    }

    tempEvent1 = (Event*)first;
    tempEvent2 = (Event*)second;

    DateTime *creation1 = &(tempEvent1->creationDateTime);
    DateTime *start1 = &(tempEvent1->startDateTime);

    DateTime *creation2 = &(tempEvent2->creationDateTime);
    DateTime *start2 = &(tempEvent2->startDateTime);


    if(strcmp(tempEvent1->UID, tempEvent2->UID) == 0){
        if(strcmp(printDate((void*)creation1), printDate((void*)creation2)) == 0){
            if(strcmp(toString(tempEvent1->properties), toString(tempEvent2->properties)) == 0){
                if(strcmp(toString(tempEvent1->alarms), toString(tempEvent2->alarms))){
                    if(strcmp(printDate((void*)start1), printDate((void*)start2)) == 0){
                        return 0;
                    }
                }
            }
        }
    }
    
    return 1;
}

char* printEvent(void* toBePrinted){

    char *string = NULL;
    Event *event = NULL;

    event = (Event*)toBePrinted;

    if(toBePrinted == NULL){
        return NULL;
    }

    DateTime *creation = &(event->creationDateTime);
    DateTime *start = &(event->startDateTime);
    char *creationDate = printDate((void*)creation);
    char *startDate = printDate((void*)start);
    char *eventProps = toString(event->properties);
    char *eventAlarms = toString(event->alarms);

    string = malloc(strlen(event->UID) + strlen(creationDate) + strlen(startDate) + strlen(eventProps) + strlen(eventAlarms) + 250);

    sprintf(string, "Event:\nUID: %s\ncreationDateTime: %s\nstartDateTime: %s\nEventProperties: %s\nEventAlarms: %s",event->UID, creationDate, startDate, eventProps, eventAlarms);

    free(creationDate);
    free(startDate);
    free(eventProps);
    free(eventAlarms);

    return string;
}

//ALARMS
void deleteAlarm(void* toBeDeleted){

    Alarm *tempAlarm = NULL;

    if(toBeDeleted == NULL){
        return;
    }

    tempAlarm = (Alarm*)toBeDeleted;

    if(tempAlarm->trigger != NULL){
        free(tempAlarm->trigger);
    }

    if(tempAlarm->properties != NULL){
        freeList(tempAlarm->properties);
    }

    if(tempAlarm != NULL){
        free(tempAlarm);
    }
}
int compareAlarms(const void* first, const void* second){

    Alarm *tempAlarm1 = NULL;
    Alarm *tempAlarm2 = NULL;

    if(first == NULL || second == NULL){
        return 1;
    }

    tempAlarm1 = (Alarm*)first;
    tempAlarm2 = (Alarm*)second;

    if(strcmp(tempAlarm1->action, tempAlarm2->action) == 0){
        if(strcmp(tempAlarm1->trigger, tempAlarm2->trigger) == 0){
            if(strcmp(toString(tempAlarm1->properties), toString(tempAlarm2->properties)) == 0){
                return 0;
            }
        }
    }

    return 1;
}
char* printAlarm(void* toBePrinted){

    Alarm *tempAlarm = NULL;
    char *string = NULL;

    if(toBePrinted == NULL){
        return NULL;
    }

    tempAlarm = (Alarm*)toBePrinted;
    char *alarmProps = toString(tempAlarm->properties);

    string = malloc(strlen(tempAlarm->action) + strlen(tempAlarm->trigger) + strlen(alarmProps) + 150);
    sprintf(string, "Alarm:\nAction: %s, Trigger: %s\nAlarmProperties: %s", tempAlarm->action, tempAlarm->trigger, alarmProps);

    free(alarmProps);

    return string;
}

//PROPERTIES
void deleteProperty(void* toBeDeleted){

    Property *tempProp = NULL;

    if(toBeDeleted == NULL){
        return;
    }

    tempProp = (Property*)toBeDeleted;

    if(tempProp != NULL){
        free(tempProp);
    }
}

//done
int compareProperties(const void* first, const void* second){

    Property *tempProp1 = NULL;
    Property *tempProp2 = NULL;

    if(first == NULL || second == NULL){
        return 1;
    }

    tempProp1 = (Property*)first;
    tempProp2 = (Property*)second;

    if(strcmp(tempProp1->propName, tempProp2->propName) == 0){
        if(strcmp(tempProp1->propDescr, tempProp2->propDescr) == 0){
            return 0;
        }
    }

    return 1;
}

//done
char* printProperty(void* toBePrinted){

    Property *tempProp = NULL;
    char *string = NULL;

    if(toBePrinted == NULL){
        return NULL;
    }

    tempProp = (Property*)toBePrinted;

    string = malloc((strlen(tempProp->propName) + strlen(tempProp->propDescr) + 150));

    sprintf(string, "Name: %s, Description: %s", tempProp->propName, tempProp->propDescr);

    return string;
}

//DATES
void deleteDate(void* toBeDeleted){

    DateTime *tempDate = NULL;

    if(toBeDeleted == NULL){
        return;
    }

    tempDate = (DateTime*)toBeDeleted;
    strcpy(tempDate->date, "\0");
    strcpy(tempDate->time, "\0");
    tempDate->UTC = false;
}

int compareDates(const void* first, const void* second){

    DateTime *tempFirst = NULL;
    DateTime *tempSecond = NULL;

    if(first == NULL || second == NULL){
        return 1;
    }

    tempFirst = (DateTime*)first;
    tempSecond = (DateTime*)second;

    if(strcmp(tempFirst->date, tempSecond->date) == 0){
        if(strcmp(tempFirst->time, tempSecond->time) == 0){
            if(tempFirst->UTC == tempSecond->UTC){
                return 0;
            }
        }
    }

    return 1;
}
    
//done
char* printDate(void* toBePrinted){

    DateTime *tempDate = NULL;
    char *string = NULL;

    if(toBePrinted == NULL){
        return NULL;
    }

    tempDate = (DateTime*)toBePrinted;

    string = malloc(strlen(tempDate->date) + strlen(tempDate->time) + 150);

    if(tempDate->UTC == true){
        sprintf(string, "Date: %s, Time: %s, UTC: true", tempDate->date, tempDate->time);
    }else{
        sprintf(string, "Date: %s, Time: %s, UTC: false", tempDate->date, tempDate->time);
    }

    return string;
}

ICalErrorCode writeCalendar(char* fileName, const Calendar* obj){

    //obj is not null
    if(obj == NULL){
        return INV_CAL;
    }

    //filename not null
    if(fileName == NULL){
        return INV_FILE;
    }

    //check for valid extensions
    if(strcmp(fileName, "") != 0) {
        if(strcmp(strrchr(fileName, '.'), ".ics") != 0){
            return INV_FILE;
        }
    }else {
        return INV_FILE;
    }

    //create and open for writing
    FILE *fp;
    fp = fopen(fileName, "w");

    if(fp == NULL){
        return WRITE_ERROR;
    }

    char *calEvents = eventListToString(obj->events);
    char *calProps = propListToString(obj->properties);

    //write calendar to file
    fprintf(fp,"BEGIN:VCALENDAR\r\nVERSION:%0.0f\r\nPRODID:%s\r\n%s%sEND:VCALENDAR\r\n", obj->version, obj->prodID, calEvents, calProps);

    fclose(fp);
    free(calEvents);
    free(calProps);

    return OK;
}


ICalErrorCode validateCalendar(const Calendar* obj){ 

    if(obj == NULL){
        return INV_CAL;
    }

    ICalErrorCode code = OK;

    bool validEventList = false;
    bool validPropList = false;

    if(strcmp(obj->prodID, "") != 0 && strlen(obj->prodID) <= 999){
        if(obj->events != NULL && obj->properties != NULL && getLength(obj->events) >= 1){

            ListIterator eventIterator = createIterator(obj->events);
            ListIterator propIterator = createIterator(obj->properties);
            void* elem;
            int count = 0;
            int length = getLength(obj->properties);

            if(count == length){
                validPropList = true;
            }

            //validate the property list
            while((elem = nextElement(&propIterator)) != NULL){
                Property *prop = (Property*)elem;
                if(validateProperty(prop) == 0){
                    if(validateCalendarProperty(prop, obj->properties) == 0){
                        count++;
                        if(count == length){
                            validPropList = true;
                        }
                    }
                }
            }

            if(validPropList == false){
                return INV_CAL;
            }

            //reset
            count = 0;
            length = getLength(obj->events);

            //validate the event list
            while((elem = nextElement(&eventIterator)) != NULL){
                Event *event = (Event*)elem;
                code = validateEvent(event);

                if(code != OK){
                    return code;
                }else{
                    count++;
                    if(count == length){
                        validEventList = true;
                    }
                }
            }

            //check that they have successful lists
            if(validEventList == true && validPropList == true){
                return OK;
            }
        }
    }

    return INV_CAL;
}

char* dtToJSON(DateTime prop){

    char *string = NULL;

    string = malloc(strlen(prop.date) + strlen(prop.time) + 150);

    if(prop.UTC == true){
        sprintf(string, "{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":true}", prop.date, prop.time);
    }else{
        sprintf(string, "{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":false}", prop.date, prop.time);
    }

    return string;
}

char* eventToJSON(const Event* event){

    //vars
    char *string = malloc(sizeof(char)*100);

    if(event == NULL){
        strcpy(string,"{}");
        return string;
    }

    char *startDate = dtToJSON(event->startDateTime);
    int numProps = 3 + getLength(event->properties);
    int numAlarms = getLength(event->alarms);

    //search list for summary //might leak memory
    char* description = findPropertyDescription(event->properties, "SUMMARY");
    char *location = findPropertyDescription(event->properties, "LOCATION");
    char *organizer = findPropertyDescription(event->properties, "ORGANIZER");

    //allocate memory for a newly allocated string
    string = realloc(string, strlen(startDate) + strlen(description) + strlen(location) + strlen(organizer) + 100);

    sprintf(string,"{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\",\"location\":\"%s\",\"organizer\":\"%s\"}", startDate, numProps, numAlarms, description, location, organizer);

    free(startDate);

    return string;
}


char* eventListToJSON(const List* eventList){

    char *string = malloc(sizeof(char)*100);

    //failed input
    if(eventList == NULL){
        strcpy(string,"[]");
        return string;
    }

    //counts the number of events
    //counts the number of events
    Node *cur = eventList->head;
    int countEvents = 0;
    while(cur != NULL){	
        cur = cur->next;
        countEvents++;
    }

    //vars
    char **json = malloc((sizeof(char*)*countEvents) + 10);
    strcpy(string,"");
    int i = 0;
    Node *temp = eventList->head;

    //fill 2d array with event json strings
    while(temp != NULL){

        Event *event = (Event*)temp->data;
        char *data = eventToJSON(event);
        json[i] = malloc(strlen(data) + 10);
        strcpy(json[i], data);
        free(data);
        temp = temp->next;
        i++;
    }

    strcat(string, "[");

    for(int j = 0; j < i; j++){
        if(j + 1 == i){
            string = realloc(string, strlen(string) + strlen(json[j]) + 100);
            strcat(string, json[j]);
        }else{
            string = realloc(string, strlen(string) + strlen(json[j]) + 100);
            strcat(string, json[j]);
            strcat(string, ",");
        }
    }

    //free temp2d array
    for(int j = 0; j < i; j++){
        free(json[j]);
    }
    free(json);
    strcat(string, "]");

    return string;
}


char* calendarToJSON(const Calendar* cal){

    char *string = malloc(sizeof(char)*20);

    if(cal == NULL){
        strcpy(string, "{}");
        return string;
    }

    string = realloc(string, strlen(cal->prodID) + 100);

    int numProps = 2 + getLength(cal->properties);
    int numEvents = getLength(cal->events);

    sprintf(string, "{\"version\":%0.0f,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}", cal->version, cal->prodID, numProps, numEvents);

    return string;
}

//Assignment 3 *************************************************************************************************************

//server side
char* passCalJSONtoServer(char *fileName){
    Calendar *cal = NULL;
    char *JSON = NULL;
    ICalErrorCode code;

    code = createCalendar(fileName, &cal);
    if(code != OK){return "Invalid";}

    code = validateCalendar(cal);
    if(code != OK){return "Invalid";}

    JSON = calendarToJSON(cal);

    deleteCalendar(cal);

    return JSON;
}

char *passEventListToServer(char *fileName){
    Calendar *cal = NULL;
    char *JSON = NULL;
    ICalErrorCode code;

    code = createCalendar(fileName, &cal);
    if(code != OK){return "Invalid";}

    code = validateCalendar(cal);
    if(code != OK){return "Invalid";}

    JSON = eventListToJSON(cal->events);

    deleteCalendar(cal);

    return JSON;
}

char *passAlarmListToServer(char *fileName, int index){

    Calendar *cal = NULL;
    char *JSON = NULL;
    ICalErrorCode code;
    int toFind = index - 1;
    int i = 0;

    code = createCalendar(fileName, &cal);
    if(code != OK){return "Invalid";}

    code = validateCalendar(cal);
    if(code != OK){return "Invalid";}

    if(index > getLength(cal->events)){
        return "Invalid Input";
    }
    if(toFind < 0){
        return "Invalid Input";
    }	

    Node *temp = cal->events->head;
    Event *event = NULL;

    while(temp != NULL){
        if(toFind == i){
            event = (Event*)temp->data;
        }
        temp =  temp->next;
        i++;
    }

    JSON = alarmListToJSON(event->alarms);

    deleteCalendar(cal);

    return JSON;
}

char *passPropertyListToServer(char *fileName, int index){

    Calendar *cal = NULL;
    char *JSON = NULL;
    ICalErrorCode code;
    int toFind = index - 1;
    int i = 0;

    code = createCalendar(fileName, &cal);
    if(code != OK){return "Invalid";}

    code = validateCalendar(cal);
    if(code != OK){return "Invalid";}

    if(index > getLength(cal->events)){
        return "Invalid Input";
    }
    if(toFind < 0){
        return "Invalid Input";
    }


    Node *temp = cal->events->head;
    Event *event = NULL;

    while(temp != NULL){
        if(toFind == i){
            event = (Event*)temp->data;
        }
        temp = temp->next;
        i++;
    }

    JSON = propListToJSON(event->properties);

    deleteCalendar(cal);

    return JSON;
}
    
bool createEventFromServer(char *fileName, char *UID, char *cr_date, char *cr_time, char *cr_utc, char *st_date, char *st_time, char *st_utc, char *summary){

    Calendar *cal = NULL;
    ICalErrorCode code;

    code = createCalendar(fileName, &cal);
    if(code != OK){return false;}

    code = validateCalendar(cal);
    if(code != OK){return false;}

    Event *event = malloc(sizeof(Event));

    DateTime creation;
    DateTime start;

    strcpy(creation.time,cr_time);
    strcpy(creation.date,cr_date);
    if(strcmp(cr_utc, "true") == 0){
        creation.UTC = true;
    }else{
        creation.UTC = false;
    }

    strcpy(start.time,st_time);
    strcpy(start.date,st_date);
    if(strcmp(st_utc, "true") == 0){
        start.UTC = true;
    }else{
        start.UTC = false;
    }

    event->creationDateTime = creation;
    event->startDateTime = start;
    strcpy(event->UID,UID);

    event->alarms = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);
    event->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);

    if(strcmp(summary, "") != 0){
        Property *prop = malloc(sizeof(Property) + strlen(summary) + 10);
        strcpy(prop->propName, "SUMMARY");
        strcpy(prop->propDescr, summary);
        insertBack(event->properties, (void*)prop);
    }

    addEvent(cal, event);

    code = validateCalendar(cal);
    if(code == OK){
        writeCalendar(fileName, cal);
        deleteCalendar(cal);
        return true;
    }else{
        deleteCalendar(cal);
        return false;
    }

    return false;
}

bool createCalendarFromServer(char *fileName, char *UID, char *cr_date, char *cr_time, char *cr_utc, char *st_date, char *st_time, char *st_utc, char *prod_id, char *version){

    Calendar *cal = malloc(sizeof(Calendar));
    ICalErrorCode code;

    if(checkDigits(version) == true){
        cal->version = atof(version);
    }else{
        return false;
    }

    if(strlen(prod_id) <= 1000){
        strcpy(cal->prodID, prod_id);
    }else{
        return false;
    }
    cal->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
    cal->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);

    Event *event = malloc(sizeof(Event));

    DateTime creation;
    DateTime start;

    strcpy(creation.time,cr_time);
    strcpy(creation.date,cr_date);
    if(strcmp(cr_utc, "true") == 0){
        creation.UTC = true;
    }else{
        creation.UTC = false;
    }

    strcpy(start.time,st_time);
    strcpy(start.date,st_date);
    if(strcmp(st_utc, "true") == 0){
        start.UTC = true;
    }else{
        start.UTC = false;
    }

    event->creationDateTime = creation;
    event->startDateTime = start;
    strcpy(event->UID,UID);

    event->alarms = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);
    event->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);

    addEvent(cal, event);

    code = validateCalendar(cal);
    if(code == OK){
        writeCalendar(fileName, cal);
        deleteCalendar(cal);
        return true;
    }else{
        deleteCalendar(cal);
        return false;
    }
    
    return false;
}

//JSON parsing funtions 
char *alarmToJSON(const Alarm *alarm){

    char *string = malloc(sizeof(char)*20);

    if(alarm == NULL){
        strcpy(string, "{}");
        return string;
    }

    string = realloc(string, strlen(alarm->trigger) + strlen(alarm->action) + 100);

    int numProps = 2 + getLength(alarm->properties);

    sprintf(string, "{\"action\":\"%s\",\"trigger\":\"%s\",\"numProps\":%d}", alarm->action, alarm->trigger, numProps);

    return string;
}

char* alarmListToJSON(const List *alarmList){

    char *string = malloc(sizeof(char)*100);

    //failed input
    if(alarmList == NULL){
        strcpy(string,"[]");
        return string;
    }

    //counts the number of alarms
    //counts the number of events
    Node *cur = alarmList->head;
    int countAlarms = 0;
    while(cur != NULL){	
        cur = cur->next;
        countAlarms++;
    }

    //vars
    char **json = malloc((sizeof(char*)*countAlarms) + 10);
    strcpy(string,"");
    int i = 0;
    Node *temp = alarmList->head;

    //fill 2d array with event json strings
    while(temp != NULL){

        Alarm *alarm = (Alarm*)temp->data;
        char *data = alarmToJSON(alarm);
        json[i] = malloc(strlen(data) + 10);
        strcpy(json[i], data);
        free(data);
        temp = temp->next;
        i++;
    }

    strcat(string, "[");

    for(int j = 0; j < i; j++){
        if(j + 1 == i){
            string = realloc(string, strlen(string) + strlen(json[j]) + 100);
            strcat(string, json[j]);
        }else{
            string = realloc(string, strlen(string) + strlen(json[j]) + 100);
            strcat(string, json[j]);
            strcat(string, ",");
        }
    }

    //free temp2d array
    for(int j = 0; j < i; j++){
        free(json[j]);
    }
    free(json);
    strcat(string, "]");

    return string;
}

char* propertyToJSON(const Property *prop){

    char *string = malloc(sizeof(char)*20);

    if(prop == NULL){
        strcpy(string, "{}");
        return string;
    }

    string = realloc(string, strlen(prop->propName) + strlen(prop->propDescr) + 100);

    sprintf(string, "{\"name\":\"%s\",\"descr\":\"%s\"}", prop->propName, prop->propDescr);

    return string;
}

char *propListToJSON(const List *propList){

    char *string = malloc(sizeof(char)*100);

    //failed input
    if(propList == NULL){
        strcpy(string,"[]");
        return string;
    }

    //counts the number of events
    Node *cur = propList->head;
    int countProps = 0;
    while(cur != NULL){	
        cur = cur->next;
        countProps++;
    }

    //vars
    char **json = malloc((sizeof(char*)*countProps) + 10);
    strcpy(string,"");
    int i = 0;
    Node *temp = propList->head;

    //fill 2d array with event json strings
    while(temp != NULL){

        Property *prop = (Property*)temp->data;
        char *data = propertyToJSON(prop);
        json[i] = malloc(strlen(data) + 10);
        strcpy(json[i], data);
        free(data);
        temp = temp->next;
        i++;
    }

    strcat(string, "[");

    for(int j = 0; j < i; j++){
        if(j + 1 == i){
            string = realloc(string, strlen(string) + strlen(json[j]) + 100);
            strcat(string, json[j]);
        }else{
            string = realloc(string, strlen(string) + strlen(json[j]) + 100);
            strcat(string, json[j]);
            strcat(string, ",");
        }
    }

    //free temp2d array
    for(int j = 0; j < i; j++){
        free(json[j]);
    }
    free(json);
    strcat(string, "]");

    return string;
}



//*************************************************************************************************************************


Calendar* JSONtoCalendar(const char* str){

    if(str == NULL){
        return NULL;
    }

    char *new = constToString(str);
    Calendar *cal = malloc(sizeof(Calendar));
    char *token = NULL;
    strcpy(cal->prodID, " ");

    token = strtok(new, ":");
    if(token == NULL){
        free(new);
        free(cal);
        return NULL;
    }
    token = strtok(NULL, ",");
    if(token == NULL){
        free(new);
        free(cal);
        return NULL;
    }

    cal->version = atof(token);

    token = strtok(NULL, ":");
    if(token == NULL){
        free(new);
        free(cal);
        return NULL;
    }

    token = strtok(NULL, "}");
    if(token == NULL){
        free(new);
        free(cal);
        return NULL;
    }

    char array[strlen(token)];
    int index = 0;
    for(int i = 0; i < strlen(token); i ++){
        if(i != 0 && i != strlen(token) - 1){
            array[index] = token[i];
            index++;
        }
    }
    array[index] = '\0';


    strcpy(cal->prodID, array);
    
    cal->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
    cal->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);

    free(new);

    return cal;
}


Event* JSONtoEvent(const char* str){

    if(str == NULL){
        return NULL;
    }

    char *new = constToString(str);
    char *token = NULL;
    Event *event = malloc(sizeof(Event));
    strcpy(event->UID, "");

    token = strtok(new, ":");
    if(token == NULL){
        free(event);
        free(new);
        return NULL;
    }

    token = strtok(NULL, "}");
    if(token == NULL){
        free(event);
        free(new);
        return NULL;
    }
    
    char array[strlen(token)];
    int index = 0;
    for(int i = 0; i < strlen(token); i ++){
        if(i != 0 && i != strlen(token) -1){
            array[index] = token[i];
            index++;
        }
    }
    array[index] = '\0';
    strcpy(event->UID, array);

    event->alarms = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);
    event->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);

    free(new);

    return event;
}


void addEvent(Calendar* cal, Event* toBeAdded){

    if(cal != NULL && toBeAdded != NULL){
        insertBack(cal->events, (void*)toBeAdded);
    }
}


