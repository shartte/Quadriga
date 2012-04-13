
var log = require("log"),
    vfs = require("vfs"),
    output = require("conversion/output"),
    Plan = require("plan").Plan;

exports.run = function(gamePath) {
    log.info("Starting asset conversion...");

    vfs.addDefaultArchives(gamePath);

    /* Create the output categories */
    // output.openCategoryAsDirectory("interface", "");

    var plan = new Plan();
    plan.add(require("conversion/materials").run);
    plan.add(require("conversion/images").run);

    plan.run(function () {
        log.info("Finished!");
    });
};
