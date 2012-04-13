
var scheduling = require("scheduling"),
    log = require("log");

function Plan() {
    this.elements = [];
}

Plan.prototype.add = function (task) {
    this.elements.push(task);
};

Plan.prototype.run = function(finishCallback) {

    var self = this;

    function runStep() {
        var nextElement = self.elements.shift();
        if (!nextElement) {
            log.trace("No more plan elements. Stopping.");
            if (finishCallback)
                finishCallback();
            return;
        }

        // Run the next plan element.
        var result = nextElement();
        if (result instanceof Array) {
            log.trace("Adding {} more plan elements after running plan element.", result.length);
            result.forEach(function (newTask) {
                self.elements.unshift(newTask);
            });
        }

        scheduling.defer(runStep);
    }

    scheduling.defer(runStep);

};

exports.Plan = Plan;
