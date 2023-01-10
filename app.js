'use strict'

//define data base
const mysql = require('mysql');
let connection = mysql.createConnection({});

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});


//******************** Your code goes here ******************** 

//Sample endpoint
app.get('/someendpoint', function(req , res){
  res.send({
    foo: "bar"
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);

//time to do some dancing ****************************************
//define c functions
let sharedLib = ffi.Library('./sharedLib', {
  'passCalJSONtoServer': [ 'string', ['string'] ],
  'passEventListToServer': [ 'string', ['string'] ],
  'passAlarmListToServer': ['string', ['string', 'int'] ],
  'passPropertyListToServer': ['string', ['string', 'int'] ],
  'createEventFromServer': ['bool', ['string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string'] ],
  'createCalendarFromServer': ['bool', ['string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string'] ],
});

//return info for a file log panel
app.get('/FileLogPanel', function(req, res){
  let path = './uploads';
  let cals = [];
  let names = [];
  let object = {invalid: "invalid file"};
  
  fs.readdir(path, function(error, fileNames) {
    for(let index in fileNames) {

      let fileName = fileNames[index];
      let filepath = 'uploads/' + fileName;
      let calJSON = sharedLib.passCalJSONtoServer(filepath);

      if(fileName.split('.').pop() !== 'ics'){
        continue;
      }

      if(calJSON === 'Invalid'){
        names.push(JSON.stringify(fileName));
        cals.push(JSON.stringify(object));
        continue;
      }

      names.push(JSON.stringify(fileName));
      cals.push(calJSON);
    }

    res.send({"arr":cals, "names":names});

  });
});

//check if files are able to be uploaded or if they exist
app.get('/updateFiles', function(req, res){
  let path = './uploads';
  let names = [];
  let search = req.query.search;
  let trig = false;
  let filePath = 'uploads/' + search;

  fs.readdir(path, function(error, fileNames) {
    for(let i in fileNames) {
      if(fileNames[i] === search){
        trig = true;
      }
    }
    if(trig === true){
      res.send({"trig":"true","file":search});
    }else{
      res.send({"trig":"false"});
    }
  });
});

//get information for calendar viewer
app.get('/CalendarViewPanel', function(req, res){
  let file = req.query.name;
  let filepath = 'uploads/' + file;
  let events = sharedLib.passEventListToServer(filepath);

  if(events === 'Invalid'){
    res.send({"invalid":"Invalid"});
  }else{
    let object = JSON.parse(events);
    res.send({"events":object});
  }
});


app.get('/showAlarms', function(req, res){

  let file = req.query.name;
  let filepath = 'uploads/' + file;
  let num = req.query.num;

  let alarms = sharedLib.passAlarmListToServer(filepath, num);

  if(alarms === 'Invalid'){
    res.send({"alarms":"Invalid File","file":filepath});
  }else if(alarms === 'Invalid Input'){
    res.send({"alarms":"Invalid Input", "num":num});
  }else if(alarms === '[]'){
    res.send({"alarms":"[]","num":num,"file":filepath});
  }else{
    let object = JSON.parse(alarms);
    res.send({"alarms":object});
  }

});

app.get('/showProps', function(req, res){

  let file = req.query.name;
  let filepath = 'uploads/' + file;
  let num = req.query.num;

  let props = sharedLib.passPropertyListToServer(filepath, num);

  if(props === 'Invalid'){
    res.send({"props":"Invalid File","file":filepath});
  }else if(props === 'Invalid Input'){
    res.send({"props":"Invalid Input", "num":num});
  }else if(props === '[]'){
    res.send({"props":"[]","num":num,"file":filepath});
  }else{
    let object = JSON.parse(props);
    res.send({"props":object});
  }

});

app.get('/createEvent', function(req, res){

  let file = req.query.file;
  let filepath = 'uploads/' + file;
  let cr_date = req.query.cr_date;
  let cr_time = req.query.cr_time;
  let cr_utc = req.query.cr_utc;

  let st_date = req.query.st_date;
  let st_time = req.query.st_time;
  let st_utc = req.query.st_utc;

  let uid = req.query.uid;
  let summary = req.query.summary;

  
  if(cr_date.length === 8 && st_date.length === 8){
    if(cr_time.length === 6 && st_time.length === 6){
      if((cr_utc === 'true' || cr_utc === 'false') && (st_utc === 'true' || st_utc === 'false')){
        let valid = sharedLib.createEventFromServer(filepath, uid, cr_date, cr_time, cr_utc, st_date, st_time, st_utc, summary);
        if(valid == true){
          res.send({"valid":"true"});
        }else{
          res.send({"valid":"false"});
        }
      }else{
        res.send({"valid":"false"});
      }
    }else{
      res.send({"valid":"false"});
    }
  }else{
    res.send({"valid":"false"});
  }

});

app.get('/createCalendar', function(req, res){

  let file = req.query.file;
  let filepath = 'uploads/' + file;
  let cr_date = req.query.cr_date;
  let cr_time = req.query.cr_time;
  let cr_utc = req.query.cr_utc;

  let st_date = req.query.st_date;
  let st_time = req.query.st_time;
  let st_utc = req.query.st_utc;
  let uid = req.query.uid;

  let prod_id = req.query.prod_id;
  let version = req.query.version;

  let dup = false;
  let path = './uploads';
  let ext = false;
  fs.readdir(path, function(error, fileNames) {
    for(let i in fileNames) {
      if(fileNames[i] === file){
        dup = true;
      }
    }
  });

  if(file.split('.').pop() !== 'ics'){
    ext = true;
  }

  if(ext === false && dup === false){
    if(cr_date.length === 8 && st_date.length === 8){
      if(cr_time.length === 6 && st_time.length === 6){
        if((cr_utc === 'true' || cr_utc === 'false') && (st_utc === 'true' || st_utc === 'false')){
          let valid = sharedLib.createCalendarFromServer(filepath, uid, cr_date, cr_time, cr_utc, st_date, st_time, st_utc, prod_id, version);
          if(valid == true){
            res.send({"valid":"true"});
          }else{
            res.send({"valid":"false"});
          }
        }else{
          res.send({"valid":"false"});
        }
      }else{
        res.send({"valid":"false"});
      }
    }else{
      res.send({"valid":"false"});
    }
  }else{
    res.send({"valid":"false"});
  }

});

app.get('/createDataBase', function(req, res){

  let user_name = req.query.user_name;
  let password = req.query.password;
  let db_name = req.query.db_name;

  connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : user_name,
    password : password,
    database : db_name
  });

  connection.connect(function(err){
    if(err){
      res.send({'state':'fail'});
    }else{
      connection.query("SELECT * FROM FILE", function(err, rows, fields){
        if(err){
          connection.query("CREATE TABLE FILE (cal_id INT AUTO_INCREMENT PRIMARY KEY, file_Name VARCHAR(60) NOT NULL, version INT NOT NULL, prod_id VARCHAR(256) NOT NULL)", function(err, rows, fields){
            if(err){console.log(err);} 
          });
        }else{
          console.log('table FILE already exists'); 
        }
      }); 

      connection.query("SELECT * FROM EVENT", function(err, rows, fields){
        if(err){
          connection.query("CREATE TABLE EVENT (event_id INT AUTO_INCREMENT PRIMARY KEY, summary VARCHAR(1024), start_time DATETIME NOT NULL, location VARCHAR(60), organizer VARCHAR(256), cal_file INT NOT NULL, FOREIGN KEY(cal_file) REFERENCES FILE(cal_id) ON DELETE CASCADE)", function(err, rows, fields){
            if(err){console.log(err);} 
          });
        }else{
          console.log('table EVENT already exists');
        }
      });
  
      connection.query("SELECT * FROM ALARM", function(err, rows, fields){
        if(err){
          connection.query("CREATE TABLE ALARM (alarm_id INT AUTO_INCREMENT PRIMARY KEY, action VARCHAR(256) NOT NULL, `trigger` VARCHAR(256) NOT NULL, event INT NOT NULL, FOREIGN KEY(event) REFERENCES EVENT(event_id) ON DELETE CASCADE)", function(err, rows, fields){
            if(err){console.log(err);} 
          });
        }else{
          console.log('table ALARM already exists');
        }
      });
      res.send({'state':'success'}); 
    }
  });
}); 

app.get('/fillDataBase', function(req, res){

  let event_id_ref = 0;

  //clear data base before refill

  let files = req.query.files;

  if(connection.state === 'authenticated'){

    connection.query("DELETE FROM FILE", function(err, rows, fields){
      if(err){
        console.log(err);
      }
    }); 

      //set auto increments to 1 before starting
    connection.query("ALTER TABLE FILE AUTO_INCREMENT = 1", function(err, rows, fields){
      if(err){console.log(err);} 
    });

    connection.query("ALTER TABLE EVENT AUTO_INCREMENT = 1", function(err, rows, fields){
      if(err){console.log(err);} 
    });

    connection.query("ALTER TABLE ALARM AUTO_INCREMENT = 1", function(err, rows, fields){
      if(err){console.log(err);} 
    });

    //fill data base now
    for(let i in files){
      let cal = sharedLib.passCalJSONtoServer('uploads/' + files[i]);
      let dbFILE = JSON.parse(cal);
      connection.query("SELECT FILE.file_Name FROM FILE WHERE FILE.file_Name ='" + files[i] + "'", function(err, rows, fields){
        if(err){
          console.log(err);
        }else{
          if(rows.length === 0){
             connection.query("INSERT INTO FILE (file_Name, version, prod_id) VALUES ('" + files[i] + "'," + dbFILE.version + ",'" + dbFILE.prodID + "')", function(err, rows, fields){
              if(err){
                console.log(err);
              }else{
                let event = sharedLib.passEventListToServer('uploads/' + files[i]);
                let dbEVENT = JSON.parse(event);

                //loop through all events
                for(let j in dbEVENT){
                  let summary = dbEVENT[j].summary;
                  if(summary === ''){summary = null;}
                  let location = dbEVENT[j].location;
                  if(location === ''){location = null;}
                  let organizer = dbEVENT[j].organizer;
                  if(organizer === ''){organizer = null;}

                  let tempdate = dbEVENT[j].startDT.date;
                  let temptime = dbEVENT[j].startDT.time;
                  let date = tempdate.substr(0, 4) + '/' + tempdate.substr(4,2) + '/' + tempdate.substr(6,2);
                  let time = temptime.substr(0,2) + ':' + temptime.substr(2,2) + ':' + temptime.substr(4,2);

                  connection.query("SELECT FILE.cal_id FROM FILE WHERE FILE.file_name ='" + files[i] + "'", function(err, rows, fields){
                    if(err){
                      console.log(err);
                    }else{
                      let cal_ref = rows[0].cal_id;
                      connection.query("INSERT INTO EVENT (summary, start_time, location, organizer, cal_file) VALUES ('" + summary + "','" + date + " " + time + "','" + location + "','" + organizer + "','" + cal_ref + "')", function(err, rows, fields){
                        if(err){
                          console.log(err);
                        }else{
                          event_id_ref++;
                          let num = j;
                          num++;
                          let alarm = sharedLib.passAlarmListToServer('uploads/' + files[i], num);
                          let dbALARM = JSON.parse(alarm);
                          for(let k in dbALARM){
                            connection.query("INSERT INTO ALARM (action, `trigger`, event) VALUES ('" + dbALARM[k].action + "','" + dbALARM[k].trigger + "'," + event_id_ref + ")", function(err, rows, fields){
                              if(err){
                                console.log('alarm fail');
                              }
                            });
                          }
                        }
                      });
                    }
                  });
                }
              }
            });
          }
        }
      });
    }
    res.send({'state':'success'});  
  }
});


app.get('/clearDataBase', function(req, res){

  //clear the data base
  if(connection.state === 'authenticated'){
    connection.query("DELETE FROM FILE", function(err, rows, fields){
      if(err){
        console.log(err);
      }else{
        res.send({"state":"success"});
      }
    }); 
  }else{
    res.send({"state":"fail"});
  }    
});

app.get('/displayDataBase', function(req, res){

  let fileCount = 0;
  let eventCount = 0;
  let alarmCount = 0;

  //clear the data base
  if(connection.state === 'authenticated'){
    connection.query("SELECT * FROM FILE", function(err, rows, fields){
      if(err){
        console.log(err);
      }else{
        fileCount = rows.length;
        connection.query("SELECT * FROM EVENT", function(err, rows, fields){
          if(err){
            console.log(err);
          }else{
            eventCount = rows.length;
            connection.query("SELECT * FROM ALARM", function(err, rows, fields){
              if(err){
                console.log(err);
              }else{
                alarmCount = rows.length;
                let object = {
                  file:fileCount,
                  event:eventCount,
                  alarm:alarmCount
                };
                res.send({"counts":object});
              }
            });
          }
        });
      }
    }); 
  }else{
    res.send({"counts":"fail"});
  }    
});

app.get('/displayAllEvents', function(req, res){

  if(connection.state === 'authenticated'){

    connection.query("SELECT * FROM EVENT ORDER BY start_time", function(err, rows, fields){
      if(err){
        console.log(err);
      }else{
       let events = JSON.stringify(rows);
       res.send({"events":events});
      }
    });
  }else{ 
    res.send({"state":"fail"});
  }
});

app.get('/displayEventsFile', function(req, res){

  let filename = req.query.file;

  if(connection.state === 'authenticated'){

    connection.query("SELECT FILE.cal_id FROM FILE WHERE FILE.file_Name ='" + filename + "'", function(err, rows, fields){
      if(err){
        console.log(err);
      }else{
        if(rows.length !== 0){
          let cal_id = rows[0].cal_id;
          
          connection.query("SELECT * FROM EVENT WHERE EVENT.cal_file = " + cal_id, function(err, rows, fields){
            if(err){
              console.log(err);
            }else{
              let events = JSON.stringify(rows);
              res.send({"events":events});
            }
          }); 
        }else{
          res.send({"events":"fail"});
        }
      }
    });
  }else{
    res.send({"events":"fail"});
  }
});

app.get('/displayAlarmsFile', function(req, res){

  let filename = req.query.file;

  if(connection.state === 'authenticated'){

    connection.query("SELECT FILE.cal_id FROM FILE WHERE FILE.file_Name ='" + filename + "'", function(err, rows, fields){
      if(err){
        console.log(err);
      }else{
        if(rows.length !== 0){
          let cal_id = rows[0].cal_id;
          
          connection.query("SELECT EVENT.event_id FROM EVENT WHERE EVENT.cal_file = " + cal_id, function(err, rows, fields){
            if(err){
              console.log(err);
            }else{
              let find = '';
              for(let i = 0; i < rows.length; i++){
                if((i + 1) === rows.length){
                  find += rows[i].event_id;
                }else{
                  find += rows[i].event_id + ' OR ALARM.event =';
                }
              }
              connection.query("SELECT * FROM ALARM WHERE ALARM.event = " + find, function(err, rows, fields){
                if(err){
                  console.log(err);
                }else{
                  if(rows.length !== 0){
                    let alarmObject = JSON.stringify(rows);
                    res.send({"alarms":alarmObject});
                  }else{
                    res.send({"alarms":"empty"});
                  }
                }
              });
            }
          }); 
        }else{
          res.send({"alarms":"fail"});
        }
      }
    });
  }else{
    res.send({"alarms":"fail"});
  }
});

app.get('/deleteFile', function(req, res){

  let filename = req.query.file;

  if(connection.state === 'authenticated'){
    connection.query("DELETE FROM FILE WHERE FILE.file_Name = '" + filename + "'", function(err, rows, fields){
      if(err){
        console.log(err);
      }else{
        res.send({"state":"success"});
      }
    });
  }else{
    res.send({"state":"fail"});
  }
});

app.get('/deleteUpcomingEvent', function(req, res){

  let filename = req.query.file;

  if(connection.state === 'authenticated'){

    connection.query("SELECT FILE.cal_id FROM FILE WHERE FILE.file_Name ='" + filename + "'", function(err, rows, fields){
      if(err){
        console.log(err);
      }else{
        if(rows.length !== 0){
          let cal_id = rows[0].cal_id;
          connection.query("SELECT * FROM EVENT WHERE EVENT.cal_file = " + cal_id + " ORDER BY start_time", function(err, rows, fields){
            if(err){
              console.log(err);
            }else{
              if(rows.length !== 0){
                let event_id = rows[0].event_id;
                connection.query("DELETE FROM EVENT WHERE EVENT.event_id = " + event_id, function(err, rows, fields){
                  if(err){
                    console.log(err);
                  }else{
                    res.send({"state":"success"});
                  }
                })
              }else{
                res.send({"state":"fail"});
              }
            }
          });
        }else{
          res.send({"state":"fail"});
        }
      }
    });
  }else{ 
    res.send({"state":"fail"});
  }
});




