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
  "addRouteToGPXFile": ["int", ["string", "string", "string"]],
  "getRouteAsJSON": ["string", ["string", "string"]],
  "getRoutesBetweenAsJSON": ["string", ["string", "float", "float", "float", "float"]],
  "getTracksBetweenAsJSON": ["string", ["string", "float", "float", "float", "float"]],
  "getOtherDataAsJSON": ["string", ["string", "int", "string"]],
  "renameRoute": ["int", ["string", "int", "string"]],
  "renameTrack": ["int", ["string", "int", "string"]]
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

app.post("/addroute", function(req, res) {
  let file = req.body.file;
  let name = req.body.name;
  let waypoints = req.body.waypoints;
  let status = libgpxparser.addRouteToGPXFile("uploads/" + file, name, JSON.stringify(waypoints));
  if (status == 0) {
    return res.status(422).send("Route could not be added.");
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
  let routes = [];
  let tracks = [];
  let files = fs.readdirSync("uploads");
  for (let i = 0; i < files.length; i++) {
    let paths = JSON.parse(libgpxparser.getRoutesBetweenAsJSON("uploads/" + files[i], startLat, startLon, endLat, endLon));
    if (Object.keys(paths).length > 0) {
      for (let j = 0; j < paths.length; j++) {
        routes.push(paths[j]);
      }
    }
    paths = JSON.parse(libgpxparser.getTracksBetweenAsJSON("uploads/" + files[i], startLat, startLon, endLat, endLon));
    if (Object.keys(paths).length > 0) {
      for (let j = 0; j < paths.length; j++) {
        tracks.push(paths[j]);
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
  let s = 0;
  if (type === "Route") {
    s = libgpxparser.renameRoute("uploads/" + name, index, newname);
  }
  else if (type === "Track") {
    s = libgpxparser.renameTrack("uploads/" + name, index, newname);
  }
  else {
    res.status(400).send("Bad Request");
  }
  res.status(204).send();
});

app.get('/gpxinfo/length', function(req, res) {
  let len = req.query.len;
  let files = fs.readdirSync("uploads");
  let numRoutes = 0;
  let numTracks = 0;
  if (len === undefined) {
    return res.status(422).send("Length is invalid");
  }
  for (let i = 0; i < files.length; i++) {
    let routes = JSON.parse(libgpxparser.getGPXRoutesAsJSON("uploads/" + files[i]));
    let tracks = JSON.parse(libgpxparser.getGPXTracksAsJSON("uploads/" + files[i]));
    for (let j = 0; j < routes.length; j++) {
      if (Math.abs(routes[j].len - len) <= 10) {
        numRoutes++;
      }
    }
    for (let j = 0; j < tracks.length; j++) {
      if (Math.abs(tracks[j].len - len) <= 10) {
        numTracks++;
      }
    }
  }
  res.status(200).send({
    numRoutes: numRoutes,
    numTracks: numTracks
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
