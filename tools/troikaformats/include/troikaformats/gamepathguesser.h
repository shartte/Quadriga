#ifndef TROIKAFORMATS_GAMEPATHGUESSER_H
#define TROIKAFORMATS_GAMEPATHGUESSER_H

#include <QString>

class GamePathGuesser
{
public:
    /**
      Attempts to guess where ToEE is installed.
      Will return undefined if ToEE is not installed or the path could not
      be determined.
      */
    static QString guessGamePath();
};

#endif // TROIKAFORMATS_GAMEPATHGUESSER_H
