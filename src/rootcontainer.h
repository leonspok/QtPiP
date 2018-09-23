#ifndef ROOTCONTAINER_H
#define ROOTCONTAINER_H

#include <QWidget>

class RootContainer : public QWidget
{
    Q_OBJECT
public:
    explicit RootContainer(QWidget *parent = nullptr);

signals:
    void draggedInDirection(QPoint direction);
    void draggingFinished();
    void onMouseHover();
    void onMouseLeave();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    bool isDragging;
    QPoint previousPosition;
};

#endif // ROOTCONTAINER_H
