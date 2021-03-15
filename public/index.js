var files = [];
var waypoint = [];

// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    initializeAddRouteWaypoints();
    // let file = {"name": "test.gpx", "version": 1.1, "creator": "Ethan Rowan", "numWaypoints": 3, "numRoutes": 1, "numTracks": 1};
    // // setGPXViewFile(file);
    // addFileToFileLog(file);
    // let routes = [{"name": "testroute", "numPoints": 5, "len": 123, "loop": true},{"name": "testroute2", "numPoints": 7, "len": 456, "loop": false}];
    // let tracks = [{"name": "testtrack", "numPoints": 5, "len": 123, "loop": true},{"name": "testtrack2", "numPoints": 7, "len": 456, "loop": false}];
    // addComponentsToGPXView(file, routes, "Route");
    // addComponentsToGPXView(file, tracks, "Track");

    // // On page-load AJAX Example
    // $.ajax({
    //     type: 'get',            //Request type
    //     dataType: 'json',       //Data type - we will use JSON for almost everything 
    //     url: '/endpoint1',   //The server endpoint we are connecting to
    //     data: {
    //         stuff: "Value 1",
    //         junk: "Value 2"
    //     },
    //     success: function (data) {
    //         /*  Do something with returned object
    //             Note that what we get is an object, not a string, 
    //             so we do not need to parse it on the server.
    //             JavaScript really does handle JSONs seamlessly
    //         */
    //         $('#blah').html("On page load, received string '"+data.stuff+"' from server");
    //         //We write the object to the console to show that the request was successful
    //         console.log(data); 

    //     },
    //     fail: function(error) {
    //         // Non-200 return, do something with error
    //         $('#blah').html("On page load, received error from server");
    //         console.log(error); 
    //     }
    // });

    /* Request all GPX files, then initialize file log 
    and drop down menus */
    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/uploads',
        success: function (data) {
            files = data.files;
            initializeFileLog();
            initializeGPXDropDowns();
        },
        fail: function(error) {
            console.log(error); 
        }
    });

    /* Override default form upload setup so callbacks can be created */
    $("#uploadForm").submit(function(e) {
        e.preventDefault();
        let uploadFiles = document.getElementById("uploadFile").files;
        let file = uploadFiles[0];
        let formdata = new FormData();
        formdata.append("uploadFile", file);
        $.ajax({
            type: 'post',
            dataType: 'json',
            contentType: false,
            processData: false,
            data: formdata,
            url: '/upload',
            success: function (data) {
                alert("Upload successful!");
                addNewFile(data);
            },
            error: function(error) {
                alert("Error: " + error.responseText);
            }
        });
    });

    $("#createForm").submit(function(e) {
        e.preventDefault();
        let nameEntry = document.getElementById("createGpxNameEntry");
        let filename = nameEntry.value;
        if (filename.length <= 4 || filename.match(".gpx$") === null) {
            $("#createGpxNameEntry").addClass("is-invalid");
            return;
        }
        else {
            $("#createGpxNameEntry").removeClass("is-invalid");
        }
        $.ajax({
            type: 'post',
            dataType: 'json',
            contentType: 'application/json',
            url: '/create',
            data: JSON.stringify({
                name: filename
            }),
            success: function(data) {
                alert("GPX creation successful!");
                addNewFile(data);
            },
            error: function(error) {
                alert("Error: " + error);
            }
        });
    });

    $("#addRouteForm").submit(function(e) {
        console.log("Add route requested");
        e.preventDefault();
        let fileSelect = document.getElementById("addRouteFileSelect");
        let file = files[fileSelect.selectedIndex-1];
        if (!validateAddRouteForm()) {
            return;
        }
        let waypoints = getAddRouteWaypoints();
        $.ajax({
            type: 'post',
            dataType: 'json',
            contentType: 'application/json',
            url: '/addroute',
            data: JSON.stringify({
                file: file.name,
                waypoints: waypoints
            }),
            success: function(data) {
                alert("Route added successfully!");
                incrementFileLogNumRoutes(file.name);
                if (getGPXViewSelectedFile() === file.name) {
                    setGPXViewSelectedFile(file);
                }
            },
            error: function(error) {
                alert("Error: " + error);
            }
        });
    });

    $("#findPathForm").submit(function(e) {
        console.log("Find path requested");
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            //Create an object for connecting to another waypoint
        });
    });

    document.getElementById("btnAddWpt").onclick = function() {
        addWaypointElement();
    };

    document.getElementById("gpxViewFileSelect").onchange = function(e) {
        let fileSelect = document.getElementById("gpxViewFileSelect");
        let file = files[fileSelect.selectedIndex-1];
        setGPXViewSelectedFile(file);
    };
});

