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
            console.log("GPX files loaded successfully");
            files = data.files;
            initializeFileLog();
            initializeGPXDropDowns();
        },
        fail: function(error) {
            console.log("Failed to load GPX files: " + error.responseText); 
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
                console.log("Successfully uploaded file");
                alert("Upload successful!");
                addNewFile(data);
            },
            error: function(error) {
                console.log("Failed to upload file: " + error.responseText);
                alert("Error: " + error.responseText);
            }
        });
    });

    $("#createForm").submit(function(e) {
        e.preventDefault();
        if (!validateCreateGPXForm()) {
            return;
        }
        let file = getCreateGPXFile();
        console.log(file);
        $.ajax({
            type: 'post',
            dataType: 'json',
            contentType: 'application/json',
            url: '/create',
            data: JSON.stringify(file),
            success: function(data) {
                console.log("Successfully created GPX file");
                alert("GPX creation successful!");
                addNewFile(data);
            },
            error: function(error) {
                console.log("Failed to create GPX file: " + error.responseText);
                alert("Error: " + error.responseText);
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
        let nameEntry = document.getElementById("addRouteNameEntry");
        let routeName = nameEntry.value;
        let waypoints = getAddRouteWaypoints();
        $.ajax({
            type: 'post',
            dataType: 'json',
            contentType: 'application/json',
            url: '/addroute',
            data: JSON.stringify({
                file: file.name,
                name: routeName,
                waypoints: waypoints
            }),
            success: function(data) {
                console.log("Successfully added route to '" + file.name + "'");
                alert("Route added successfully!");
                incrementFileLogNumRoutes(file.name);
                if (getGPXViewSelectedFile() === file.name) {
                    setGPXViewSelectedFile(file);
                }
            },
            error: function(error) {
                console.log("Failed to add route to '" + file.name + "': " + error.responseText);
                alert("Error: " + error.responseText);
            }
        });
    });

    $("#findPathForm").submit(function(e) {
        console.log("Find path requested");
        e.preventDefault();
        if (!validateFindRouteForm()) {
            $("#foundPathText").hide();
            return;
        }
        let waypoints = getFindRouteWaypoints();
        console.log(waypoints);
        $.ajax({
            type: 'get',
            dataType: 'json',
            contentType: 'application/json',
            url: '/findpaths',
            data: {
                startLat: waypoints.start.lat,
                startLon: waypoints.start.lon,
                endLat: waypoints.end.lat,
                endLon: waypoints.end.lon
            },
            success: function(data) {
                console.log("Successfully got list of found paths");
                console.log(data);
                let numPaths = 0;
                if (data.routes.length !== undefined) {
                    numPaths += data.routes.length;
                }
                if (data.tracks.length !== undefined) {
                    numPaths += data.tracks.length;
                }
                updateFoundPathText(numPaths);
                clearFindPathTable();
                if (numPaths > 0) {
                    $("#findPathView").show();
                    $("#findPathTable").show();
                }
                let routes = data.routes;
                let tracks = data.tracks;
                addComponentsToFindPathTable(routes, "Route");
                addComponentsToFindPathTable(tracks, "Track");
                window.scrollBy(0, document.body.scrollHeight);
            },
            error: function(error) {
                console.log("Failed to get list of found paths: " + error.responseText);
                alert("Error: " + error.responseText);
            }
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
            console.log("Successfully loaded GPX info for '" + file.name + "'");
            $("#noFileSelectedHeader").hide();
            $("#gpxView").show();
            clearGPXViewTable();
            let routes = data.routes;
            let tracks = data.tracks;
            addComponentsToGPXView(file, routes, "Route");
            addComponentsToGPXView(file, tracks, "Track");
        },
        fail: function(error) {
            console.log("Failed to load GPX info for '" + file.name + "': " + error.responseText);
            alert("Error: " + error.responseText);
        }
    });
}

function updateFoundPathText(numPaths) {
    let foundPathText = document.getElementById("foundPathText");
    $("#foundPathText").show();
    if (numPaths === 0) {
        $("#foundPathText").css("color", "red");
        foundPathText.innerText = "Found " + numPaths + " paths";
    }
    else {
        $("#foundPathText").css("color", "green");
        if (numPaths === 1) {
            foundPathText.innerText = "Found " + numPaths + " path";
        }
        else {
            foundPathText.innerText = "Found " + numPaths + " paths";
        }
    }
}

function getGPXViewSelectedFile() {
    let fileSelect = document.getElementById("gpxViewFileSelect");
    return fileSelect.options[fileSelect.selectedIndex].value;
}

function getCreateGPXFile() {
    let nameEntry = document.getElementById("createGpxNameEntry");
    let versionEntry = document.getElementById("createGpxVersionEntry");
    let creatorEntry = document.getElementById("createGpxCreatorEntry");
    let file = {
        name: nameEntry.value,
        version: parseFloat(versionEntry.value),
        creator: creatorEntry.value
    };
    return file;
}

function validateCreateGPXForm() {
    let valid = true;
    let nameEntry = document.getElementById("createGpxNameEntry");
    if (nameEntry.value.length <= 4 || nameEntry.value.match(".gpx$") === null) {
        $("#createGpxNameEntry").addClass("is-invalid");
        valid = false;
    }
    else {
        $("#createGpxNameEntry").removeClass("is-invalid");
    }
    let version = document.getElementById("createGpxVersionEntry");
    if (!version.value.match("^[0-9]+[.]?[0-9]*$")) {
        $("#createGpxVersionEntry").addClass("is-invalid");
        valid = false;
    }
    else {
        $("#createGpxVersionEntry").removeClass("is-invalid");
    }
    let creator = document.getElementById("createGpxCreatorEntry");
    if (creator.value.length === 0) {
        $("#createGpxCreatorEntry").addClass("is-invalid");
        valid = false;
    }
    else {
        $("#createGpxCreatorEntry").removeClass("is-invalid");
    }
    return valid;
}

function getFindRouteWaypoints() {
    let startLatInput = document.getElementById("startLatInput");
    let startLonInput = document.getElementById("startLonInput");
    let endLatInput = document.getElementById("endLatInput");
    let endLonInput = document.getElementById("endLonInput");
    let waypoints = {
        start: {
            lat: parseFloat(startLatInput.value),
            lon: parseFloat(startLonInput.value)
        },
        end: {
            lat: parseFloat(endLatInput.value),
            lon: parseFloat(endLonInput.value)
        }
    };
    return waypoints;
}

function validateFindRouteForm() {
    let valid = true;
    let startLatInput = document.getElementById("startLatInput");
    let startLonInput = document.getElementById("startLonInput");
    let endLatInput = document.getElementById("endLatInput");
    let endLonInput = document.getElementById("endLonInput");
    if (!validateWaypointInput(startLatInput)) {
        valid = false;
    }
    if (!validateWaypointInput(startLonInput)) {
        valid = false;
    }
    if (!validateWaypointInput(endLatInput)) {
        valid = false;
    }
    if (!validateWaypointInput(endLonInput)) {
        valid = false;
    }
    return valid;
}

function getAddRouteWaypoints() {
    let latitudeInputs = document.getElementsByClassName("latitudeInput");
    let longitudeInputs = document.getElementsByClassName("longitudeInput");
    let waypoints = [];
    for (let i = 0; i < latitudeInputs.length; i++) {
        let waypoint = {};
        waypoint.lat = parseFloat(latitudeInputs[i].value);
        waypoint.lon = parseFloat(longitudeInputs[i].value);
        waypoints.push(waypoint);
    }
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
        if (!validateWaypointInput(latitudeInputs[i])) {
            valid = false;
        }
        if (!validateWaypointInput(longitudeInputs[i])) {
            valid = false;
        }
    }
    return valid;
}

function validateWaypointInput(input) {
    // Use regex to validate for floating point numbers
    if (!input.value.match("^-?[0-9]+[.]?[0-9]*$")) {
        input.classList.add("is-invalid");
        return false;
    }
    else {
        input.classList.remove("is-invalid");
        return true;
    }
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
    addComponentsToTable(gpxViewTable, file, components, type, true);
}

function addComponentsToFindPathTable(components, type) {
    let findPathTable = document.getElementById("findPathTable");
    addComponentsToTable(findPathTable, null, components, type, false);
}

function addComponentsToTable(table, file, components, type, hasActions) {
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
        if (hasActions) {
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
        }
        table.appendChild(row);
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
    clearTable(gpxViewTable);
}

function clearFindPathTable() {
    let findPathTable = document.getElementById("findPathTable");
    $("#findPathView").hide();
    clearTable(findPathTable);
}

function clearTable(table) {
    while (table.children.length > 1) {
        table.removeChild(table.lastChild);
    }
}
