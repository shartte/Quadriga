
exports.changeExtension = function(filename, to) {
    var extensionIndex = filename.lastIndexOf(".");

    if (extensionIndex === -1)
        return filename + "." + to;

    return filename.substr(0, extensionIndex + 1) + to;
}