// Increments the number of routes counter in the file log for a specific file
function incrementFileLogNumRoutes(filename) {
    let fileLogTable = document.getElementById("fileLogTable");
    let logEntries = fileLogTable.getElementsByTagName("tr");
    for (let i = 0; i < logEntries.length; i++) {
        if (logEntries[i].getAttribute("name") === filename) {
            let entryData = logEntries[i].getElementsByTagName("td");
            entryData[4].textContent++;
            break;
        }
    }
}

function setGPXViewSelectedFile(file) {
    $.ajax({
        type: 'get',
        dataType: 'json',
        data: {"file": file.name},
        url: '/gpxinfo',
        success: function (data) {
            $("#noFileSelectedHeader").hide();
            $("#gpxView").show();
            clearGPXViewTable();
            console.log(data);
            let routes = data.routes;
            let tracks = data.tracks;
            addComponentsToGPXView(file, routes, "Route");
            addComponentsToGPXView(file, tracks, "Track");
        },
        fail: function(error) {
            console.log(error); 
        }
    });
}

function getGPXViewSelectedFile() {
    let fileSelect = document.getElementById("gpxViewFileSelect");
    return fileSelect.options[fileSelect.selectedIndex].value;
}

function getAddRouteWaypoints() {
    let latitudeInputs = document.getElementsByClassName("latitudeInput");
    let longitudeInputs = document.getElementsByClassName("longitudeInput");
    let waypoints = [];
    for (let i = 0; i < latitudeInputs.length; i++) {
        let waypoint = {};
        waypoint.lat = parseFloat(latitudeInputs[i].value);
        waypoint.lon = parseFloat(longitudeInputs[i].value);
        console.log(waypoint);
        waypoints.push(waypoint);
    }
    console.log(waypoints);
    return waypoints;
}

function validateAddRouteForm() {
    let valid = true;
    let fileSelect = document.getElementById("addRouteFileSelect");
    if (fileSelect.selectedIndex === 0) {
        $("#addRouteFileSelect").addClass("is-invalid");
        valid = false;
    }
    else {
        $("#addRouteFileSelect").removeClass("is-invalid");
    }
    let latitudeInputs = document.getElementsByClassName("latitudeInput");
    let longitudeInputs = document.getElementsByClassName("longitudeInput");
    for (let i = 0; i < latitudeInputs.length; i++) {
        // Use regex to validate for floating point numbers
        if (!latitudeInputs[i].value.match("^-?[0-9]+[.]?[0-9]*$")) {
            latitudeInputs[i].classList.add("is-invalid");
            valid = false;
        }
        else {
            latitudeInputs[i].classList.remove("is-invalid");
        }
        // Use regex to validate for floating point numbers
        if (!longitudeInputs[i].value.match("^-?[0-9]+[.]?[0-9]*$")) {
            longitudeInputs[i].classList.add("is-invalid");
            valid = false;
        }
        else {
            longitudeInputs[i].classList.remove("is-invalid");
        }
    }
    return valid;
}

function addNewFile(data) {
    for (let i = 0; i < files.length; i++) {
        if (files[i].name === data.name) {
            files[i] = data;
            return;
        }
    }
    files.push(data);
    addFileToFileLog(data);
    addOptionToDropDowns(data.name);
}

function addWaypointElement() {
    let grid = document.getElementById("addRouteWaypointGrid");
    let row = document.createElement("div");
    row.setAttribute("class", "row waypoint");
    for (let i = 0; i < 2; i++) {
        let col = document.createElement("div");
        col.setAttribute("class", "col");
        col.appendChild(createLatLonInput(i));
        row.appendChild(col);
    }
    row.appendChild(createRemoveWaypointButton());
    grid.appendChild(row);
    window.scrollBy(0, 50);
}

function createLatLonInput(type) {
    let input = document.createElement("input");
    input.setAttribute("type", "text");
    let typeStr = type ? "longitude" : "latitude";
    input.setAttribute("class", "form-control gridInput " + typeStr + "Input");
    input.setAttribute("placeholder", typeStr);
    return input;
}

function createRemoveWaypointButton() {
    let input = document.createElement("input");
    input.setAttribute("type", "button");
    input.setAttribute("class", "btn btn-secondary gridInput");
    input.setAttribute("value", "Remove waypoint");
    input.setAttribute("tabindex", -1);
    input.onclick = function() {
        input.parentElement.remove();
    }
    return input;
}

