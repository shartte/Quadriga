
var vfs = require("vfs"),
    log = require("log"),
    scheduling = require("scheduling"),
    material = require("conversion/material"),
    output = require("conversion/output"),
    exclusions = require("conversion/exclusions"),
    ogreMaterialWriter = require("conversion/ogrematerial");

function convertMaterial(path) {

    if (exclusions.isExcluded(path)) {
        log.info("Skipping {}", path);
        return;
    }

    log.trace("Converting material {}", path);
    return material.parseMaterial(path);
}

function copyTexture(texture) {
    var buffer = vfs.readFile(texture);
    if (!buffer) {
        log.warn("Texture not found: {}", texture);
    } else {
        output.addBuffer("textures", texture, buffer);
    }
}

exports.run = function() {

    var filesToConvert = vfs.listAllFiles("*.mdf");

    log.info("Found {} material files to convert.", filesToConvert.length);

    var texturesToCopy = [];

    var tasks = [];

    filesToConvert.forEach(function (path) {
        tasks.push(function() {
            var mat = convertMaterial(path);

            if (!mat) {
                return;
            }

            var ogreMaterial = ogreMaterialWriter.createOgreMaterial(mat);
            path = path.replace(".mdf", ".material");
            output.addString("materials", path, ogreMaterial);

            for (var i = 0; i < material.Material.TextureStageCount; ++i) {
                var ts = mat.textureStage(i);
                if (ts.filename && texturesToCopy.indexOf(ts.filename) === -1) {
                    texturesToCopy.push(ts.filename);
                }
            }
        });
    });

    // Convert textures
    tasks.push(function() {
        log.info("Copying {} textures.", texturesToCopy.length);
        return texturesToCopy.map(function (texture) {
            return function() {
                copyTexture(texture);
            };
        });
    });

    return tasks;

};
