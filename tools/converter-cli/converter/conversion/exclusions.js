
var exclusions = [
    // Broken Models
    "art/meshes/particle/test",
    "art/meshes/scenery/foliage/tree1",
    "art/meshes/player/walker",

    // These simply don't exist
    "art/meshes/npcs/calmert/calmert",
    "art/meshes/npcs/terjon/terjon",
    "art/meshes/npcs/jaroo/jaroo",
    "art/meshes/pcs/thief/thief",
    "art/meshes/pcs/dwarf/dwarf",
    "art/meshes/pcs/sorceress/sorceress",
    "art/meshes/pcs/ranger/ranger",
    "art/meshes/pcs/paladin/paladin",
    "art/meshes/pcs/barbarian/barbarian",
    "art/meshes/helms/pc_leather_helm",
    "art/meshes/particle/golden_orb_of_death",
    "art/meshes/particle/gargoyle-climb2fly",
    "art/meshes/particle/earthelem-death",
    "art/meshes/particle/grassblade",

    // Various test maps
    "art/ground/4096test",
    "art/ground/testarea",
    "art/ground/battle-royal",
    "art/ground/clip-test",
    "art/ground/hommletsketch",
    "art/ground/hommlet",
    "art/ground/--temp--",

    // Broken Materials
    "art/clip/"
];

var normalizeSeparators = /[\/\\]+/g;

exports.isExcluded = function isExcluded(path) {
    path = path.toLowerCase();

    path = path.replace(normalizeSeparators, "/");

    /* Normalize path separators */
    if (path[0] === "/")
        path = path.substr(1);

    var excludedPath;
    for (var i = 0; i < exclusions.length; ++i) {
        excludedPath = exclusions[i];
        if (path.indexOf(excludedPath) === 0)
            return true;
    }
    return false;
};


exports.isNotExcluded = function isNotExcluded(path) {
    return !exports.isExcluded(path);
};
