#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <mpv/client.h>
#include <mpv/opengl_cb.h>
#include <mpv/qthelper.hpp>

#include <QtWidgets/QOpenGLWidget>

#include <src/track.h>

class VideoWidget Q_DECL_FINAL: public QOpenGLWidget
{
    Q_OBJECT
public:
    VideoWidget(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
    ~VideoWidget() override;

    bool isPlaying();
    float position();
    QList<Track> *getTracks();

    void play();
    void pause();
    void setPosition(float newPosition);
    void enableTrack(Track track);
    void disableSubtitlesTrack();
    void openUrl(QString urlString);

signals:
    void playbackChanged(bool isPlaying);
    void videoSizeChanged(QSize size);
    void positionChanged(float position);
protected:
    void initializeGL() override;
    void paintGL() override;
private slots:
    void swapped();
    void on_mpv_events();
    void maybeUpdate();
private:
    void handle_mpv_event(mpv_event *event);
    static void on_update(void *ctx);

    mpv::qt::Handle mpv;
    mpv_opengl_cb_context *mpv_gl;
};

#endif // VIDEOWIDGET_H
