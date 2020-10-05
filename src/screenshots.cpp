#include "screenshots.h"
#include "ui_screenshots.h"
#include <QFont>
#include <QClipboard>

#include <QWheelEvent>
#include <QGraphicsSceneWheelEvent>

Screenshots::Screenshots(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Screenshots)
{
    ui->setupUi(this);

    QIcon left;
    left.addPixmap(QPixmap(":/icons/arrow-left-circle-line.png"), QIcon::Normal);
    left.addPixmap(QPixmap(":/icons/disabled-arrow-left-circle-line.png"), QIcon::Disabled);
    ui->previous->setIcon(left);

    QIcon right;
    right.addPixmap(QPixmap(":/icons/arrow-right-circle-line.png"), QIcon::Normal);
    right.addPixmap(QPixmap(":/icons/disabled-arrow-right-circle-line.png"), QIcon::Disabled);
    ui->next->setIcon(right);

    ui->graphicsView->setStyleSheet("border:none");
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    connect(ui->next,&QPushButton::clicked,[=](){
        _pixmapLoaded = false;
        emit next();
    });
    connect(ui->previous,&QPushButton::clicked,[=](){
        _pixmapLoaded = false;
        emit prev();
    });

    QGraphicsScene* scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    ui->copyCommand->hide();

    wall_loader = new WaitingSpinnerWidget(this,true,false);
    wall_loader->setRoundness(70.0);
    wall_loader->setMinimumTrailOpacity(15.0);
    wall_loader->setTrailFadePercentage(70.0);
    wall_loader->setNumberOfLines(40);
    wall_loader->setLineLength(20);
    wall_loader->setLineWidth(2);
    wall_loader->setInnerRadius(2);
    wall_loader->setRevolutionsPerSecond(3);
    wall_loader->setColor(QColor("#1e90ff"));

    isReady = true;
    ui->graphicsView->scene()->installEventFilter(this);

}

void Screenshots::showIstallCommand(QString command)
{
    wall_loader->start();
    ui->graphicsView->scene()->clear();
    ui->graphicsView->resetMatrix();
    QFont f;
    f.setFamily("FreeMono");
    f.setStyleHint(QFont::Monospace);
    f.setPixelSize(22);
    QGraphicsTextItem* ti = new QGraphicsTextItem(command);
    ti->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ti->setFlags(QGraphicsItem::ItemIsSelectable| QGraphicsItem::ItemIsMovable | ti->flags() );
    ti->setFont(f);

    ui->graphicsView->scene()->setSceneRect(ti->boundingRect());
    ui->graphicsView->scene()->addItem(ti);
    wall_loader->stop();
    ui->copyCommand->show();
}

void Screenshots::setItem(QString urlStr)
{
    ui->copyCommand->hide();
    if(_pixmapLoaded){
        wall_loader->start();
        ui->graphicsView->scene()->clear();
        QGraphicsPixmapItem* item = new QGraphicsPixmapItem();
        item->setFlags(QGraphicsItem::ItemIsSelectable| QGraphicsItem::ItemIsMovable | item->flags() );
        item->setTransformationMode(Qt::SmoothTransformation);
        item->setPixmap(_currentPixmap);
        ui->graphicsView->scene()->setSceneRect(_currentPixmap.rect());
        ui->graphicsView->scene()->addItem(item);
        ui->graphicsView->fitInView(item,Qt::KeepAspectRatio);
        wall_loader->stop();
    }else{
        wall_loader->start();
        RemotePixmapLabel *sc = new RemotePixmapLabel(this);
        sc->setRemotePixmap(urlStr);
        connect(sc,&RemotePixmapLabel::pixmapLoaded,[=](QByteArray data){
            ui->graphicsView->scene()->clear();
            QGraphicsPixmapItem* item = new QGraphicsPixmapItem();
            item->setFlags(QGraphicsItem::ItemIsSelectable| QGraphicsItem::ItemIsMovable | item->flags() );
            item->setTransformationMode(Qt::SmoothTransformation);
            _currentPixmap.loadFromData(data);
            item->setPixmap(_currentPixmap);
            ui->graphicsView->scene()->setSceneRect(_currentPixmap.rect());
            ui->graphicsView->scene()->addItem(item);
            ui->graphicsView->fitInView(item,Qt::KeepAspectRatio);
            _pixmapLoaded = true;
            wall_loader->stop();
            sc->deleteLater();
        });
        connect(sc,&RemotePixmapLabel::pixmapLoading,[=](){
           wall_loader->start();
        });
    }
}

void Screenshots::navigationUpdate(bool left, bool right)
{
    ui->next->setEnabled(right);
    ui->previous->setEnabled(left);
}

Screenshots::~Screenshots()
{
    delete ui;
}

void Screenshots::on_close_clicked()
{
    _pixmapLoaded = false;
    ui->graphicsView->resetMatrix();
    ui->graphicsView->scene()->clear();
    ui->copyCommand->hide();
    ui->copyCommand->setEnabled(true);
    wall_loader->stop();
    this->hide();
}

void Screenshots::on_copyCommand_clicked()
{
    if(!ui->graphicsView->scene()->items().isEmpty()){
        ui->copyCommand->setEnabled(true);
        QGraphicsItem *item = ui->graphicsView->scene()->items().first();
        QGraphicsTextItem *commandItem = static_cast<QGraphicsTextItem*>(item);
        if(clipboard==nullptr)
            clipboard = QApplication::clipboard();
        clipboard->setText(commandItem->toPlainText().trimmed().simplified());
    }else{
        ui->copyCommand->setEnabled(false);
    }
}

bool Screenshots::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::GraphicsSceneWheel)
        {
            Zoom(static_cast<QGraphicsSceneWheelEvent*> (ev));
        }
    return QWidget::eventFilter(obj,ev);
}

void Screenshots::Zoom(QGraphicsSceneWheelEvent *event)
{
    qreal scaleFactor=pow((double)2, event->delta() / 240.0);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->scale(scaleFactor, scaleFactor);
}
