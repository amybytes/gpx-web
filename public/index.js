var files = [];
var waypoint = [];
var renameFileName = null;
var renameIndex = -1;
var renameType = null;

// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    initializeAddRouteWaypoints();
    
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

    $("#renameForm").submit(function(e) {
        e.preventDefault();
        if (!validateRenameForm()) {
            return;
        }
        let newnameEntry = document.getElementById("renameNewNameEntry");
        let newname = newnameEntry.value;
        $.ajax({
            type: 'post',
            dataType: 'json',
            contentType: 'application/json',
            url: '/rename',
            data: JSON.stringify({
                name: renameFileName,
                index: renameIndex,
                type: renameType,
                newname: newname
            }),
            success: function (data) {
                console.log("Component in '" + renameFileName + "' renamed successfully");
                alert(renameType + " renamed successfully!");
                let gpxViewTable = document.getElementById("gpxViewTable");
                let rows = gpxViewTable.getElementsByTagName("tr");
                for (let i = 1; i < rows.length; i++) {
                    let entries = rows[i].getElementsByTagName("td");
                    console.log(entries[0].textContent, renameType + " " + (renameIndex+1));
                    if (entries[0].textContent === (renameType + " " + (renameIndex+1))) {
                        entries[1].textContent = newname;
                    }
                }
            },
            fail: function(error) {
                console.log("Failed to rename component in GPX file '" + renameFileName + "'");
                alert("Error: " + error.responseText); 
            }
        });
    });

    $("#numComponentsForm").submit(function(e) {
        e.preventDefault();
        if (!validateNumComponentsForm()) {
            $("#getLengthText").hide();
            return;
        }
        let lengthEntry = document.getElementById("numComponentsLengthEntry");
        let len = lengthEntry.value;
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/gpxinfo/length',
            data: {
                len: len
            },
            success: function (data) {
                console.log("Successfully got number of components with length " + len + ".");
                updateLengthText(len, data.numRoutes, data.numTracks);
                window.scrollBy(0, document.body.scrollHeight);
            },
            fail: function(error) {
                console.log("Failed to get the number of components with length " + len + ".");
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
            if (routes.length === 0 && tracks.length === 0) {
                $("#noComponentsHeader").show();
                $("#gpxView").hide();
            }
            else {
                $("#noComponentsHeader").hide();
                $("#gpxView").show();
            }
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

function validateNumComponentsForm() {
    let lengthEntry = document.getElementById("numComponentsLengthEntry");
    if (!lengthEntry.value.match("^[0-9]+[.]?[0-9]*$")) {
        $("#numComponentsLengthEntry").addClass("is-invalid");
        return false;
    }
    else {
        $("#numComponentsLengthEntry").removeClass("is-invalid");
        return true;
    }
}

function updateLengthText(len, numRoutes, numTracks) {
    let lengthText = document.getElementById("getLengthText");
    $("#getLengthText").show();
    let routesColor = "green";
    let tracksColor = "green";
    if (numRoutes === 0) {
        routesColor = "red";
    }
    if (numTracks === 0) {
        tracksColor = "red";
    }
    lengthText.innerHTML = "";
    if (numRoutes === 1) {
        lengthText.innerHTML += "There is <span style='color:" + routesColor + "'>" + numRoutes + "</span> route and <span style='color:" + tracksColor + "'>" + numTracks;
    }
    else {
        lengthText.innerHTML += "There are <span style='color:" + routesColor + "'>" + numRoutes + "</span> routes and <span style='color:" + tracksColor + "'>" + numTracks;
    }
    if (numTracks === 1) {
        lengthText.innerHTML += "</span> track with length " + len + "m.";
    }
    else {
        lengthText.innerHTML += "</span> tracks with length " + len + "m.";
    }
}

function getGPXViewSelectedFile() {
    let fileSelect = document.getElementById("gpxViewFileSelect");
    return fileSelect.options[fileSelect.selectedIndex].value;
}

function validateRenameForm() {
    let newnameEntry = document.getElementById("renameNewNameEntry");
    if (newnameEntry.value.length === 0) {
        $("#renameNewNameEntry").addClass("is-invalid");
        return false;
    }
    else {
        $("#renameNewNameEntry").removeClass("is-invalid");
        return true;
    }
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
        $("#fileLog").show();
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

function showRenameAction() {
    showAction("#rename");
}

function showNumComponentsAction() {
    showAction("#numComponents");
}

function renameComponent(gpxFileName, index, type) {
    console.log("Rename requested route/track '" + index + "' of '" + gpxFileName + "'");
    showRenameAction();
    window.scrollBy(0, document.body.scrollHeight);
    renameFileName = gpxFileName;
    renameIndex = index;
    renameType = type;
}

function showOtherData(gpxFileName, index, type) {
    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/otherdata',
        data: {
            name: gpxFileName,
            index: index,
            type: type
        },
        success: function (data) {
            console.log("Other data for GPX file '" + gpxFileName + "' loaded successfully");
            let readableData = ">>OTHER DATA<<\n";
            let keys = Object.keys(data);
            if (keys.length > 0) {
                for (let i = 0; i < keys.length; i++) {
                    readableData += keys[i] + ": " + data[keys[i]] + "\n";
                }
            }
            else {
                readableData += "No other data.";
            }
            alert(readableData);
        },
        fail: function(error) {
            console.log("Failed to load other data for GPX file '" + gpxFileName + "'");
            alert("Error: " + error.responseText); 
        }
    });
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
        if (components[i]["name"] === "None") {
            td.innerHTML = "";
        }
        else {
            td.innerHTML = components[i]["name"];
        }
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
            btnRename.onclick = function() {
                renameComponent(file["name"], i, type);
            }
            let btnViewOtherData = createBasicButton("View other data");
            td.appendChild(btnViewOtherData);
            btnViewOtherData.onclick = function() {
                showOtherData(file["name"], i, type);
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
