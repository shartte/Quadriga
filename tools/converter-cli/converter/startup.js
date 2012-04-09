
var eventbus = require("eventbus"); /* Eventbus */
var worker = require("worker"); /* Worker */
var buffer = require("buffer");
var vfs = require("vfs");

exports.startup = function(guiRoot) {

    guiRoot.state = "choosePath";

    console.log(guiRoot.pathChooser);

    guiRoot.pathChooser.path = vfs.guessGamePath();
    guiRoot.pathChooser.ok.connect(function() {
        guiRoot.state = "conversion";
        require("conversion").run(guiRoot.pathChooser.path + "/");
    });

    /*
    worker.start(function (context) {
      console.log("Running some heavy computational stuff.");
    });
    var partsys = vfs.readFileAsString("rules/partsys0.tab");
    console.log("PartSys: " + partsys);

    var b = new buffer.Buffer(123);
    b.writeInt32LE(123456, 0);
    console.log("INT@0:", b.readInt32LE(0));

    var gamePath = vfs.guessGamePath();
    console.log("Guessed game path: " + gamePath);

    if (gamePath) {
        var result = vfs.validateGamePath(gamePath);
        if (!result.valid) {
            console.log("Game path validation failed.", JSON.stringify(result));
        } else {
            vfs.addDefaultArchives(gamePath);
        }
    }

*/
};
