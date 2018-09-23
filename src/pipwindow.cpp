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

#include <src/constants.h>
#include <src/mutedoverlaywidget.h>
#include <src/videowidget.h>
#include <src/finishablesizegrip.h>
#include <src/rootcontainer.h>

PiPWindow::PiPWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint|
                         Qt::X11BypassWindowManagerHint|
                         Qt::FramelessWindowHint|
                         Qt::CustomizeWindowHint);

    RootContainer *rootContainer = new RootContainer(this);
    connect(rootContainer, &RootContainer::draggedInDirection, this, &PiPWindow::windowDraggedInDirection);
    connect(rootContainer, &RootContainer::draggingFinished, this, &PiPWindow::windowDraggingFinished);
    connect(rootContainer, &RootContainer::onMouseHover, this, &PiPWindow::onWindowHover);
    connect(rootContainer, &RootContainer::onMouseLeave, this, &PiPWindow::onWindowLeave);

    this->rootContainer = rootContainer;
    this->setCentralWidget(this->rootContainer);

    this->playerWidget = new PlayerWidget(this->rootContainer);
    this->playerWidget->hide();
    connect(this->playerWidget, &PlayerWidget::wantsToChangeSizeTo, this, &PiPWindow::videoSizeChanged);

    this->webContentWidget = new WebContentWidget(this->rootContainer);
    this->webContentWidget->hide();

    this->mutedOverlayWidget = new MutedOverlayWidget(this->rootContainer);
    this->mutedOverlayWidget->setMuted(false);

    this->exitButton = new QPushButton(this->rootContainer);
    this->exitButton->setIcon(QIcon(":exit"));
    this->exitButton->setFixedSize(22, 22);
    this->exitButton->setVisible(false);
    connect(this->exitButton, &QPushButton::clicked, this, &PiPWindow::exitPiP);

    this->draggingView = new DraggingButton(this->rootContainer);
    this->draggingView->setFixedSize(22, 22);
    this->draggingView->setVisible(false);
    connect(this->draggingView, &DraggingButton::draggedInDirection, this, &PiPWindow::windowDraggedInDirection);
    connect(this->draggingView, &DraggingButton::draggingFinished, this, &PiPWindow::windowDraggingFinished);
    connect(this->draggingView, &DraggingButton::doubleClicked, this, &PiPWindow::hideWindow);

    this->bottomRightSizeGrip = new FinishableSizeGrip(this->rootContainer);
    connect(this->bottomRightSizeGrip, &FinishableSizeGrip::resizeFinished, this, &PiPWindow::windowDraggingFinished);

    this->aspectRatio = 16.0f / 9;
    this->setMinimumSize(150, 150);
    this->setVisuallyMuted(false);
}

PiPWindow::~PiPWindow()
{

}

PiPWindowMode PiPWindow::mode() {
    return this->currentMode;
}

void PiPWindow::openUrl(QString urlString, PiPWindowMode mode) {
    this->currentMode = mode;

    this->playerWidget->hide();
    this->playerWidget->reset();
    this->webContentWidget->hide();
    this->webContentWidget->reset();

    switch (mode) {
        case PiPWindowModePlayer: {
            QStackedLayout *layout = new QStackedLayout();
            layout->addWidget(this->mutedOverlayWidget);
            layout->addWidget(this->playerWidget);
            layout->setStackingMode(QStackedLayout::StackAll);
            this->rootContainer->setLayout(layout);
            this->playerWidget->openUrl(urlString);
            this->playerWidget->show();
            break;
        }
        case PiPWindowModeWeb: {
            QStackedLayout *layout = new QStackedLayout();
            layout->addWidget(this->mutedOverlayWidget);
            layout->addWidget(this->webContentWidget);
            layout->setStackingMode(QStackedLayout::StackAll);
            this->rootContainer->setLayout(layout);
            this->webContentWidget->openUrl(urlString);
            this->webContentWidget->show();
            break;
        }
    }
}

void PiPWindow::moveToDefaultPosition() {
    QSize screenSize = this->getCurrentScreen()->size();
    this->resize(480, 320);
    this->move(screenSize.width() - BOUNDS_OFFSET - this->size().width(),
               screenSize.height() - BOUNDS_OFFSET - this->size().height());
    this->windowDraggingFinished();
}

void PiPWindow::videoSizeChanged(QSize size) {
    int width = size.width();
    int height = size.height();

    float ratio = static_cast<float>(width) / height;
    this->aspectRatio = ratio;

    QRect screenRect = this->getCurrentScreen()->geometry();

    int screenWidth = screenRect.width() - BOUNDS_OFFSET * 2;
    int screenHeight = screenRect.height() - BOUNDS_OFFSET * 2;

    int targetWidth, targetHeight;

    if (ratio >= 1) {
        targetWidth = std::min(screenWidth / 3, width);
        targetHeight = static_cast<int>(std::floor((static_cast<float>(targetWidth) / ratio)));
    } else {
        targetHeight = std::min(screenHeight / 3, height);
        targetWidth = static_cast<int>(std::floor((static_cast<float>(targetHeight) / ratio)));
    }

    this->resize(targetWidth, targetHeight);
    this->moveToCorner(ForceVisible);
}

