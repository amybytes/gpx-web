'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

const xsdFile = "gpx.xsd";

var libgpxparser = ffi.Library(__dirname + "/parser/bin/libgpxparser", {
  "validateGPXFile": ["int", ["string", "string"]],
  "getValidGPXFileAsJSON": ["string", ["string", "string", "string"]],
  "getGPXFileAsJSON": ["string", ["string", "string"]],
  "getGPXRoutesAsJSON": ["string", ["string"]],
  "getGPXTracksAsJSON": ["string", ["string"]],
  "createGPXFileFromJSON": ["int", ["string", "string", "string"]],
  "addRouteToGPXFile": ["int", ["string", "string", "string", "string"]],
  "getRouteAsJSON": ["string", ["string", "string"]],
  "getRoutesBetweenAsJSON": ["string", ["string", "float", "float", "float", "float", "float"]],
  "getTracksBetweenAsJSON": ["string", ["string", "float", "float", "float", "float", "float"]],
  "getOtherDataAsJSON": ["string", ["string", "int", "string"]],
  "renameRoute": ["int", ["string", "int", "string", "string"]],
  "renameTrack": ["int", ["string", "int", "string", "string"]],
  "getRouteWaypointsAsJSON": ["string", ["string", "int"]]
});

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

  if (uploadFile.mimetype != "application/gpx+xml") {
    return res.status(422).send("The file is not a GPX file.");
  }

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    if (!libgpxparser.validateGPXFile("uploads/" + uploadFile.name, xsdFile)) {
      fs.unlink("uploads/" + uploadFile.name, function(err) {}); // Delete the temporary file
      return res.status(422).send("The GPX file is not valid.");
    }

    let fileJson = libgpxparser.getGPXFileAsJSON("uploads/" + uploadFile.name, uploadFile.name);

    res.status(200).send(fileJson); // Upload successful!
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

app.get('/uploads', function(req, res) {
  let gpxArr = [];
  let files = fs.readdirSync("uploads")
  for (let i = 0; i < files.length; i++) {
    let gpxJson = JSON.parse(libgpxparser.getValidGPXFileAsJSON("uploads/" + files[i], files[i], xsdFile));
    if (Object.keys(gpxJson).length > 0) {
      gpxArr.push(gpxJson);
    }
  }
  res.send({
    files: gpxArr
  });
});

app.get('/gpxinfo', function(req, res) {
  let filename = req.query.file;
  let jsonRoutes = JSON.parse(libgpxparser.getGPXRoutesAsJSON("uploads/" + filename));
  let jsonTracks = JSON.parse(libgpxparser.getGPXTracksAsJSON("uploads/" + filename));
  res.send({
    routes: jsonRoutes,
    tracks: jsonTracks
  });
});

const bodyParser = require('body-parser');
app.use(bodyParser.json());

function isGPXNameUnique(name) {
  let files = fs.readdirSync("uploads");
  return files.indexOf(name) === -1;
}

app.post('/create', function(req, res) {
  let filename = req.body.name;
  if (filename === null) {
    return res.status(400).send("Bad request");
  }
  if (!isGPXNameUnique(filename)) {
    return res.status(422).send("A GPX file with that name already exists.");
  }
  let status = libgpxparser.createGPXFileFromJSON(JSON.stringify(req.body), "uploads/" + filename, xsdFile);
  if (status === 0) {
    return res.status(422).send("GPX creation failed."); // Unprocessable entity
  }
  let fileJson = JSON.parse(libgpxparser.getGPXFileAsJSON("uploads/" + filename, filename));
  res.status(200).send(fileJson); // GPX creation successful!
});

app.post("/addroute", async function(req, res) {
  let file = req.body.file;
  let name = req.body.name;
  let waypoints = req.body.waypoints;
  let auth = req.body.auth;
  let status = libgpxparser.addRouteToGPXFile("uploads/" + file, name, JSON.stringify(waypoints), xsdFile);
  if (status == 0) {
    return res.status(422).send("Route could not be added.");
  }
  if (auth !== undefined) {
    await addRouteToDBFile(file, auth);
  }
  res.status(204).send(); // Success; no content
});

app.get("/findpaths", function(req, res) {
  let waypoints = req.query;
  if (waypoints == null) {
    return res.status(400).send("Bad request");
  }
  else if (waypoints.startLat == null || waypoints.startLon == null ||
      waypoints.endLat == null || waypoints.endLon == null) {
    return res.status(400).send("Bad request");
  }
  let startLat = parseFloat(waypoints.startLat);
  let startLon = parseFloat(waypoints.startLon);
  let endLat = parseFloat(waypoints.endLat);
  let endLon = parseFloat(waypoints.endLon);
  let delta = req.query.delta;
  let routes = [];
  let tracks = [];
  let files = fs.readdirSync("uploads");
  for (let i = 0; i < files.length; i++) {
    if (libgpxparser.validateGPXFile("uploads/" + files[i], xsdFile)) {
      let paths = JSON.parse(libgpxparser.getRoutesBetweenAsJSON("uploads/" + files[i], startLat, startLon, endLat, endLon, delta));
      if (Object.keys(paths).length > 0) {
        for (let j = 0; j < paths.length; j++) {
          routes.push(paths[j]);
        }
      }
      paths = JSON.parse(libgpxparser.getTracksBetweenAsJSON("uploads/" + files[i], startLat, startLon, endLat, endLon, delta));
      if (Object.keys(paths).length > 0) {
        for (let j = 0; j < paths.length; j++) {
          tracks.push(paths[j]);
        }
      }
    }
  }
  res.status(200).send({
    routes: routes,
    tracks: tracks
  });
});

