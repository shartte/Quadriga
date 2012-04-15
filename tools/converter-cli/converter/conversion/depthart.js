
var vfs = require("vfs"),
    output = require("conversion/output"),
    log = require("log");

function convertDepthArt(path) {
    log.trace("Converting {}", path);

    var buffer = vfs.readFile(path);

    if (!buffer) {
        console.warn("Unable to read {}", path);
        return; // Why did we list it in the first place?
    }


}

exports.run = function() {

    var depthArt = vfs.listAllFiles("*.dag");

    return depthArt.map(function (path) {
        return convertDepthArt.bind(null, path);
    });

};
