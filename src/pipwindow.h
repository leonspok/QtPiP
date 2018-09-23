#ifndef PIPWINDOW_H
#define PIPWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QStackedLayout>
#include <mpv/client.h>

#include <src/mutedoverlaywidget.h>
#include <src/playerwidget.h>
#include <src/webcontentwidget.h>
#include <src/finishablesizegrip.h>
#include <src/draggingbutton.h>

typedef enum {
    PiPWindowModePlayer,
    PiPWindowModeWeb
} PiPWindowMode;

typedef enum {
    DoNotForcePosition,
    ForceVisible,
    ForceHidden
} PiPPositionMode;

class PiPWindow : public QMainWindow
{
    Q_OBJECT

public:
    PiPWindow(QWidget *parent = nullptr);
    ~PiPWindow() override;

    PiPWindowMode mode();
    void openUrl(QString urlString, PiPWindowMode mode);
    void moveToDefaultPosition();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void exitPiP();
    void videoSizeChanged(QSize size);

    void windowDraggedInDirection(QPoint direction);
    void windowDraggingFinished();
    void onWindowHover();
    void onWindowLeave();
    void hideWindow();

private:
    QWidget *rootContainer;

    PlayerWidget *playerWidget;
    WebContentWidget *webContentWidget;
    MutedOverlayWidget *mutedOverlayWidget;
    DraggingButton *draggingView;

    QPushButton *exitButton;
    FinishableSizeGrip *bottomRightSizeGrip;

    PiPWindowMode currentMode;
    float aspectRatio;
    bool visuallyMuted;

    QScreen *getCurrentScreen();
    void moveToCorner(PiPPositionMode positionMode = DoNotForcePosition);
    void setVisuallyMuted(bool muted);
};

#endif // PIPWINDOW_H
