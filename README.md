# iCalendarApp
Brayden Klemens - 2019

## About
The purpose of the web application allow users to upload iCalendar files (.ics) for viewing and manipulation.

## Use Cases
- Login to the web application
- Users can switch between any calendar file they have uploaded for viewing
- Users are able to view all of the events, alarms and other properties associated with the calendar through a convienient UI
- Users can add/create new events for specific calendars
- Users can create entire calendars
- Users can use Database functionality (Save all files, Clear all data, Display DB Status)
- Users can use the UI to make DB queries such as "Delete upcoming event from file".

## Tech Stack & Functionality
- Self built C library for calendar parsing, validation and modification (iCalendarApp/Parser/)
- NodeJS for accessing api endpoints (App.js)
- Javascript/HTML/CSS for the UI (iCalendarApp/public)
- MySql for database queries

## Install & Run

Clone the repo & Install: ```NodeJs``` ```npm``` ```gcc```

```
cd iCalendarApp
npm install
cd parser
make
cd ..
```
Run the app on a port number
```
node app.js 1337
```
On a browser navigate to: ```localhost:1337```
