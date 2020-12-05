#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QObject>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QEvent>
#include <QWheelEvent>
#include <QGraphicsSceneWheelEvent>
#include <QImageReader>
#include <QShortcut>

class graphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    graphicsView(QWidget *parent = nullptr);

public slots:
    void rotateSelectedItems();
    void rotateItem(QGraphicsItem *item = nullptr, qreal angle = 90);
    int  rotation(QGraphicsItem *item = nullptr);
    void resetZoom();
protected slots:
    void wheelEvent(QWheelEvent *event);
private slots:
    bool isSceneEmpty();
    void shortCutZoomOut();
    void shortCutZoomIn();
private:
    QShortcut *m_pZoomInSc;
    QShortcut *m_pZoomOutSc;

};

#endif // GRAPHICSVIEW_H
