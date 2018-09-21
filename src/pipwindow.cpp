#include "pipwindow.h"
#include "iostream"
#include <algorithm>
#include <math.h>
#include <mpv/qthelper.hpp>

#include <QGuiApplication>
#include <QApplication>
#include <QScreen>
#include <QCursor>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsAnchorLayout>
#include <QStackedLayout>
#include <QSizeGrip>
#include <QPropertyAnimation>
#include <QGraphicsBlurEffect>
#include <QMenu>

#include <src/windowdragger.h>
#include <src/videowidget.h>
#include <src/finishablesizegrip.h>

#define OFFSET 20

PiPWindow::PiPWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint|
                         Qt::X11BypassWindowManagerHint|
                         Qt::FramelessWindowHint|
                         Qt::CustomizeWindowHint);

    this->rootContainer = new QWidget(this);
    this->setCentralWidget(this->rootContainer);

    this->videoWidget = new VideoWidget(this->rootContainer);
    connect(this->videoWidget, &VideoWidget::videoSizeChanged, this, &PiPWindow::videoSizeChanged);
    connect(this->videoWidget, &VideoWidget::playbackChanged, this, &PiPWindow::playbackChanged);
    connect(this->videoWidget, &VideoWidget::positionChanged, this, &PiPWindow::progressChanged);

    this->windowDragger = new WindowDragger(this->rootContainer);
    this->windowDragger->setAttribute(Qt::WA_NoSystemBackground, true);
    connect(this->windowDragger, &WindowDragger::draggedInDirection, this, &PiPWindow::windowDraggedInDirection);
    connect(this->windowDragger, &WindowDragger::draggingFinished, this, &PiPWindow::windowDraggingFinished);
    connect(this->windowDragger, &WindowDragger::onMouseHover, this, &PiPWindow::onWindowHover);
    connect(this->windowDragger, &WindowDragger::onMouseLeave, this, &PiPWindow::onWindowLeave);

    QStackedLayout *layout = new QStackedLayout();
    layout->addWidget(this->windowDragger);
    layout->addWidget(this->videoWidget);
    layout->setStackingMode(QStackedLayout::StackAll);
    this->rootContainer->setLayout(layout);

    this->playbackButton = new QPushButton(this->windowDragger);
    this->playbackButton->setIcon(QIcon(":play"));
    this->playbackButton->setVisible(false);
    connect(this->playbackButton, &QPushButton::pressed, this, &PiPWindow::togglePlayback);

    this->exitButton = new QPushButton(this->windowDragger);
    this->exitButton->setIcon(QIcon(":exit"));
    this->exitButton->setVisible(false);
    connect(this->exitButton, &QPushButton::pressed, this, &PiPWindow::exitPiP);

    this->audioTrackButton = new QPushButton(this->windowDragger);
    this->audioTrackButton->setIcon(QIcon(":audio"));
    this->audioTrackButton->setVisible(false);
    connect(this->audioTrackButton, &QPushButton::pressed, this, &PiPWindow::audioTrackButtonPressed);

    this->subtitlesTrackButton = new QPushButton(this->windowDragger);
    this->subtitlesTrackButton->setIcon(QIcon(":subtitles"));
    this->subtitlesTrackButton->setVisible(false);
    connect(this->subtitlesTrackButton, &QPushButton::pressed, this, &PiPWindow::subtitlesTrackButtonPressed);

    this->progressSlider = new QSlider(Qt::Horizontal, this->windowDragger);
    this->progressSlider->setMaximum(1000000);
    this->progressSlider->setMinimum(0);
    this->progressSlider->setVisible(false);
    connect(this->progressSlider, &QSlider::sliderReleased, this, &PiPWindow::sliderReleased);
    connect(this->progressSlider, &QSlider::sliderPressed, this, &PiPWindow::sliderPressed);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setContentsMargins(OFFSET, 0, OFFSET, 0);
    controlsLayout->addWidget(this->playbackButton, 0, Qt::AlignBottom | Qt::AlignLeft);
    controlsLayout->addWidget(this->audioTrackButton, 0, Qt::AlignBottom | Qt::AlignLeft);
    controlsLayout->addWidget(this->subtitlesTrackButton, 0, Qt::AlignBottom | Qt::AlignLeft);
    controlsLayout->addWidget(this->exitButton, 0, Qt::AlignBottom | Qt::AlignLeft);
    controlsLayout->addWidget(this->progressSlider, 1);

    FinishableSizeGrip *bottomRightSizeGrip = new FinishableSizeGrip(this);
    connect(bottomRightSizeGrip, &FinishableSizeGrip::resizeFinished, this, &PiPWindow::windowDraggingFinished);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addStretch();
    vLayout->addLayout(controlsLayout);
    vLayout->addWidget(bottomRightSizeGrip, 0, Qt::AlignBottom | Qt::AlignRight);
    this->windowDragger->setLayout(vLayout);

    this->aspectRatio = 16.0f / 9;
    this->ignoreProgressChange = false;
    this->setMinimumSize(160, 90);
}

