
var vfs = require("vfs"),
    log = require("log"),
    buffer = require("buffer");

function assembleImage(path) {

    var slowBuffer = vfs.readFile(path);
    var imgBuffer = new buffer.Buffer(slowBuffer, slowBuffer.length, 0);

    console.log(imgBuffer.inspect());

    var w = imgBuffer.readInt16LE(0),
        h = imgBuffer.readInt16LE(2);
    log.info("Image " + path + ": " + w + ", " + h);
}

exports.run = function() {

    log.info("Starting image conversion...");

    assembleImage("art/splash/legal0322.img");

};
