
var vfs = require("vfs"),
    log = require("log"),
    buffer = require("buffer"),
    image = require("image"),
    output = require("./output");

var tileSize = 256;

function assembleImage(path) {

    // Strip extension
    path = path.substr(0, path.length - 4);

    var imgDeclBuffer = vfs.readFile(path + ".img"),
        w = imgDeclBuffer.readInt16LE(0),
        h = imgDeclBuffer.readInt16LE(2);

    log.trace("Image {}: {}x{}", path, w, h);

    var xTiles = Math.ceil(w / tileSize),
        yTiles = Math.ceil(h / tileSize),
        tiles = [],
        imgBuffer;

    for (var y = 0; y < yTiles; ++y) {
        for (var x = 0; x < xTiles; ++x) {
            imgBuffer = vfs.readFile(path + "_" + x + "_" + y + ".tga");
            tiles.push(imgBuffer);
        }
    }

    var result = image.convertTargasToPng(w, h, tiles);
    output.addBuffer("images", path + ".png", result);
}

exports.run = function() {

    log.info("Starting image conversion...");

    var imageFiles = vfs.listAllFiles("*.img");

    console.info("Found", imageFiles.length, "tiled images to convert.");

    imageFiles.forEach(function (path) {
        assembleImage(path);
    });

};