function createBasicButton(name) {
    let input = document.createElement("input");
    input.setAttribute("type", "button");
    input.setAttribute("class", "btn btn-secondary gridInput");
    input.setAttribute("value", name);
    return input;
}

function initializeAddRouteWaypoints() {
    for (let i = 0; i < 2; i++) {
        addWaypointElement();
    }
}

function initializeGPXDropDowns() {
    let dropdowns = document.getElementsByClassName("gpxSelect");
    for (let i = 0; i < dropdowns.length; i++) {
        for (let j = 0; j < files.length; j++) {
            let option = document.createElement("option");
            option.innerHTML = files[j].name;
            dropdowns[i].appendChild(option);
        }
    }
}

function initializeFileLog() {
    if (files.length > 0) {
        $("#noFileHeader").hide();
        $("#fileLogTable").show();
    }
    for (let i = 0; i < files.length; i++) {
        addFileToFileLog(files[i]);
    }
}

function showAction(id) {
    $(".actionDiv").hide();
    $(id).show();
    window.scrollBy(0, document.body.scrollHeight);
}

function showUploadGPXAction() {
    showAction("#uploadGpx");
}

function showCreateGPXAction() {
    showAction("#createGpx");
}

function showAddRouteAction() {
    showAction("#addRoute");
}

function showFindPathAction() {
    showAction("#findPath");
}

function renameComponent(gpxFileName, componentName, type) {
    console.log("Rename requested route/track '" + componentName + "' of '" + gpxFileName + "'");
}

function showOtherData(gpxFileName) {
    console.log("Showing other data for '" + gpxFileName + "'");
}

function addFileToFileLog(file) {
    let fileLogTable = document.getElementById("fileLogTable");
    let row = document.createElement("tr");
    row.setAttribute("name", file.name);
    let td = document.createElement("td");
    let link = document.createElement("a");
    let linkText = document.createTextNode(file["name"]);
    link.setAttribute("href", file["name"]);
    link.appendChild(linkText);
    td.appendChild(link);
    row.appendChild(td);
    td = document.createElement("td");
    td.innerHTML = file["version"];
    row.appendChild(td);
    td = document.createElement("td");
    td.innerHTML = file["creator"];
    row.appendChild(td);
    td = document.createElement("td");
    td.innerHTML = file["numWaypoints"];
    row.appendChild(td);
    td = document.createElement("td");
    td.innerHTML = file["numRoutes"];
    row.appendChild(td);
    td = document.createElement("td");
    td.innerHTML = file["numTracks"];
    row.appendChild(td);
    fileLogTable.appendChild(row);
}

function addComponentsToGPXView(file, components, type) {
    let gpxViewTable = document.getElementById("gpxViewTable");
    for (let i = 0; i < components.length; i++) {
        let row = document.createElement("tr");
        let td = document.createElement("td");
        td.innerHTML = type + " " + (i+1);
        row.appendChild(td);
        td = document.createElement("td");
        td.innerHTML = components[i]["name"];
        row.appendChild(td);
        td = document.createElement("td");
        td.innerHTML = components[i]["numPoints"];
        row.appendChild(td);
        td = document.createElement("td");
        td.innerHTML = components[i]["len"] + "m";
        row.appendChild(td);
        td = document.createElement("td");
        td.innerHTML = components[i]["loop"];
        row.appendChild(td);
        td = document.createElement("td");
        let btnRename = createBasicButton("Rename");
        td.appendChild(btnRename);
        btnRename.onclick = function(e) {
            renameComponent(file["name"], components[i]["name"], type);
        }
        let btnViewOtherData = createBasicButton("View other data");
        td.appendChild(btnViewOtherData);
        btnViewOtherData.onclick = function() {
            showOtherData(file["name"]);
        };
        row.appendChild(td);
        gpxViewTable.appendChild(row);
    }
}

function addOptionToDropDowns(filename) {
    let dropdowns = document.getElementsByClassName("gpxSelect");
    for (let i = 0; i < dropdowns.length; i++) {
        let option = document.createElement("option");
        option.innerHTML = filename;
        dropdowns[i].appendChild(option);
    }
}

function clearGPXViewTable() {
    let gpxViewTable = document.getElementById("gpxViewTable");
    while (gpxViewTable.children.length > 1) {
        gpxViewTable.removeChild(gpxViewTable.lastChild);
    }
}
