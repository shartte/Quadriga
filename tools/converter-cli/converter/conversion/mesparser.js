
var vfs = require("vfs"),
    log = require("log");

var LeftBracket = '{';
var RightBracket = '}';
var Newline = '\n';
var Slash = '/';

var ParserState = {
    WaitingForKey: 0, // Everything until {
    WaitingForKeyComment: 1, // Everything after // until newline, while waiting for key
    Key: 2,
    WaitingForValue: 3,
    WaitingForValueComment: 4, // Everything after // until newline, while waiting for value
    Value: 5
};

function parseMes(content) {
    var result = {},
        state = ParserState.WaitingForKey,
        previous = '\0',
        key = [],
        keyNumber,
        value = [],
        c;

    for (var i = 0; i < content.length; ++i)
    {
        c = content.charAt(i);

        switch (state)
        {
            case ParserState.WaitingForKey:
                if (c === LeftBracket)
                {
                    state = ParserState.Key;
                }
                else if (c === Slash && previous === Slash)
                {
                    state = ParserState.WaitingForKeyComment;
                }
                break;
            case ParserState.WaitingForKeyComment:
                if (c === Newline)
                {
                    state = ParserState.WaitingForKey;
                }
                break;
            case ParserState.Key:
                if (c === RightBracket)
                {
                    state = ParserState.WaitingForValue;
                }
                else
                {
                    key.push(c);
                }
                break;
            case ParserState.WaitingForValue:
                if (c === LeftBracket)
                {
                    state = ParserState.Value;
                }
                else if (c === Slash && previous == Slash)
                {
                    state = ParserState.WaitingForValueComment;
                }
                break;
            case ParserState.WaitingForValueComment:
                if (c === Newline)
                {
                    state = ParserState.WaitingForValue;
                }

                break;
            case ParserState.Value:
                if (c === RightBracket)
                {
                    // Try parsing the key
                    keyNumber = parseInt(key.join(""), 10);
                    if (isNaN(keyNumber)) {
                        log.warn("Invalid key for message: {}", key.join(""));
                    } else {
                        result[keyNumber] = value.join("");
                    }

                    value = [];
                    key = [];
                    state = ParserState.WaitingForKey;
                }
                else
                {
                    value.push(c);
                }
                break;
        }

        previous = c;
    }

    return result;
}

function loadMes(path) {
    var content = vfs.readFileAsString(path);

    if (!content)
        throw new Error("Message file not found: " + path);

    return parseMes(content);
}

exports.parseMes = parseMes;
exports.loadMes = loadMes;
