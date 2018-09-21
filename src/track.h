#ifndef TRACK_H
#define TRACK_H

#include <QString>

typedef enum {
    TrackTypeVideo,
    TrackTypeAudio,
    TrackTypeSubtitles
} TrackType;

struct Track {
    int id;
    TrackType type;
    QString title;
    bool selected;
    QString language;
};

#endif // TRACK_H
