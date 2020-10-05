#ifndef SCREENSHOTS_H
#define SCREENSHOTS_H

#include <QWidget>
#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QFont>
#include <QClipboard>
#include <QWheelEvent>
#include <QGraphicsSceneWheelEvent>

#include "remotepixmaplabel.h"
#include "widgets/waitingspinnerwidget.h"

namespace Ui {
class Screenshots;
}

class Screenshots : public QWidget
{
    Q_OBJECT

public:
    explicit Screenshots(QWidget *parent = nullptr);
    ~Screenshots();

signals:
    void next();
    void prev();
public slots:
    void setItem(QString urlStr);
    void navigationUpdate(bool left, bool right);
    void showIstallCommand(QString command);
protected slots:
    bool eventFilter(QObject *obj, QEvent *ev);
private slots:
    void on_close_clicked();

    void on_copyCommand_clicked();

    void Zoom(QGraphicsSceneWheelEvent *event);
private:
    Ui::Screenshots *ui;
    WaitingSpinnerWidget *wall_loader = nullptr;
    QPixmap _currentPixmap;
    bool _pixmapLoaded = false;
    bool _screenshot = true;
    QClipboard *clipboard = nullptr;
    bool isReady = false;
    QGraphicsOpacityEffect *eff = nullptr;


};

#endif // SCREENSHOTS_H
