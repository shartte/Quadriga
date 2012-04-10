
var vfs = require("vfs"),
    log = require("log"),
    buffer = require("buffer"),
    image = require("image"),
    output = require("./output"),
    util = require("./util"),
    scheduling = require("scheduling");

var tileSize = 256;

var exclusions = [
    'art/interface/PARTY_QUICKVIEW_UI/nav_background.img',
    'art/interface/PARTY_QUICKVIEW_UI/background.img'
];

function assembleImage(path) {

    if (exclusions.indexOf(path) !== -1) {
        log.trace("Skipping {} because it is excluded.", path);
        return; // Skip
    }

    // Strip extension
    path = path.substr(0, path.length - 4);

    var imgDeclBuffer = vfs.readFile(path + ".img"),
        w = imgDeclBuffer.readInt16LE(0),
        h = imgDeclBuffer.readInt16LE(2);

    log.trace("Image {}: {}x{}", path, w, h);

    var xTiles = Math.ceil(w / tileSize),
        yTiles = Math.ceil(h / tileSize),
        tiles = [],
        imgBuffer,
        filename;

    for (var y = 0; y < yTiles; ++y) {
        for (var x = 0; x < xTiles; ++x) {
            filename = path + "_" + x + "_" + y + ".tga";
            exclusions.push(filename); /* Do not convert it a second time */
            imgBuffer = vfs.readFile(filename);
            tiles.push(imgBuffer);
        }
    }

    var result = image.convertTargasToPng(w, h, tiles);
    output.addBuffer("interface", path + ".png", result);
}

function convertImage(path) {

    if (exclusions.indexOf(path) !== -1)
        return;

    var imgBuffer = vfs.readFile(path),
        pngBuffer,
        newFilename = util.changeExtension(path, "png");

    if (imgBuffer) {
        pngBuffer = image.convertTargaToPng(imgBuffer);
        if (pngBuffer) {
            output.addBuffer("interface", newFilename, pngBuffer);
        } else {
            log.warn("Failed to convert {}", path);
        }
    }

}

exports.run = function() {

    log.info("Starting image conversion...");

    var imageFiles = vfs.listAllFiles("*.img");

    log.info("Found {} tiled images to convert.", imageFiles.length);

    var executionPlan = [];

    imageFiles.forEach(function(img) {
        executionPlan.push(function() {
            assembleImage(img);
        });
    });

    imageFiles = vfs.listAllFiles("*.tga");

    imageFiles = imageFiles.filter(function (path) {
        return path.indexOf("art/interface/") === 0;
    });

    log.info("Found {} normal images to convert.", imageFiles.length);

    imageFiles.forEach(function (path) {
        executionPlan.push(function() {
            convertImage(path);
        });
    });

    scheduling.defer(executionPlan);

};