app.get('/otherdata', function(req, res) {
  let name = req.query.name;
  let index = req.query.index;
  let type = req.query.type;
  let otherData = JSON.parse(libgpxparser.getOtherDataAsJSON("uploads/" + name, index, type));
  res.send(otherData);
});

app.post('/rename', function(req, res) {
  let name = req.body.name;
  let index = req.body.index;
  let type = req.body.type;
  let newname = req.body.newname;
  let auth = req.body.auth;
  let s = 0;
  if (type === "Route") {
    s = libgpxparser.renameRoute("uploads/" + name, index, newname, xsdFile);
  }
  else if (type === "Track") {
    s = libgpxparser.renameTrack("uploads/" + name, index, newname, xsdFile);
  }
  else {
    res.status(400).send("Bad Request");
  }
  if (s && auth !== undefined) {
    renameDBRoute(name, auth, index, newname);
  }
  res.status(204).send();
});

app.get('/gpxinfo/length', function(req, res) {
  let len = req.query.len;
  let delta = req.query.delta;
  let files = fs.readdirSync("uploads");
  let numRoutes = 0;
  let numTracks = 0;
  if (len === undefined) {
    return res.status(422).send("Length is invalid");
  }
  for (let i = 0; i < files.length; i++) {
    if (libgpxparser.validateGPXFile("uploads/" + files[i], xsdFile)) {
      let routes = JSON.parse(libgpxparser.getGPXRoutesAsJSON("uploads/" + files[i]));
      let tracks = JSON.parse(libgpxparser.getGPXTracksAsJSON("uploads/" + files[i]));
      for (let j = 0; j < routes.length; j++) {
        if (Math.abs(routes[j].len - len) <= delta) {
          numRoutes++;
        }
      }
      for (let j = 0; j < tracks.length; j++) {
        if (Math.abs(tracks[j].len - len) <= delta) {
          numTracks++;
        }
      }
    }
  }
  res.status(200).send({
    numRoutes: numRoutes,
    numTracks: numTracks
  });
});

const mysql = require('mysql2/promise');

async function login(user, pass, db) {
  let connection;
  try {
    connection = await mysql.createConnection({
      host: 'dursley.socs.uoguelph.ca',
      user: user,
      password: pass,
      database: db
    });
    return 204;
  }
  catch (e) {
    console.log(e);
    if (e.code === 'ER_ACCESS_DENIED_ERROR') {
      return 401;
    }
    else if (e.code === 'ENOTFOUND') {
      return 404;
    }
  }
  finally {
    if (connection && connection.end) {
      connection.end();
    }
  }
}

async function executeQueries(user, pass, db, queries) {
  let connection;
  let responses = [];
  let i = 0;
  try {
    connection = await mysql.createConnection({
      host: 'dursley.socs.uoguelph.ca',
      user: user,
      password: pass,
      database: db
    });
    for (i = 0; i < queries.length; i++) {
      responses[i] = await connection.execute(queries[i]);
    }
  }
  catch (e) {
    for (; i < queries.length; i++) {
      repsonses[i] = null;
    }
  }
  finally {
    if (connection && connection.end) {
      connection.end();
    }
  }
  return responses;
}

async function createTables(user, pass, db) {
  let queries = ["CREATE TABLE IF NOT EXISTS FILE (" +
                  "gpx_id INT AUTO_INCREMENT, " +
                  "file_name VARCHAR(60) NOT NULL, " +
                  "ver DECIMAL(2,1) NOT NULL, " +
                  "creator VARCHAR(256) NOT NULL, " +
                  "PRIMARY KEY(gpx_id));",
                "CREATE TABLE IF NOT EXISTS ROUTE (" +
                  "route_id INT AUTO_INCREMENT, " +
                  "route_name VARCHAR(256), " +
                  "route_len FLOAT(15,7) NOT NULL, " +
                  "gpx_id INT NOT NULL, " +
                  "PRIMARY KEY(route_id), " +
                  "FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE);",
                "CREATE TABLE IF NOT EXISTS POINT (" +
                  "point_id INT AUTO_INCREMENT, " +
                  "point_index INT NOT NULL, " +
                  "latitude DECIMAL(11,7) NOT NULL, " +
                  "longitude DECIMAL(11,7) NOT NULL, " +
                  "point_name VARCHAR(256), " +
                  "route_id INT NOT NULL, " +
                  "PRIMARY KEY(point_id), " +
                  "FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE);"];
  executeQueries(user, pass, db, queries);
}