void PiPWindow::windowDraggedInDirection(QPoint direction) {
    int newX = this->pos().x() + direction.x();
    int newY = this->pos().y() + direction.y();
    this->move(newX, newY);
}

void PiPWindow::windowDraggingFinished() {
    this->moveToCorner();
}

void PiPWindow::onWindowHover() {
    if (this->playerWidget != nullptr) {
        this->playerWidget->setControlsVisible(true);
    }
    if (this->mode() == PiPWindowModeWeb && !this->visuallyMuted) {
        this->activateWindow();
    }
    this->exitButton->setVisible(true);
    this->draggingView->setVisible(this->mode() == PiPWindowModeWeb && !this->visuallyMuted);
}

void PiPWindow::onWindowLeave() {
    if (this->playerWidget != nullptr) {
        this->playerWidget->setControlsVisible(false);
    }
    this->exitButton->setVisible(false);
    this->draggingView->setVisible(false);
}

void PiPWindow::hideWindow() {
    this->moveToCorner(ForceHidden);
}

void PiPWindow::exitPiP() {
    QApplication::exit(0);
}

void PiPWindow::moveToCorner(PiPPositionMode positionMode) {
    QPoint windowCenter = this->geometry().center();
    QSize windowSize = this->geometry().size();
    QPoint screenCenter = this->getCurrentScreen()->geometry().center();
    QSize screenSize = this->getCurrentScreen()->geometry().size();
    QPoint newPosition;
    if (windowCenter.x() <= screenCenter.x() && windowCenter.y() <= screenCenter.y()) {
        int x = BOUNDS_OFFSET;
        if ((windowCenter.x() < 0 && positionMode != ForceVisible) || positionMode == ForceHidden) {
            x -= windowSize.width();
        }
        newPosition = QPoint(x, BOUNDS_OFFSET);
    } else if (windowCenter.x() <= screenCenter.x() && windowCenter.y() > screenCenter.y()) {
        int x = BOUNDS_OFFSET;
        if ((windowCenter.x() < 0 && positionMode != ForceVisible) || positionMode == ForceHidden) {
            x -= windowSize.width();
        }
        newPosition = QPoint(x, screenSize.height() - windowSize.height() - BOUNDS_OFFSET);
    } else if (windowCenter.x() > screenCenter.x() && windowCenter.y() <= screenCenter.y()) {
        int x = screenSize.width() - windowSize.width() - BOUNDS_OFFSET;
        if ((windowCenter.x() > screenSize.width() && positionMode != ForceVisible) || positionMode == ForceHidden) {
            x += windowSize.width();
        }
        newPosition = QPoint(x, BOUNDS_OFFSET);
    } else {
        int x = screenSize.width() - windowSize.width() - BOUNDS_OFFSET;
        if ((windowCenter.x() > screenSize.width() && positionMode != ForceVisible) || positionMode == ForceHidden) {
            x += windowSize.width();
        }
        newPosition = QPoint(x, screenSize.height() - windowSize.height() - BOUNDS_OFFSET);
    }

    bool xPositionOutOfBounds = windowCenter.x() < 0 || windowCenter.x() > screenSize.width();
    this->setVisuallyMuted((xPositionOutOfBounds && positionMode != ForceVisible) || positionMode == ForceHidden);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    animation->setDuration(100);
    animation->setStartValue(this->pos());
    animation->setEndValue(newPosition);
    animation->start();
}

void PiPWindow::resizeEvent(QResizeEvent *event) {
    float maxRatio = this->mode() == PiPWindowModePlayer ? 0.5f : 0.7f;
    int maxWidth = static_cast<int>(floor((this->getCurrentScreen()->size().width() - BOUNDS_OFFSET * 2) * maxRatio));
    int maxHeight = static_cast<int>(floor((this->getCurrentScreen()->size().height() - BOUNDS_OFFSET * 2) * maxRatio));
    int width = std::min(maxWidth, event->size().width());
    int height = 0;
    if (this->mode() == PiPWindowModePlayer) {
        height = static_cast<int>(floor(width / this->aspectRatio));
    } else {
        height = std::min(maxHeight, event->size().height());
    }
    QSize finalSize = QSize(width, height);
    this->resize(finalSize);

    this->exitButton->move(width - BOUNDS_OFFSET - this->exitButton->size().width(), BOUNDS_OFFSET);
    this->draggingView->move(this->exitButton->pos().x() - BOUNDS_OFFSET / 2 - this->draggingView->size().width(), BOUNDS_OFFSET);
    this->bottomRightSizeGrip->move(width - this->bottomRightSizeGrip->size().width(),
                                    finalSize.height() - this->bottomRightSizeGrip->size().height());
}

QScreen *PiPWindow::getCurrentScreen() {
    int screenNumber = QApplication::desktop()->screenNumber(this);
    return QGuiApplication::screens().at(screenNumber);
}

void PiPWindow::setVisuallyMuted(bool muted) {
    this->visuallyMuted = muted;
    this->mutedOverlayWidget->setMuted(muted);
    this->webContentWidget->setAttribute(Qt::WA_TransparentForMouseEvents, muted);
    if (this->mode() == PiPWindowModeWeb && !muted) {
        this->onWindowHover();
    }
}
