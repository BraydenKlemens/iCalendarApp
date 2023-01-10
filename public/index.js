// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    
    // On page-load AJAX Example
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/someendpoint',   //The server endpoint we are connecting to
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            $('#blah').html("On page load, Received string '"+JSON.stringify(data)+"' from server");
            //We write the object to the console to show that the request was successful
            console.log(data); 

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

    // Event listener form replacement example, building a Single-Page-App, no redirects if possible
    $('#someform').submit(function(e){
        $('#blah').html("Callback from the form");
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });

    //MY CODE *****************************************************************************************

    //clear status panel
    $('#clearButton').click(function(){
        $('#status').val('');
    });

    //request for File Log Panel
    getLogPanelInformation();

    function getLogPanelInformation(){
        $.ajax({
            type: 'get',           
            dataType: 'json',
            url: '/FileLogPanel',
            success: function (data) {
                let addToLog = '<tr><th>File Name<br/>(click to download)</th><th>Version</th><th>Product ID</th><th>Number of events</th><th>Number of properties</th></tr>';
                let options = '<option>none</option>';
                
                for(let index in data.arr){

                    let cal = JSON.parse(data.arr[index]);
                    let name = JSON.parse(data.names[index]);
                    let keys = Object.keys(JSON.parse(data.arr[index]));

                    if(keys[0] !== "invalid"){
                        options += '<option>' + name + '</option>';
                        addToLog += '<tr><td><a href="/uploads/' + name + '">' + name +'</a></td><td>' + cal.version + '</td><td>' + cal.prodID + '</td><td>' + cal.numEvents + '</td><td>' + cal.numProps + '</td></tr>';
                    }
                }
                
                if(data.names.length === 0){addToLog = '<tr><td>No Files</td></tr>';}
                $('#log_table').html(addToLog);
                $('#view_dropdown').html(options);
                $('#view_dropdown_event').html(options);
                $('#choosefile_dropdown').html(options);
                
            },
            error: function(error) {
                console.log(error); 
            }
        });
    }

    //upload button for file logpanel
    
    $('#uploadButton').on('change', function(get){
        let formData = new FormData();
        formData.append('uploadFile', get.target.files[0]);
        let search = {search:get.target.files[0].name};

        let fn = get.target.files[0].name;
        if(fn.split('.').pop() === 'ics'){

            $.ajax({
                //dataType json?
                url: '/updateFiles',
                data: search,
                type: 'GET',
                success: function (data) {
                    if(data.trig === 'true'){
                        let $status = $('#status');
                        let cur = $status.val();
                        $status.val(cur + 'UPLOAD ERROR:' + data.file +' already exists in /uploads\n');
                    }else{
                         $.ajax({
                            url: '/upload',
                            data: formData,
                            type: 'POST',
                            contentType: false,
                            processData: false,
                            success: function (data) {
                                getLogPanelInformation();
                            },
                            error: function(error) {
                                let $status = $('#status');
                                let cur = $status.val();
                                $status.val(cur + error + '\n');
                            }   
                        });
                    }
                },
                error: function(error) {
                     console.log(error);
                }   
            });
        }else{
            let $status = $('#status');
            let cur = $status.val();
            $status.val(cur + 'UPLOAD ERROR:' + fn +' is not a valid calendar file\n');
        }
    });

    
    //request information for calendar viewer panel
    $('#view_dropdown').on('change', function(){

        let file = {name:$('#view_dropdown').val()};

         $.ajax({
            type: 'get',           
            dataType: 'json',
            data: file,
            url: '/CalendarViewPanel',
            success: function (data){
                let add = '<tr><th>Event</th><th>Start Date</th><th>Start Time</th><th>Summary</th><th>Props</th><th>Alarms</th></tr>';
                let time = '';
                let date = '';
                let eventNum = 1;

                if(data.events !== undefined){

                    for(let i in data.events){
                        let tempdate = data.events[i].startDT.date;
                        let temptime = data.events[i].startDT.time;
                        
                        //parse into new string
                        date = tempdate.substr(0, 4) + '/' + tempdate.substr(4,2) + '/' + tempdate.substr(6,2);
                        
                        if(data.events[i].startDT.isUTC === true){
                            time = temptime.substr(0,2) + ':' + temptime.substr(2,2) + ':' + temptime.substr(4,2) + '(UTC)';
                        }else{
                            time = temptime.substr(0,2) + ':' + temptime.substr(2,2) + ':' + temptime.substr(4,2);
                        }

                        add += '<tr><td>' + eventNum + '</td><td>' + date + '</td><td>' + time + '</td><td>' + data.events[i].summary + '</td><td>' + data.events[i].numProps + '</td><td>' + data.events[i].numAlarms + '</td></tr>'; 
                        eventNum++;
                    }
                }

                $('#view_table').html(add);
            },
            error: function(error) {
                console.log(error); 
            }
        });
    });

    
    $('#show_alarm_btn').click(function(){

        let object = {name:$('#view_dropdown').val(), num:$('#showalarm_entry').val()};
        $('#showalarm_entry').val('');

         $.ajax({
            type: 'get',           
            dataType: 'json',
            data: object,
            url: '/showAlarms',
            success: function (data){
                let $status = $('#status');
                if(data.alarms === 'Invalid File'){
                    let cur = $status.val();
                    $status.val(cur + 'SHOW ALARMS ERROR:' + data.file + ' is an invalid file\n');
                }else if(data.alarms === 'Invalid Input'){
                    let cur = $status.val();
                    $status.val(cur + 'SHOW ALARMS ERROR: Event No. ' + data.num + ' does not exist\n');
                }else if(data.alarms === '[]'){
                    let cur = $status.val();
                    $status.val(cur + 'SHOW ALARMS ERROR: No alarms exist in ' + data.file + ' Event No. ' + data.num + '\n');
                }else{
                    let string = '';
                    let $status = $('#status');
                    let cur = $status.val();
                    let index = 1;
                    for(let i in data.alarms){
                        string += 'Alarm ' + index + ': Action:' + data.alarms[i].action + ', Trigger:' + data.alarms[i].trigger + ', numProps:' + data.alarms[i].numProps + '\n';
                        index++;
                    }
                    $status.val(cur + string);
                }
            },
            error: function(error) {
                console.log(error); 
            }
        });
    });


    $('#extract_entry_btn').click(function(){

        let object = {name:$('#view_dropdown').val(), num:$('#extract_entry').val()};
        $('#extract_entry').val('');

         $.ajax({
            type: 'get',           
            dataType: 'json',
            data: object,
            url: '/showProps',
            success: function (data){
                let $status = $('#status');
                if(data.props === 'Invalid File'){
                    let cur = $status.val();
                    $status.val(cur + 'EXTRACT PROPS ERROR:' + data.file + ' is an invalid file\n');
                }else if(data.props === 'Invalid Input'){
                    let cur = $status.val();
                    $status.val(cur + 'EXTRACT PROPS ERROR: Event No. ' + data.num + ' does not exist\n');
                }else if(data.props === '[]'){
                    let cur = $status.val();
                    $status.val(cur + 'EXTRACT PROPS ERROR: No optional properties exist in ' + data.file + ' Event No. ' + data.num + '\n');
                }else{
                    let string = '';
                    let $status = $('#status');
                    let cur = $status.val();
                    let index = 1;
                    for(let i in data.props){
                        string += 'Property ' + index + ': Name:' + data.props[i].name + ', Description:' + data.props[i].descr + '\n';
                        index++;
                    }
                    $status.val(cur + string);
                }
            },
            error: function(error) {
                console.log(error); 
            }
        });
    });


    //add an event for the user
    $('#add_event').click(function(){

        let object = {
            cr_date:$('#cr_date').val(),
            cr_time:$('#cr_time').val(),
            cr_utc:$('#cr_utc').val(),
            st_date:$('#st_date').val(),
            st_time:$('#st_time').val(),
            st_utc:$('#st_utc').val(),
            uid:$('#UID').val(),
            summary:$('#Summary').val(),
            file:$('#view_dropdown_event').val()
        };

        $('#cr_date').val('');
        $('#cr_time').val('');
        $('#cr_utc').val('');
        $('#st_date').val('');
        $('#st_time').val('');
        $('#st_utc').val('');
        $('#UID').val('');
        $('#Summary').val('');
        

        $.ajax({
            type: 'get',           
            dataType: 'json',
            data: object,
            url: '/createEvent',
            success: function (data){
                if(data.valid === 'true'){
                    getLogPanelInformation();
                    $('#view_dropdown').val('none');
                }else{
                    let $status = $('#status');
                    let cur = $status.val();
                    $status.val(cur + 'CREATE EVENT ERROR: failed to add an event to ' + object.file + '\n');
                }
            },
            error: function(error) {
                console.log(error); 
            }
        });
    });

    $('#create_cal').click(function(){

        let object = {
            cr_date:$('#cr_date2').val(),
            cr_time:$('#cr_time2').val(),
            cr_utc:$('#cr_utc2').val(),
            st_date:$('#st_date2').val(),
            st_time:$('#st_time2').val(),
            st_utc:$('#st_utc2').val(),
            uid:$('#UID2').val(),
            prod_id:$('#prod_id').val(),
            version:$('#version').val(),
            file:$('#file_name').val()
        };

        $('#cr_date2').val('');
        $('#cr_time2').val('');
        $('#cr_utc2').val('');
        $('#st_date2').val('');
        $('#st_time2').val('');
        $('#st_utc2').val('');
        $('#UID2').val('');
        $('#file_name').val('');
        $('#version').val('');
        $('#prod_id').val('');

        $.ajax({
            type: 'get',           
            dataType: 'json',
            data: object,
            url: '/createCalendar',
            success: function (data){
                if(data.valid === 'true'){
                    getLogPanelInformation();
                    let $status = $('#status');
                    let cur = $status.val();
                    $status.val(cur + 'SUCCESS: created ' + object.file + '\n');
                }else{
                    let $status = $('#status');
                    let cur = $status.val();
                    $status.val(cur + 'CREATE CALENDAR ERROR: failed to create ' + object.file + '\n');
                }
            },
            error: function(error) {
                console.log(error); 
            }
        });
    });

    //new code goes here

    $('#selectfile_div').hide();
    $('#query_div1').hide();
    $('#query_div2').hide();
    let choice = '';

    //login details and set up tables
    $('#db_login').click(function(){

        let object = {
            user_name:$('#user_name').val(),
            password:$('#password').val(),
            db_name:$('#db_name').val()
        };

        $('#user_name').val('');
        $('#password').val('');
        $('#db_name').val('');

        $.ajax({
            type: 'get',           
            dataType: 'json',
            data: object,
            url: '/createDataBase',
            success: function (data){
                let $login = $('#login_state');
                let $status = $('#status');
                if(data.state === 'fail'){
                    $login.html('Login State: login failed, try again');
                }else{
                    $login.html('Login State: authentication successful');
                    $('#login_div').hide();
                    $('#query_div1').show();
                    $('#query_div2').show();
                }
            },
            error: function(error) {
                console.log(error); 
            }
        });    
    });


    //store all files - get a list of the files int the drop down
    $('#store_files').click(function(){

        let fileArray = [];

        let ddl = document.getElementById('view_dropdown');
        for (let i = 0; i < ddl.options.length; i++) {
            if(ddl.options[i].value !== 'none'){
                fileArray.push(ddl.options[i].value);
            }
        }

        let object = {
            files: fileArray
        };

        $.ajax({
            type: 'get',           
            dataType: 'json',
            data: object,
            url: '/fillDataBase',
            success: function (data){
                let $status = $('#status');
                if(data.state === 'success'){
                    let cur = $status.val();
                    $status.val(cur + "SUCCESS: stored all files\n");
                }else{
                    let cur = $status.val();
                    $status.val(cur + "ERROR: failed to store all files.\n");
                }
            },
            error: function(error) {
                console.log(error); 
            }
        });    
    });


    $('#clear_data').click(function(){

        $.ajax({
            type: 'get',           
            dataType: 'json',
            url: '/clearDataBase',
            success: function (data){
                let $status = $('#status');
                if(data.state === 'success'){
                    let cur = $status.val();
                    $status.val(cur + "SUCCESS: cleared data base.\n");
                }else{
                    let cur = $status.val();
                    $status.val(cur + "ERROR: failed to clear data base.\n");
                }
            },
            error: function(error) {
                console.log(error); 
            }
        });    
    });

    $('#view_db').click(function(){

        $.ajax({
            type: 'get',           
            dataType: 'json',
            url: '/displayDataBase',
            success: function (data){
                if(data.counts !== 'fail'){
                    let db_status = 'Database has ' + data.counts.file + ' files, ' + data.counts.event + ' events, and ' + data.counts.alarm + ' alarms.\n';
                    let $status = $('#status');
                    let cur = $status.val();
                    $status.val(cur + db_status);
                }
            },
            error: function(error) {
                console.log(error); 
            }
        });    
    });


    $('#query_dropdown').on('change', function(){
        choice = $('#query_dropdown').val();

        if(choice === 'Display events from a file'){
            $('#selectfile_div').show();
        }

        if(choice === 'Display all events sorted by start date'){
            $('#selectfile_div').hide();
        }

        if(choice === 'Display alarms from a file'){
            $('#selectfile_div').show();
        }

        if(choice === 'Delete a calendar file'){
            $('#selectfile_div').show();
        }

        if(choice === 'none'){
            $('#selectfile_div').hide();
        }

        if(choice === 'Delete upcoming event from file'){
            $('#selectfile_div').show();
        }
    });

    $('#execute').click(function(){

        if(choice  === 'Display all events sorted by start date'){
            $.ajax({
                type: 'get',           
                dataType: 'json',
                url: '/displayAllEvents',
                success: function (data){
                    let events = JSON.parse(data.events);
                    let add = 'DISPLAY ALL EVENTS:\n';
                    let $status = $('#status'); 
                    let num = 1;
                    for(let i in events){
                        add += "Event #" + num + ": start_time: " + events[i].start_time + ", summary: " + events[i].summary + "\n";
                        num++;
                    }
                    let cur = $status.val();
                    $status.val(cur + add);
                },
                error: function(error) {
                    console.log(error); 
                }
            });
        }

        if(choice  === 'Display events from a file'){

            let object = {
                file:$('#choosefile_dropdown').val()
            };

            if($('#choosefile_dropdown').val() !== 'none'){
                $.ajax({
                    type: 'get',           
                    dataType: 'json',
                    data: object,
                    url: '/displayEventsFile',
                    success: function (data){
                        if(data.events !== 'fail'){
                            let events = JSON.parse(data.events);
                            let add = 'DISPLAY EVENTS IN :' + object.file + '\n';
                            let $status = $('#status'); 
                            let num = 1;
                            for(let i in events){
                                add += "Event #" + num + ": start_time: " + events[i].start_time + ", summary: " + events[i].summary + "\n";
                                num++;
                            }
                            let cur = $status.val();
                            $status.val(cur + add);
                        }else{
                            let $status = $('#status'); 
                            let cur = $status.val();
                            $status.val(cur + "FAIL:" + object.file +" doesnt exist in data base or file has no events\n");
                        }
                    },
                    error: function(error) {
                        console.log(error); 
                    }
                });
            }else{
                let $status = $('#status'); 
                let cur = $status.val();
                $status.val(cur + 'ERROR: invalid file choice for query execution\n');
            }
        }

        if(choice  === 'Display alarms from a file'){

            let object = {
                file:$('#choosefile_dropdown').val()
            };

            if($('#choosefile_dropdown').val() !== 'none'){
                $.ajax({
                    type: 'get',           
                    dataType: 'json',
                    data: object,
                    url: '/displayAlarmsFile',
                    success: function (data){
                        if(data.alarms === 'fail'){
                            let $status = $('#status'); 
                            let cur = $status.val();
                            $status.val(cur + "FAIL:" + object.file +" doesnt exist in data base or file has no events\n");
                        }

                        if(data.alarms !== 'empty'){
                            let alarms = JSON.parse(data.alarms);
                            let add = 'DISPLAY ALARMS IN :' + object.file + '\n';
                            let $status = $('#status'); 
                            let num = 1;
                            for(let i in alarms){
                                add += "Alarm #" + num + ": action: " + alarms[i].action + ", trigger: " + alarms[i].trigger + "\n";
                                num++;
                            }
                            let cur = $status.val();
                            $status.val(cur + add);
                        }else{
                            let $status = $('#status'); 
                            let cur = $status.val();
                            $status.val(cur + 'No alarms in calendar file ' + object.file + "\n");
                        }
                    },
                    error: function(error) {
                        console.log(error); 
                    }
                });
            }else{
                let $status = $('#status'); 
                let cur = $status.val();
                $status.val(cur + 'ERROR: invalid file choice for query execution\n');
            }
        }

        if(choice === 'Delete a calendar file'){

            let object = {
                file:$('#choosefile_dropdown').val()
            };

            if($('#choosefile_dropdown').val() !== 'none'){
                $.ajax({
                    type: 'get',           
                    dataType: 'json',
                    data: object,
                    url: '/deleteFile',
                    success: function (data){
                        let $status = $('#status'); 
                        let cur = $status.val();
                        $status.val(cur + 'SUCCESS: deleted ' + object.file + "from the data base\n");
                    },
                    error: function(error) {
                        console.log(error); 
                    }
                });
            }else{
                let $status = $('#status'); 
                let cur = $status.val();
                $status.val(cur + 'ERROR: invalid file choice for query execution\n');
            }
        }

        if(choice === 'Delete upcoming event from file'){

            let object = {
                file:$('#choosefile_dropdown').val()
            };

            $.ajax({
                type: 'get',           
                dataType: 'json',
                data: object,
                url: '/deleteUpcomingEvent',
                success: function (data){
                    if(data.state === 'success'){
                        let $status = $('#status'); 
                        let cur = $status.val();
                        $status.val(cur + 'SUCCESS: deleted upcoming event from ' + object.file + "\n");
                    }else{
                        let $status = $('#status'); 
                        let cur = $status.val();
                        $status.val(cur + 'FAIL: no events to delete in ' + object.file + "\n");
                    }
                },
                error: function(error) {
                    console.log(error); 
                }
            });
        }

    });

    //end of main
});






