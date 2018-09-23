#ifndef DRAGGINGWIDGET_H
#define DRAGGINGWIDGET_H

#include <QPushButton>
#include <QGraphicsView>

class DraggingButton : public QPushButton
{
    Q_OBJECT
public:
    explicit DraggingButton(QWidget *parent = nullptr);

signals:
    void draggedInDirection(QPoint direction);
    void draggingFinished();
    void doubleClicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    bool isDragging;
    QPoint previousPosition;
};

#endif // DRAGGINGWIDGET_H
