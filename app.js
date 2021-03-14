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

var libgpxparser = ffi.Library(__dirname + "/libgpxparser", {
  "validateGPXFile": ["int", ["string", "string"]],
  "getAllValidGPXFilesAsJSON": ["string", ["string", "string"]],
  "getGPXFileAsJSON": ["string", ["string", "string"]],
  "getGPXRoutesAsJSON": ["string", ["string"]],
  "getGPXTracksAsJSON": ["string", ["string"]]
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

//Sample endpoint
app.get('/endpoint1', function(req, res) {
  let retStr = req.query.stuff + " " + req.query.junk;
  res.send({
    stuff: retStr
  });
});

app.get('/uploads', function(req, res) {
  let retJson = JSON.parse(libgpxparser.getAllValidGPXFilesAsJSON("uploads", xsdFile));
  res.send({
    files: retJson
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

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);