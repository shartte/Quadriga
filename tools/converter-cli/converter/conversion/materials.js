
var vfs = require("vfs"),
    log = require("log"),
    scheduling = require("scheduling"),
    material = require("conversion/material");

var exclusions = [
    "art/clip/"
];

function isExcluded(path) {
    var excludedPath;
    for (var i = 0; i < exclusions.length; ++i) {
        excludedPath = exclusions[i];
        if (path.indexOf(excludedPath) === 0)
            return true;
    }
    return false;
}

function convertMaterial(path) {

    if (isExcluded(path)) {
        log.info("Skipping {}", path);
        return;
    }

    log.trace("Converting material {}", path);

    return material.parseMaterial(path);
}

exports.run = function() {

    var filesToConvert = vfs.listAllFiles("*.mdf");

    log.info("Found {} material files to convert.", filesToConvert.length);

    var texturesToCopy = [];

    var tasks = filesToConvert.map(function (path) {
        return function() {
            var mat = convertMaterial(path);

            if (!mat) {
                return;
            }

            for (var i = 0; i < material.Material.TextureStageCount; ++i) {
                var ts = mat.textureStage(i);
                if (ts.filename && texturesToCopy.indexOf(ts.filename) === -1) {
                    texturesToCopy.push(ts.filename);
                }
            }
        }
    });

    // Convert textures
    tasks.push(function() {
        log.info("Found {} textures in total.", texturesToCopy.length)
    });

    return tasks;

};
