#include "windowdragger.h"
#include "iostream"

#include <QMouseEvent>
#include <QCursor>
#include <QStyleOption>
#include <QPainter>

WindowDragger::WindowDragger(QWidget *parent) : QWidget(parent)
{
    this->setMouseTracking(true);
}

void WindowDragger::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (event->button() != Qt::LeftButton) {
        return;
    }
    this->isDragging = true;
    this->previousPosition = QCursor::pos();
}

void WindowDragger::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    if (event->button() != Qt::LeftButton) {
        return;
    }
    this->isDragging = false;
    emit this->draggingFinished();
}

void WindowDragger::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    if (!this->isDragging) {
        return;
    }
    QPoint newPosition = QCursor::pos();
    int dx = newPosition.x() - this->previousPosition.x();
    int dy = newPosition.y() - this->previousPosition.y();
    this->previousPosition = newPosition;

    emit this->draggedInDirection(QPoint(dx, dy));
}

void WindowDragger::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    this->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void WindowDragger::enterEvent(QEvent *event) {
    QWidget::enterEvent(event);
    emit this->onMouseHover();
}

void WindowDragger::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    emit this->onMouseLeave();
}
