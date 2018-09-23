#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QSlider>

#include <src/videowidget.h>

class PlayerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerWidget(QWidget *parent = nullptr);

    void openUrl(QString urlString);
    bool controlsVisible();
    void setControlsVisible(bool visible);
    void reset();

signals:
    void wantsToChangeSizeTo(QSize size);

private slots:
    void videoSizeChanged(QSize size);
    void playbackChanged(bool isPlaying);
    void progressChanged(float progress);

    void togglePlayback();
    void sliderPressed();
    void sliderReleased();
    void audioTrackButtonPressed();
    void subtitlesTrackButtonPressed();

    void audioTrackSelected(QAction *action);
    void subtitlesTrackSelected(QAction *action);
    void audioTrackMenuWillBeHidden();
    void subtitlesTrackMenuWillBeHidden();

private:
    VideoWidget *videoWidget;

    QWidget *controlsWidget;
    QPushButton *playbackButton;
    QPushButton *audioTrackButton;
    QPushButton *subtitlesTrackButton;
    QSlider *progressSlider;

    bool ignoreProgressChange;
};

#endif // PLAYERWIDGET_H
