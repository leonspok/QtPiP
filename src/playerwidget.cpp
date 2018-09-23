#include "playerwidget.h"
#include <math.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QMenu>

#include <src/constants.h>
#include <src/track.h>

PlayerWidget::PlayerWidget(QWidget *parent) : QWidget(parent)
{
    this->videoWidget = new VideoWidget(this);
    connect(this->videoWidget, &VideoWidget::videoSizeChanged, this, &PlayerWidget::videoSizeChanged);
    connect(this->videoWidget, &VideoWidget::playbackChanged, this, &PlayerWidget::playbackChanged);
    connect(this->videoWidget, &VideoWidget::positionChanged, this, &PlayerWidget::progressChanged);

    this->controlsWidget = new QWidget(this);
    this->controlsWidget->setAttribute(Qt::WA_NoSystemBackground);

    this->playbackButton = new QPushButton(this->controlsWidget);
    this->playbackButton->setIcon(QIcon(":play"));
    connect(this->playbackButton, &QPushButton::clicked, this, &PlayerWidget::togglePlayback);

    this->audioTrackButton = new QPushButton(this->controlsWidget);
    this->audioTrackButton->setIcon(QIcon(":audio"));
    connect(this->audioTrackButton, &QPushButton::clicked, this, &PlayerWidget::audioTrackButtonPressed);

    this->subtitlesTrackButton = new QPushButton(this->controlsWidget);
    this->subtitlesTrackButton->setIcon(QIcon(":subtitles"));
    connect(this->subtitlesTrackButton, &QPushButton::clicked, this, &PlayerWidget::subtitlesTrackButtonPressed);

    this->progressSlider = new QSlider(Qt::Horizontal, this->controlsWidget);
    this->progressSlider->setMaximum(1000000);
    this->progressSlider->setMinimum(0);
    connect(this->progressSlider, &QSlider::sliderReleased, this, &PlayerWidget::sliderReleased);
    connect(this->progressSlider, &QSlider::sliderPressed, this, &PlayerWidget::sliderPressed);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setContentsMargins(BOUNDS_OFFSET, 0, BOUNDS_OFFSET, 0);
    controlsLayout->addWidget(this->playbackButton, 0, Qt::AlignBottom | Qt::AlignLeft);
    controlsLayout->addWidget(this->audioTrackButton, 0, Qt::AlignBottom | Qt::AlignLeft);
    controlsLayout->addWidget(this->subtitlesTrackButton, 0, Qt::AlignBottom | Qt::AlignLeft);
    controlsLayout->addWidget(this->progressSlider, 1);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, BOUNDS_OFFSET);
    vLayout->addStretch();
    vLayout->addLayout(controlsLayout);
    this->controlsWidget->setLayout(vLayout);

    QStackedLayout *layout = new QStackedLayout();
    layout->addWidget(this->controlsWidget);
    layout->addWidget(this->videoWidget);
    layout->setStackingMode(QStackedLayout::StackAll);
    this->setLayout(layout);

    this->ignoreProgressChange = false;
    this->setControlsVisible(false);
}

bool PlayerWidget::controlsVisible() {
    return this->controlsWidget->isVisible();
}

void PlayerWidget::setControlsVisible(bool visible) {
    this->controlsWidget->setVisible(visible);
}

void PlayerWidget::openUrl(QString urlString) {
    this->videoWidget->openUrl(urlString);
    this->videoWidget->play();
}

void PlayerWidget::reset() {
    this->videoWidget->pause();
}

void PlayerWidget::videoSizeChanged(QSize size) {
    emit this->wantsToChangeSizeTo(size);
}

void PlayerWidget::playbackChanged(bool isPlaying) {
    if (isPlaying) {
        this->playbackButton->setIcon(QIcon(":pause"));
    } else {
        this->playbackButton->setIcon(QIcon(":play"));
    }
}

void PlayerWidget::progressChanged(float progress) {
    if (this->ignoreProgressChange) {
        return;
    }
    this->progressSlider->setValue(static_cast<int>(round(progress * static_cast<float>(this->progressSlider->maximum()))));
}

void PlayerWidget::togglePlayback() {
    if (this->videoWidget->isPlaying()) {
        this->videoWidget->pause();
    } else {
        this->videoWidget->play();
    }
}

void PlayerWidget::sliderPressed() {
    this->ignoreProgressChange = true;
}

void PlayerWidget::sliderReleased() {
    this->ignoreProgressChange = false;
    float progress = static_cast<float>(this->progressSlider->value()) / static_cast<float>(this->progressSlider->maximum());
    this->videoWidget->setPosition(progress);
}

void PlayerWidget::audioTrackButtonPressed() {
    QMenu *menu = new QMenu(this->audioTrackButton);
    connect(menu, &QMenu::triggered, this, &PlayerWidget::audioTrackSelected);
    connect(menu, &QMenu::aboutToHide, this, &PlayerWidget::audioTrackMenuWillBeHidden);

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

    if (menu->actions().size() > 0) {
        this->audioTrackButton->setMenu(menu);
        this->audioTrackButton->showMenu();
    } else {
        delete menu;
    }
}

void PlayerWidget::subtitlesTrackButtonPressed() {
    QMenu *menu = new QMenu(this->subtitlesTrackButton);
    connect(menu, &QMenu::triggered, this, &PlayerWidget::subtitlesTrackSelected);
    connect(menu, &QMenu::aboutToHide, this, &PlayerWidget::subtitlesTrackMenuWillBeHidden);

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

    if (menu->actions().size() > 0) {
        this->subtitlesTrackButton->setMenu(menu);
        this->subtitlesTrackButton->showMenu();
    } else {
        delete menu;
    }
}

void PlayerWidget::audioTrackSelected(QAction *action) {
    if (action->isChecked()) {
        Track track;
        track.id = action->data().toInt();
        track.type = TrackTypeAudio;
        this->videoWidget->enableTrack(track);
    }
    this->audioTrackButton->setMenu(nullptr);
}

void PlayerWidget::audioTrackMenuWillBeHidden() {
    this->audioTrackButton->setMenu(nullptr);
}

void PlayerWidget::subtitlesTrackSelected(QAction *action) {
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

void PlayerWidget::subtitlesTrackMenuWillBeHidden() {
    this->subtitlesTrackButton->setMenu(nullptr);
}
