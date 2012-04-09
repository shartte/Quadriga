
var eventbus = require("eventbus"),
    log = require("log"),
    vfs = require("vfs");

exports.run = function(gamePath) {
    log.info("Starting asset conversion...");

    vfs.addDefaultArchives(gamePath);

    var images = require("conversion/images");
    images.run();
};
