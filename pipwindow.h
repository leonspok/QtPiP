#ifndef PIPWINDOW_H
#define PIPWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <mpv/client.h>

#include <videowidget.h>
#include <windowdragger.h>

class PiPWindow : public QMainWindow
{
    Q_OBJECT

public:
    PiPWindow(QWidget *parent = nullptr);
    ~PiPWindow() override;

    void openURL(QString urlString);
    void moveToDefaultPosition();
    void setVisuallyMuted(bool muted);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void videoSizeChanged(QSize size);
    void playbackChanged(bool isPlaying);
    void progressChanged(float progress);

    void windowDraggedInDirection(QPoint direction);
    void windowDraggingFinished();
    void onWindowHover();
    void onWindowLeave();

    void togglePlayback();
    void exitPiP();
    void sliderPressed();
    void sliderReleased();

private:
    QWidget *rootContainer;
    VideoWidget *videoWidget;
    WindowDragger *windowDragger;
    QPushButton *playbackButton;
    QPushButton *exitButton;
    QSlider *progressSlider;
    QScreen *getCurrentScreen();
    float aspectRatio;
    bool visuallyMuted;
    bool ignoreProgressChange;

    void moveToCorner(bool forceVisible);
};

#endif // PIPWINDOW_H
