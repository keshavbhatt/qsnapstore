#include "graphicsview.h"

graphicsView::graphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    //setDragMode(QGraphicsView::ScrollHandDrag);
    //Ctrl+Plus and Ctrl+Minus will work, other keyboard with standard keys will work.
    m_pZoomInSc = new QShortcut( QKeySequence::ZoomIn, this, SLOT( shortCutZoomIn() ) );
    m_pZoomOutSc = new QShortcut( QKeySequence::ZoomOut, this, SLOT( shortCutZoomOut() ) );
}


//Shortcut Zoom In
void graphicsView::shortCutZoomIn()
{
    //zoom
    setTransformationAnchor( QGraphicsView::AnchorUnderMouse) ;
    // Scale the view / do the zoom
    double scaleFactor = 1.5;
    // Zoom in
    scale( scaleFactor, scaleFactor );
}

//Shortcut Zoom Out
void graphicsView::shortCutZoomOut()
{
    //zoom
    setTransformationAnchor( QGraphicsView::AnchorUnderMouse );
    // Scale the view / do the zoom
    double scaleFactor = 1.5;
    // Zoom in
    scale( 1.0 / scaleFactor, 1.0 / scaleFactor );
}


//The mousewheel event
void graphicsView::wheelEvent(QWheelEvent *event)
{
    //zoom
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    // Scale the view / do the zoom
    double scaleFactor = 1.05;
    if(event->angleDelta().y() > 0)
    {
        // Zoom in
        scale(scaleFactor, scaleFactor);
    }
    else if(event->angleDelta().y() < 0)
    {
        // Zooming out
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}

//Reset the zoom to exact 100%
void graphicsView::resetZoom()
{
    //resetTransform(); //->kills anchor
    //calculating this does not kill anchor -> latest anchor will be used
    qreal percentZoom = 100.0;
    qreal targetScale = (qreal)percentZoom / 100.0;
    qreal scaleFactor = targetScale / transform().m11();
    scale( scaleFactor, scaleFactor );
}


/*
 * Rotate all selected items in the scene
 */
void graphicsView::rotateSelectedItems()
{
     foreach(QGraphicsItem* item, this->scene()->selectedItems()) {
       rotateItem(item);
     }
}

/*
 * Return true if scene in view is empty
 */
bool graphicsView::isSceneEmpty()
{
    return this->scene()->items().isEmpty();
}

/*
 *  Rotate selected item by given angle.
 *  This function will try to rotate first item if given item is null
 */
void graphicsView::rotateItem(QGraphicsItem* item, qreal angle)
{
    if(isSceneEmpty())
        return;

    if(item == nullptr){
       item = this->scene()->items().first();
    }

    QRectF rect = item->mapToScene(item->boundingRect()).boundingRect();
    QPointF center = rect.center();

    QTransform t;
    t.translate(center.x(), center.y());
    t.rotate(angle);
    t.translate(-center.x(), -center.y());
    item->setPos(t.map(item->pos()));
    item->setRotation(item->rotation() + angle);
}


/*
 * Return the roation angle of selected item else first item
*/
int graphicsView::rotation(QGraphicsItem* item)
{
    if(isSceneEmpty())
        return 0;

    if(item == nullptr){
       item = this->scene()->items().first();
    }

    return item->rotation();
}



