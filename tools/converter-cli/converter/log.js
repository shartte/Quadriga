
var eventbus = require("eventbus");

exports.info = function(message) {

    console.log("[INFO]", message);

    eventbus.publish("log", {
                         level: "info",
                         message: message
                     });

};