PiPWindow::~PiPWindow()
{

}

void PiPWindow::openURL(QString urlString) {
    this->videoWidget->openUrl(urlString);
}

void PiPWindow::moveToDefaultPosition() {
    QSize screenSize = this->getCurrentScreen()->size();
    this->move(screenSize.width() - OFFSET - this->size().width(),
               screenSize.height() - OFFSET - this->size().height());
    this->windowDraggingFinished();
}

void PiPWindow::videoSizeChanged(QSize size) {
    int width = size.width();
    int height = size.height();

    float ratio = static_cast<float>(width) / height;
    this->aspectRatio = ratio;

    QRect screenRect = this->getCurrentScreen()->geometry();

    int screenWidth = screenRect.width() - OFFSET * 2;
    int screenHeight = screenRect.height() - OFFSET * 2;

    int targetWidth, targetHeight;

    if (ratio >= 1) {
        targetWidth = std::min(screenWidth / 3, width);
        targetHeight = static_cast<int>(std::floor((static_cast<float>(targetWidth) / ratio)));
    } else {
        targetHeight = std::min(screenHeight / 3, height);
        targetWidth = static_cast<int>(std::floor((static_cast<float>(targetHeight) / ratio)));
    }

    this->resize(targetWidth, targetHeight);
    this->moveToCorner(true);
}

void PiPWindow::playbackChanged(bool isPlaying) {
    if (isPlaying) {
        this->playbackButton->setIcon(QIcon(":pause"));
    } else {
        this->playbackButton->setIcon(QIcon(":play"));
    }
}

void PiPWindow::progressChanged(float progress) {
    if (this->ignoreProgressChange) {
        return;
    }
    this->progressSlider->setValue(static_cast<int>(round(progress * static_cast<float>(this->progressSlider->maximum()))));
}

void PiPWindow::windowDraggedInDirection(QPoint direction) {
    int newX = this->pos().x() + direction.x();
    int newY = this->pos().y() + direction.y();
    this->move(newX, newY);
}

void PiPWindow::windowDraggingFinished() {
    this->moveToCorner(false);
}

void PiPWindow::onWindowHover() {
    this->playbackButton->setVisible(true);
    this->exitButton->setVisible(true);
    this->progressSlider->setVisible(true);
    this->audioTrackButton->setVisible(true);
    this->subtitlesTrackButton->setVisible(true);
}

void PiPWindow::onWindowLeave() {
    this->playbackButton->setVisible(false);
    this->exitButton->setVisible(false);
    this->progressSlider->setVisible(false);
    this->audioTrackButton->setVisible(false);
    this->subtitlesTrackButton->setVisible(false);
}

void PiPWindow::togglePlayback() {
    if (this->videoWidget->isPlaying()) {
        this->videoWidget->pause();
    } else {
        this->videoWidget->play();
    }
}

void PiPWindow::exitPiP() {
    this->videoWidget->pause();
    QApplication::exit(0);
}

void PiPWindow::sliderPressed() {
    this->ignoreProgressChange = true;
}

void PiPWindow::sliderReleased() {
    this->ignoreProgressChange = false;
    float progress = static_cast<float>(this->progressSlider->value()) / static_cast<float>(this->progressSlider->maximum());
    this->videoWidget->setPosition(progress);
}

void PiPWindow::audioTrackButtonPressed() {
    QMenu *menu = new QMenu(this->audioTrackButton);
    connect(menu, &QMenu::triggered, this, &PiPWindow::audioTrackSelected);
    connect(menu, &QMenu::aboutToHide, this, &PiPWindow::audioTrackMenuWillBeHidden);

    QList<Track> *tracks = this->videoWidget->getTracks();
    for (int i = 0; i < tracks->size(); i++) {
        Track track = tracks->at(i);
        if (track.type == TrackTypeAudio) {
            QString trackName;
            if (track.title.isEmpty()) {
                trackName = QString();
                trackName.sprintf("Track #%d", track.id);
            } else {
                trackName = track.title;
            }
            QAction *action = new QAction(trackName, menu);
            action->setData(track.id);
            action->setCheckable(true);
            action->setChecked(track.selected);
            menu->addAction(action);
        }
    }

    this->audioTrackButton->setMenu(menu);
    this->audioTrackButton->showMenu();
}