// Insert a GPX file into the database. Only new GPX files will be added.
async function insertFileInDB(file, auth) {
  let connection;
  try {
    connection = await mysql.createConnection({
      host: 'dursley.socs.uoguelph.ca',
      user: auth.user,
      password: auth.pass,
      database: auth.db
    });
    let [rows, fields] = await connection.execute("SELECT * FROM FILE WHERE file_name = '" + file.name + "';");
    if (rows.length > 0) {
      return false; // file already exists in database
    }
    [rows, fields] = await connection.execute("INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ('" + routes[i].name + "', " + routes[i].len + ", " + gpxid + ");");
    let gpxid = rows[0]["LAST_INSERT_ID()"];
    let routes = JSON.parse(libgpxparser.getGPXRoutesAsJSON("uploads/" + file.name));
    for (let i = 0; i < routes.length; i++) {
      [rows, fields] = await connection.execute("INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ('" + routes[i].name + "', " + routes[i].len + ", " + gpxid + ");");
      [rows, fields] = await connection.execute("SELECT LAST_INSERT_ID();");
      let routeid = rows[0]["LAST_INSERT_ID()"];
      let points = JSON.parse(libgpxparser.getRouteWaypointsAsJSON("uploads/" + file.name, i));
      for (let j = 0; j < points.length; j++) {
        [rows, fields] = await connection.execute("INSERT INTO POINT (point_index, latitude, longitude, point_name, route_id) VALUES (" + j + ", " + points[j].lat + ", " + points[j].lon + ", '" + points[j].name + "', " + routeid + ");");
      }
    }
    return true;
  }
  catch (e) {
    console.log(e);
    return false;
  }
  finally {
    if (connection && connection.end) {
      connection.end();
    }
  }
}

// Insert a GPX file into the database. Only new GPX files will be added.
async function addRouteToDBFile(file, auth) {
  let connection;
  try {
    connection = await mysql.createConnection({
      host: 'dursley.socs.uoguelph.ca',
      user: auth.user,
      password: auth.pass,
      database: auth.db
    });
    let [rows, fields] = await connection.execute("SELECT gpx_id FROM FILE WHERE file_name = '" + file + "';");
    if (rows.length === 0) {
      return false; // file is NOT in the database; do nothing
    }
    let gpxid = rows[0].gpx_id;
    let routes = JSON.parse(libgpxparser.getGPXRoutesAsJSON("uploads/" + file));
    let newroute = routes[routes.length-1];
    [rows, fields] = await connection.execute("INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ('" + newroute.name + "', " + newroute.len + ", " + gpxid + ");");
    [rows, fields] = await connection.execute("SELECT LAST_INSERT_ID();");
    let routeid = rows[0]["LAST_INSERT_ID()"];
    let points = JSON.parse(libgpxparser.getRouteWaypointsAsJSON("uploads/" + file, routes.length-1));
    for (let j = 0; j < points.length; j++) {
      [rows, fields] = await connection.execute("INSERT INTO POINT (point_index, latitude, longitude, point_name, route_id) VALUES (" + j + ", " + points[j].lat + ", " + points[j].lon + ", '" + points[j].name + "', " + routeid + ");");
    }
    return true;
  }
  catch (e) {
    console.log(e);
    return false;
  }
  finally {
    if (connection && connection.end) {
      connection.end();
    }
  }
}

// Insert a GPX file into the database. Only new GPX files will be added.
async function renameDBRoute(file, auth, index, newname) {
  let connection;
  try {
    connection = await mysql.createConnection({
      host: 'dursley.socs.uoguelph.ca',
      user: auth.user,
      password: auth.pass,
      database: auth.db
    });
    let [rows, fields] = await connection.execute("SELECT gpx_id FROM FILE WHERE file_name = '" + file + "';");
    if (rows.length === 0) {
      return false; // file is NOT in the database; do nothing
    }
    let gpxid = rows[0].gpx_id;
    [rows, fields] = await connection.execute("SELECT route_id FROM ROUTE WHERE gpx_id = " + gpxid + " ORDER BY route_id ASC;");
    let routeid = rows[index].route_id;
    [rows, fields] = await connection.execute("UPDATE ROUTE SET route_name = '" + newname + "' WHERE route_id = " + routeid + ";");
    return true;
  }
  catch (e) {
    console.log(e);
    return false;
  }
  finally {
    if (connection && connection.end) {
      connection.end();
    }
  }
}

app.post('/login', async function(req, res) {
  let user = req.body.user;
  let pass = req.body.pass;
  let db = req.body.db;
  let status = await login(user, pass, db);
  if (status == 401) {
    return res.status(401).send("Authentication failed");
  }
  else if (status == 404) {
    return res.status(404).send("Database server not found");
  }
  else if (status != 204) {
    return res.status(500).send("Connection failed");
  }
  await createTables(user, pass, db);
  res.status(204).send();
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
