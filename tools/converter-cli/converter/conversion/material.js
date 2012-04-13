
var vfs = require("vfs"),
    log = require("log"),
    WHITESPACE = [" ", "\t"],
    QUOTE = "\"";


function Material(path) {
    this.path = path;
    this.textureStages = [];

    for (var i = 0; i < Material.TextureStageCount; ++i) {
        this.textureStages[i] = {
            blendType: "modulate",
            uvType: "mesh",
            speedU: 1,
            speedV: 0
        };
    }
}

Material.UvTypes = ["mesh", "drift", "swirl", "wavey", "environment"];

Material.BlendTypes = ["modulate", "add", "texturealpha", "currentalpha", "currentalphaadd"];

Material.TextureStageCount = 3;

Material.prototype.textureStage = function(unit) {
    var textureStage = this.textureStages[unit];
    if (!textureStage) {
        throw new Error("Invalid texture stage unit: " + unit);
    }
    return textureStage;
}

function isWhitespace(ch) {
    return WHITESPACE.indexOf(ch) !== -1;
}

function tokenizeLine(line) {
    line = line.trim(); // Trim first

    var inQuotes = false; // No escape characters allowed
    var tokenBuffer = [], ch, tokens = [];

    for (var i = 0; i < line.length; ++i) {
        ch = line[i];

        if (isWhitespace(ch) && !inQuotes) {
            if (tokenBuffer.length > 0) {
                tokens.push(tokenBuffer.join(""));
                tokenBuffer = [];
            }
        } else if (ch === QUOTE) {
            if (inQuotes) {
                tokens.push(tokenBuffer.join(""));
                tokenBuffer = [];
            }
            inQuotes = !inQuotes;
        } else {
            tokenBuffer.push(ch);
        }
    }

    // Dont forget about the last token
    if (tokenBuffer.length) {
        tokens.push(tokenBuffer.join(""));
    }

    return tokens;
}

function processCommand(path, material, command, args) {

    var stage;

    switch (command) {
    case "texture":
        switch (args.length) {
        case 1:
            material.textureStage(0).filename = args[0];
            break;
        case 2:
            material.textureStage(args[0]).filename = args[1];
            break;
        default:
            throw new Error("Invalid texture command in " + path);
        }
        break;

    case "glossmap":
        if (args.length !== 1)
            throw new Error("Invalid glossmap command in " + path);
        material.glossMap = args[0];
        break;
    case "colorfillonly":
        material.depthTestDisabled = true;
        break;
    case "blendtype":
        if (args.length !== 2)
            throw new Error("Invalid blendtype command in " + path);

        // TODO: Validate blend type
        material.textureStage(args[0]).blendType = args[1].toLowerCase();
        break;

    case "speed":
        if (args.length !== 1)
            throw new Error("Invalid speed command in " + path);

        var speed = parseFloat(args[0]);

        for (var i = 0; i < Material.TextureStageCount; ++i) {
            material.textureStage(i).speedU = speed;
            material.textureStage(i).speedV = speed;
        }

        break;

    case "specularpower":
        if (args.length !== 1)
            throw new Error("Invalid specularpower command in " + path);

        material.specularPower = parseFloat(args[0]);
        break;

    case "speedu":
    case "speedv":
        if (args.length !== 2)
            throw new Error("Invalid " + command + " command in " + path);

        var unit = parseInt(args[0], 10);
        var speed = parseFloat(args[1]);

        if (command === "speedu")
            material.textureStage(unit).speedU = speed;
        else
            material.textureStage(unit).speedV = speed;

        break;

    case "uvtype":
        if (args.length !== 2)
            throw new Error("Invalid uvtype command in " + path);

        // TODO: Validate
        var unit = parseInt(args[0], 10);
        material.textureStage(unit).uvType = args[1].toLowerCase();
        break;

    case "clamp":
        material.clamp = true;
        break;

    case "materialblendtype":
        if (args.length !== 1)
            throw new Error("Invalid materialblendtype command in " + path);

        // TODO: Validate
        material.blendType = args[0].toLowerCase();
        break;

    case "double":
        material.faceCullingDisabled = true;
        break;

    case "notlit":
        material.lightingDisabled = true;
        break;

    case "disablez":
        material.depthTestDisabled = true;
        break;

    case "general":
    case "highquality":
        // This was previously used by the material system to define materials
        // for different quality settings. The current hardware performance makes this
        // unneccessary. A better way to deal with this could be to ignore the "general"
        // definition completely.
        break;

    case "linearfiltering":
        material.linearFiltering = true;
        break;

    case "textured":
        // Unused
        break;

    case "recalculatenormals":
        material.recalculateNormals = true;
        break;

    case "color":
        if (args.length !== 4)
            throw new Error("Invalid color command found in " + path);

        material.color = [args.map(function (s) { return s / 255.0; })]
        break;

    default:
        throw new Error("Unknown material command '" + command + "' found in " + path);
    }

}

function parseMaterial(path) {

    var materialScript = vfs.readFileAsString(path);

    if (!materialScript)
        throw new Error("Material not found: " + path);

    var lines = materialScript
                .split(/\n/)
                .map(tokenizeLine)
                .filter(function (l) { return l.length > 0; });

    var material = new Material(path);

    lines.forEach(function (line) {
        var command = line[0].toLowerCase();

        try {
            processCommand(path, material, command, line.slice(1));
        } catch (e) {
            log.warn(e + " (" + line.join(" ") + ")");
        }
    });

    return material;
}


exports.parseMaterial = parseMaterial;
exports.Material = Material;
