
var output = require("./output"),
    loadMes = require("./mesparser").loadMes,
    isNotExcluded = require("./exclusions").isNotExcluded,
    log = require("log"),
    vfs = require("vfs");

var blackRefTile, blackRefTileString;
var removedTiles = 0;

function convertMapTiles() {
    log.trace("Converting map tiles for {}", this);

    var files = vfs
        .listDirectory(this)
        .filter(function(path) {
            return path.toLowerCase().substr(-4) === ".jpg";
        });

    if (files.length === 0) {
        log.warn("Empty map directory: {}", this);
        return;
    }

    var bounds;

    for (var i = 0; i < files.length; ++i) {
        var path = files[i];
        var results = path.match(/^.*([0-9a-f]{4})([0-9a-f]{4})\.jpg$/i);
        if (!results) {
            return;
        }

        var x = parseInt(results[2], 16),
            y = parseInt(results[1], 16);
        if (bounds === undefined) {
            bounds = {
                minX: x,
                maxX: x,
                minY: y,
                maxY: y
            };
        } else {
            if (x < bounds.minX) {
                bounds.minX = x;
            }
            if (x > bounds.maxX) {
                bounds.maxX = x;
            }
            if (y < bounds.minY) {
                bounds.minY = y;
            }
            if (y > bounds.maxY) {
                bounds.maxY = y;
            }
        }

        // Copy file to target
        var file = vfs.readFileRaw(path);

        if (file.uncompressedSize === blackRefTile.uncompressedSize) {
            removedTiles++;
            continue; // Skip this.
        }

        output.addRawBuffer("groundmaps", path, file.buffer, file.compressed, file.uncompressedSize);
    }

    output.addString("groundmaps", this + "/bounds.json", JSON.stringify(bounds));

}

exports.run = function() {

    // Load the reference tile against which other tiles will be compared
    blackRefTile = vfs.readFileRaw("art/ground/Map-2-Hommlet-Exterior/0011000F.jpg");
    blackRefTileString = blackRefTile.toString("hex");

    // Collect all ground maps that need conversion
    var groundMes = loadMes("art/ground/ground.mes");

    var directories = [];
    for (var k in groundMes) {
        if (!groundMes.hasOwnProperty(k))
            continue;

        if (directories.indexOf(groundMes[k]) === -1) {
            directories.push(groundMes[k]);
        }
    }

    directories = directories
                    .map(function(name) {
                        return "art/ground/" + name + "/";
                    })
                    .filter(isNotExcluded)
                    .map(function (path) {
                        return convertMapTiles.bind(path);
                    });

    log.info("Found {} ground maps to convert.", directories.length);

    directories.push(function() {
        log.info("Removed {} map tiles (size: {} kb) because they were 100% black", removedTiles, Math.floor(removedTiles * blackRefTile.uncompressedSize / 1024));
    });
    return directories;

};
