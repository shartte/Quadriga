#ifndef TROIKAFORMATS_COLOR_H
#define TROIKAFORMATS_COLOR_H

struct TroikaColor
{
public:
    TroikaColor() : r(0), g(0), b(0), a(255) {
    }

    TroikaColor(quint8 r, quint8 g, quint8 b, quint8 a = 255) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    quint8 r;
    quint8 g;
    quint8 b;
    quint8 a;
};

#endif // TROIKAFORMATS_COLOR_H
