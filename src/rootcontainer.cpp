#include "rootcontainer.h"

#include <QMouseEvent>
#include <QPaintEvent>

RootContainer::RootContainer(QWidget *parent) : QWidget(parent)
{
    this->installEventFilter(this);
}

bool RootContainer::eventFilter(QObject *obj, QEvent *event) {
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton) {
            break;
        }
        this->isDragging = true;
        this->previousPosition = QCursor::pos();
        break;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton) {
            break;
        }
        this->isDragging = false;
        emit this->draggingFinished();
        break;
    }
    case QEvent::MouseMove: {
        if (!this->isDragging) {
            break;
        }
        QPoint newPosition = QCursor::pos();
        int dx = newPosition.x() - this->previousPosition.x();
        int dy = newPosition.y() - this->previousPosition.y();
        this->previousPosition = newPosition;

        emit this->draggedInDirection(QPoint(dx, dy));
        break;
    }
    case QEvent::Enter:
        emit this->onMouseHover();
        break;
    case QEvent::Leave:
        emit this->onMouseLeave();
        break;
    default:
        break;
    }
    return QWidget::eventFilter(obj, event);
}
