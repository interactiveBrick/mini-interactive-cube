//
// Node.js based sequencer example
//

var midi = require('midi');
var osc = require('node-osc');

var Song = require('./src/core/song').Song;
var Player = require('./src/core/player').Player;

var DisplayHandler = require('./src/core/displayhandler.js').DisplayHandler;
var InputHandler = require('./src/core/inputhandler.js').InputHandler;
var UIHandler = require('./src/core/uihandler.js').UIHandler;

var PatternSelectorUI = require('./src/ui/patternselector.js').PatternSelectorUI;
var InstrumentSelectorUI = require('./src/ui/instrumentselector.js').InstrumentSelectorUI;
var PatternEditorUI = require('./src/ui/patterneditor.js').PatternEditorUI;
var PatternEditorUI2 = require('./src/ui/patterneditor2.js').PatternEditorUI2;
var PatternCueSelectorUI = require('./src/ui/patterncueselector.js').PatternCueSelectorUI;

var TrackEditorUI = require('./src/ui/trackeditor.js').TrackEditorUI;
var LiveTriggerUI = require('./src/ui/livetrigger.js').LiveTriggerUI;
var LevelEditorUI = require('./src/ui/leveleditor.js').LevelEditorUI;

var BeatUI = require('./src/ui/beat.js').BeatUI;


var CUBENAME = 'cube18FE';


//
// Set up application
//

var song = new Song();
var player = new Player();
var display = new DisplayHandler();
var input = new InputHandler();
var ui = new UIHandler();



//
// Set up mock UI webserver
//

var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);

app.get('/', function(req, res){
  res.sendfile('public/index.html');
});

io.on('connection', function(socket){
  console.log('a user connected');
  socket.on('chat message', function(msg){
    console.log('message: ' + msg);
  });
  socket.on('buttondown', function(msg){
    console.log('buttondown:', msg);
    input.buttonDown(msg.index);
  });
  socket.on('buttonup', function(msg){
    console.log('buttonup:', msg);
    input.buttonUp(msg.index);
  });
  socket.on('disconnect', function(){
    console.log('user disconnected');
  });
});

http.listen(3000, function(){
  console.log('listening on *:3000');
});



//
// Set up OSC input
//

var oscServer = new osc.Server(3333, '0.0.0.0');

oscServer.on("message", function (msg, rinfo) {
  console.log("TUIO message:", msg);
  if (msg[0] == '/' + CUBENAME + '/btn') {
    if (msg[2] == 1) {
      input.buttonDown(msg[1]);
    } else {
      input.buttonUp(msg[1]);
    }
  }
});



//
// Set up MIDI output
//

function MIDIDestination() {
  // this.notes = [];

  this.output = new midi.output();
  console.log('MIDI Destination: ' + this.output.getPortName(0));
  this.output.openPort(0);
}

MIDIDestination.prototype.tick = function(dT) {
  /*
  for(var j=this.notes.length-1; j>=0; j--) {
    this.notes[j].expires -= dT;
    if (this.notes[j].expires < 0.0) {
      // note off
      // console.log('Send note off:', this.notes[j].note);
      this.output.sendMessage([ 0x80, this.notes[j].note, 0 ]);

      // remove.
      this.notes.splice(j, 1);
    }
  }
  */
}

MIDIDestination.prototype.send = function(events) {
	// console.log('Send MIDI:', events);
  var _this = this;
  events.forEach(function(event) {
    // note on
    // console.log('Send note on:', event.note);
    _this.output.sendMessage([ (event.on ? 0x90 : 0x80) + event.channel, event.note, 100 ]);

    /*
    _this.notes.push({
      note: event.note,
      expires: 0.4
    })
    */
  });
}

player.addDestination(new MIDIDestination());



//
// Set up OSC display
//

var oscClient = new osc.Client('192.168.1.41', 3333);

function OSCDisplay() {
  this.queue = [];
  this.timer = 0;
}

OSCDisplay.prototype.render = function(displayinfo) {
	// console.log('OSCDisplay render', displayinfo);

  if (displayinfo.leds) {

    var bytes = [0,0,0,0, 0,0,0,0, 0,0,0,0];

    for(var s=0; s<6; s++) {
      for(var j=0; j<8; j++) {
        if (displayinfo.leds[s*16+0+j])
          bytes[s*2+0] |= (1 << j);
        if (displayinfo.leds[s*16+8+j])
          bytes[s*2+1] |= (1 << j);
      }
    }

    for(var j=0; j<4; j++) {
      var line = '';
      for(var s=0; s<6; s++) {
        for(var i=0; i<4; i++) {
          var o = s * 16 + j * 4 + i;
          line += displayinfo.leds[o] ? '#' : '.';
        }
        line += ' ';
      }
      console.log('> ' + line);
    }
    console.log('');

    this.queue.push([ '/' + CUBENAME + '/leds',
      bytes[0], bytes[1], bytes[2], bytes[3],
      bytes[4], bytes[5], bytes[6], bytes[7],
      bytes[8], bytes[9], bytes[10], bytes[11] ]);

  }

  /*
  var _this = this;
  if (displayinfo.changes) {
    displayinfo.changes.forEach(function(ch) {
      _this.queue.push([ '/cube1/led', ch.led, ch.on ? 1 : 0 ]);
    });
  }
  */

  this.queuePopQueue();
}

