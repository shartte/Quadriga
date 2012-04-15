
var log = require("log"),
    vfs = require("vfs"),
    Plan = require("plan").Plan;

exports.run = function(gamePath) {
    log.info("Starting asset conversion...");

    vfs.addDefaultArchives(gamePath);

    /* Create the output categories */
    // output.openCategoryAsDirectory("interface", "");

    var plan = new Plan();
    /*plan.add(require("conversion/meshes").run);
    plan.add(require("conversion/materials").run);
    plan.add(require("conversion/images").run);
    plan.add(require("conversion/maptiles").run);*/
    plan.add(require("conversion/depthart").run);

    plan.run(function () {
        log.info("Finished!");
    });
};
