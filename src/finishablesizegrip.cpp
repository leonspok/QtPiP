#include "finishablesizegrip.h"

FinishableSizeGrip::FinishableSizeGrip(QWidget *parent) : QSizeGrip(parent)
{
    this->setMouseTracking(true);
}

void FinishableSizeGrip::mousePressEvent(QMouseEvent *event) {
    QSizeGrip::mousePressEvent(event);
    emit this->resizeStarted();
}

void FinishableSizeGrip::mouseReleaseEvent(QMouseEvent *event) {
    QSizeGrip::mousePressEvent(event);
    emit this->resizeFinished();
}