OSCDisplay.prototype.queuePopQueue = function() {
  clearTimeout(this.timer);
  this.timer = setTimeout(this.popQueue.bind(this), 3);
}

OSCDisplay.prototype.popQueue = function() {
  if (this.queue.length < 1) {
    return;
  }

  var _this = this;
  var item = this.queue.shift();
  // console.log('OSC Sending:', item);
  oscClient.send(item[0], item[1], item[2], item[3], item[4], item[5], item[6], item[7], item[8], item[9], item[10], item[11], item[12], function() {
    // client.kill();
    _this.queuePopQueue();
  });
}

display.addDisplay(new OSCDisplay());



//
// Set up WS display
//

function WSDisplay() {}

WSDisplay.prototype.render = function(displayinfo) {
	// console.log('WSDisplay render', displayinfo);
  // io.sockets.emit('display', displayinfo);
  io.sockets.emit('display', { changes: displayinfo.changes });
}

display.addDisplay(new WSDisplay());


//
// Set up input controls
//

player.song = song;
player.bpm = 125;

display.player = player;

ui.player = player;
ui.display = display;
ui.song = song;

// ui.addUI(new PatternSelectorUI([ 12,13,14,15 ]));
// ui.addUI(new InstrumentSelectorUI([0,1,2,3, 4,5,6,7]));
// ui.addUI(new PatternEditorUI([ 16,17,18,19, 20,21,22,23, 24,25,26,27, 28,29,30,31 ]));

function IDGen(side, startcol, startrow, numcols, numrows) {
  var ret = [];
  for(var j=0; j<numrows; j++) {
    for(var i=0; i<numcols; i++) {
      var y = startrow + j;
      var x = startcol + i;
      ret.push((side * 16) + (y * 4) + x);
    }
  }
  return ret;
}

ui.addUI(new PatternSelectorUI(IDGen( 4, 0,3, 4,1 )));
ui.addUI(new InstrumentSelectorUI(IDGen( 4, 0,0, 4,2 )));
ui.addUI(new PatternCueSelectorUI(IDGen( 4, 0,2, 4,1 )));

ui.addUI(new PatternEditorUI2(IDGen( 0, 0,0, 4,4 ), 0, 0));
ui.addUI(new PatternEditorUI2(IDGen( 1, 0,0, 4,4 ), 4, 0));
ui.addUI(new PatternEditorUI2(IDGen( 2, 0,0, 4,4 ), 8, 0));
ui.addUI(new PatternEditorUI2(IDGen( 3, 0,0, 4,4 ), 12, 0));

ui.addUI(new LiveTriggerUI(IDGen( 5, 0,0, 4,4 )));

// ui.addUI(new BeatUI([ 64,65,66,67, 68,69,70,71, 72,73,74,75, 76,77,78,79 ]));
// ui.addUI(new BeatUI([ 80,81,82,83, 84,85,86,87, 88,89,90,91, 92,93,94,95 ]));
// ui.addUI(new PatternEditorUI([ 0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15 ]));

input.addListener(ui);



//
// Start everything
//

input.start();
player.start();
ui.start();


//
// Mock track
//

song.tracks[0].patterns[0].steps[0].notes = [ { note: 36, channel: 0 } ];
song.tracks[0].patterns[0].steps[4].notes = [ { note: 36, channel: 0 }, { note: 38, channel: 0 } ];
song.tracks[0].patterns[0].steps[8].notes = [ { note: 36, channel: 0 } ];
song.tracks[0].patterns[0].steps[12].notes = [ { note: 36, channel: 0 }, { note: 38, channel: 0 } ];

song.tracks[0].patterns[0].steps[2].notes = [ { note: 44, channel: 0 } ];
song.tracks[0].patterns[0].steps[6].notes = [ { note: 44, channel: 0 } ];
song.tracks[0].patterns[0].steps[10].notes = [ { note: 44, channel: 0 } ];
song.tracks[0].patterns[0].steps[14].notes = [ { note: 44, channel: 0 } ];

song.tracks[0].patterns[0].steps[13].notes = [ { note: 50, channel: 0 } ];

//
// Application is running...
//