void PiPWindow::subtitlesTrackButtonPressed() {
    QMenu *menu = new QMenu(this->subtitlesTrackButton);
    connect(menu, &QMenu::triggered, this, &PiPWindow::subtitlesTrackSelected);
    connect(menu, &QMenu::aboutToHide, this, &PiPWindow::subtitlesTrackMenuWillBeHidden);

    QList<Track> *tracks = this->videoWidget->getTracks();
    for (int i = 0; i < tracks->size(); i++) {
        Track track = tracks->at(i);
        if (track.type == TrackTypeSubtitles) {
            QString trackName;
            if (track.title.isEmpty()) {
                trackName = QString();
                trackName.sprintf("Subtitles #%d", track.id);
            } else {
                trackName = track.title;
            }
            QAction *action = new QAction(trackName, menu);
            action->setData(track.id);
            action->setCheckable(true);
            action->setChecked(track.selected);
            menu->addAction(action);
        }
    }

    this->subtitlesTrackButton->setMenu(menu);
    this->subtitlesTrackButton->showMenu();
}

void PiPWindow::audioTrackSelected(QAction *action) {
    if (action->isChecked()) {
        Track track;
        track.id = action->data().toInt();
        track.type = TrackTypeAudio;
        this->videoWidget->enableTrack(track);
    }
    this->audioTrackButton->setMenu(nullptr);
}

void PiPWindow::audioTrackMenuWillBeHidden() {
    this->audioTrackButton->setMenu(nullptr);
}

void PiPWindow::subtitlesTrackSelected(QAction *action) {
    if (action->isChecked()) {
        Track track;
        track.id = action->data().toInt();
        track.type = TrackTypeSubtitles;
        this->videoWidget->enableTrack(track);
    } else {
        this->videoWidget->disableSubtitlesTrack();
    }
    this->subtitlesTrackButton->setMenu(nullptr);
}

void PiPWindow::subtitlesTrackMenuWillBeHidden() {
    this->subtitlesTrackButton->setMenu(nullptr);
}

void PiPWindow::moveToCorner(bool forceVisible) {
    QPoint windowCenter = this->geometry().center();
    QSize windowSize = this->geometry().size();
    QPoint screenCenter = this->getCurrentScreen()->geometry().center();
    QSize screenSize = this->getCurrentScreen()->geometry().size();
    QPoint newPosition;
    if (windowCenter.x() <= screenCenter.x() && windowCenter.y() <= screenCenter.y()) {
        int x = OFFSET;
        if (windowCenter.x() < 0 && !forceVisible) {
            x -= windowSize.width();
        }
        newPosition = QPoint(x, OFFSET);
    } else if (windowCenter.x() <= screenCenter.x() && windowCenter.y() > screenCenter.y()) {
        int x = OFFSET;
        if (windowCenter.x() < 0 && !forceVisible) {
            x -= windowSize.width();
        }
        newPosition = QPoint(x, screenSize.height() - windowSize.height() - OFFSET);
    } else if (windowCenter.x() > screenCenter.x() && windowCenter.y() <= screenCenter.y()) {
        int x = screenSize.width() - windowSize.width() - OFFSET;
        if (windowCenter.x() > screenSize.width() && !forceVisible) {
            x += windowSize.width();
        }
        newPosition = QPoint(x, OFFSET);
    } else {
        int x = screenSize.width() - windowSize.width() - OFFSET;
        if (windowCenter.x() > screenSize.width() && !forceVisible) {
            x += windowSize.width();
        }
        newPosition = QPoint(x, screenSize.height() - windowSize.height() - OFFSET);
    }

    this->setVisuallyMuted((windowCenter.x() < 0 || windowCenter.x() > screenSize.width()) && !forceVisible);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    animation->setDuration(100);
    animation->setStartValue(this->pos());
    animation->setEndValue(newPosition);
    animation->start();
}

void PiPWindow::resizeEvent(QResizeEvent *event) {
    int maxWidth = static_cast<int>(floor((this->getCurrentScreen()->size().width() - OFFSET * 2) / 2));
    int width = std::min(maxWidth, event->size().width());
    QSize finalSize = QSize(width, static_cast<int>(floor(width / this->aspectRatio)));
    this->resize(finalSize);
}

QScreen *PiPWindow::getCurrentScreen() {
    int screenNumber = QApplication::desktop()->screenNumber(this);
    return QGuiApplication::screens().at(screenNumber);
}

void PiPWindow::setVisuallyMuted(bool muted) {
    if (this->visuallyMuted == muted) {
        return;
    }
    this->visuallyMuted = muted;
    if (muted) {
        this->windowDragger->setStyleSheet("WindowDragger { background-color: #404040; border: 2px solid #A0A0A0; }");
    } else {
        this->windowDragger->setStyleSheet("WindowDragger { background-color: transparent; border: none; }");
    }
}
