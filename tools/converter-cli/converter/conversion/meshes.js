
var mesparser = require("./mesparser"),
    log = require("log"),
    vfs = require("vfs"),
    modelconverter = require("native/modelconverter"),
    output = require("conversion/output"),
    exclusions = require("conversion/exclusions");

function resolveMaterial(material) {
    var extension = material.lastIndexOf(".");
    if (extension !== -1)
        material = material.substr(0, extension);
    return material.toLowerCase();
}

function convertMesh(path) {

    if (exclusions.isExcluded(path)) {
        log.trace("Skipping {}", path);
        return;
    }

    log.trace("Converting {}", path);

    var model = vfs.readFile(path + ".skm");
    if (!model) {
        log.error("Unable to find mesh {}", path + ".skm");
        return;
    }

    var skeleton = vfs.readFile(path + ".ska");
    var converted;

    try {
        var resolveSkeleton = function(skeleton) { return path + ".skeleton"; };

        converted = modelconverter.convert(model, skeleton, resolveMaterial, resolveSkeleton);
    } catch (e) {
        log.error("Unable to convert {}: {}", path, e);
        return;
    }

    output.addBuffer("models", path + ".mesh", converted.mesh);
    if (converted.skeleton) {
        output.addBuffer("models", path + ".skeleton", converted.skeleton);
    }

}

exports.run = function() {

    var meshes = mesparser.loadMes("art/meshes/meshes.mes");
    var requireConversion = [];

    for (var k in meshes) {
        if (!meshes.hasOwnProperty(k))
            continue;
        // Fix filenames
        meshes[k] = "art/meshes/" + meshes[k].replace("\\", "/");

        if (requireConversion.indexOf(meshes[k]) === -1) {
            requireConversion.push(meshes[k]); // Deduplication?
        }
    }

    return requireConversion.map(function(path) {
        return function() {
            convertMesh(path);
        };
    });

};
