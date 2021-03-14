var files = [];

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
        console.log("Create GPX requested");
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
                for (let i = 0; i < files.length; i++) {
                    if (files[i].name == data.name) {
                        files[i] = data;
                        return;
                    }
                }
                files.push(data);
                addFileToFileLog(data);
                addOptionToDropDowns(file.name);
            },
            error: function(error) {
                alert("Error: " + error.responseText);
            }
        });
    });

    $("#createForm").submit(function(e) {
        console.log("Create GPX requested");
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            //Create an object for connecting to another waypoint
        });
    });

    $("#addRouteForm").submit(function(e) {
        console.log("Add route requested");
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            //Create an object for connecting to another waypoint
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
        let selected = fileSelect.options[fileSelect.selectedIndex].value;
        $.ajax({
            type: 'get',
            dataType: 'json',
            data: {"file": selected},
            url: '/gpxinfo',
            success: function (data) {
                $("#noFileSelectedHeader").hide();
                $("#gpxView").show();
                clearGPXViewTable();
                console.log(data);
                let routes = data.routes;
                let tracks = data.tracks;
                let file = files[fileSelect.selectedIndex];
                addComponentsToGPXView(file, routes, "Route");
                addComponentsToGPXView(file, tracks, "Track");
            },
            fail: function(error) {
                console.log(error); 
            }
        });
    };
});

function addWaypointElement() {
    let grid = document.getElementById("addRouteWaypointGrid");
    let row = document.createElement("div");
    row.setAttribute("class", "row");
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
    input.setAttribute("class", "form-control gridInput");
    input.setAttribute("placeholder", type ? "longitude" : "latitude");
    return input;
}

function createRemoveWaypointButton() {
    let input = document.createElement("input");
    input.setAttribute("type", "button");
    input.setAttribute("class", "btn btn-secondary gridInput");
    input.setAttribute("value", "Remove waypoint");
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
        td.innerHTML = components[i]["len"];
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
