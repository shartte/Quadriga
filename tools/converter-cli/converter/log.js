
var eventbus = require("eventbus");

exports.trace = function(message) {
    /* do not print trace */
};

exports.info = function(message) {

    console.log("[INFO]", message);

    eventbus.publish("log", {
                         level: "info",
                         message: message
                     });

};
