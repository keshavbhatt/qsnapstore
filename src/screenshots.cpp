#include "screenshots.h"
#include "ui_screenshots.h"

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


    connect(ui->next,&QPushButton::clicked,[=](){
        _pixmapLoaded = false;
        emit next();
    });
    connect(ui->previous,&QPushButton::clicked,[=](){
        _pixmapLoaded = false;
        emit prev();
    });

    QGraphicsScene* scene = new QGraphicsScene(ui->graphicsViewWidget);
    ui->graphicsViewWidget->setScene(scene);

    ui->graphicsViewWidget->setStyleSheet("border:none");

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
    ui->graphicsViewWidget->installEventFilter(this);

}

void Screenshots::showIstallCommand(QString command)
{
    _screenshot = false;
    ui->next->hide();
    ui->previous->hide();
    wall_loader->start();
    ui->graphicsViewWidget->scene()->clear();
    ui->graphicsViewWidget->resetMatrix();
    QFont f;
    f.setFamily("FreeMono");
    f.setStyleHint(QFont::Monospace);
    f.setPixelSize(22);
    QGraphicsTextItem* ti = new QGraphicsTextItem(command);
    ti->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ti->setFlags(QGraphicsItem::ItemIsSelectable| QGraphicsItem::ItemIsMovable | ti->flags() );
    ti->setFont(f);

    ui->graphicsViewWidget->scene()->setSceneRect(ti->boundingRect());
    ui->graphicsViewWidget->scene()->addItem(ti);
    ui->copyCommand->show();
    wall_loader->stop();
}

void Screenshots::setItem(QString urlStr)
{
    if(!_screenshot)//prevent setting screenshot if install command is visible (this happe in parent's resize event)
        return;

    ui->copyCommand->hide();
    if(_pixmapLoaded){
        ui->graphicsViewWidget->scene()->clear();
        QGraphicsPixmapItem* item = new QGraphicsPixmapItem();
        item->setFlags(QGraphicsItem::ItemIsSelectable| QGraphicsItem::ItemIsMovable | item->flags() );
        item->setTransformationMode(Qt::SmoothTransformation);
        item->setPixmap(_currentPixmap);
        ui->graphicsViewWidget->scene()->setSceneRect(_currentPixmap.rect());
        ui->graphicsViewWidget->scene()->addItem(item);
        ui->graphicsViewWidget->fitInView(item,Qt::KeepAspectRatio);
    }else{
        wall_loader->start();
        RemotePixmapLabel *sc = new RemotePixmapLabel(this);
        sc->setRemotePixmap(urlStr,"qrc:/icons/others/wall_placeholder_180.jpg");
        connect(sc,&RemotePixmapLabel::pixmapLoaded,[=](QByteArray data){
            ui->graphicsViewWidget->scene()->clear();
            QGraphicsPixmapItem* item = new QGraphicsPixmapItem();
            item->setFlags(QGraphicsItem::ItemIsSelectable| QGraphicsItem::ItemIsMovable | item->flags() );
            item->setTransformationMode(Qt::SmoothTransformation);
            _currentPixmap.loadFromData(data);
            item->setPixmap(_currentPixmap);
            ui->graphicsViewWidget->scene()->setSceneRect(_currentPixmap.rect());
            ui->graphicsViewWidget->scene()->addItem(item);
            ui->graphicsViewWidget->fitInView(item,Qt::KeepAspectRatio);
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
    eff =  new QGraphicsOpacityEffect(this);
    if(eff!=nullptr){
        this->setGraphicsEffect(eff);
        QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
        a->setDuration(300);
        a->setStartValue(1);
        a->setEndValue(0);
        a->setEasingCurve(QEasingCurve::Linear);
        connect(a,&QPropertyAnimation::finished,[=](){
            _screenshot   = true;
            _pixmapLoaded = false;
            ui->graphicsViewWidget->resetMatrix();
            ui->graphicsViewWidget->scene()->clear();
            ui->copyCommand->hide();
            ui->copyCommand->setEnabled(true);
            wall_loader->stop();
            ui->next->show();
            ui->previous->show();
            this->close();
            eff->deleteLater();
        });
        a->start(QPropertyAnimation::DeleteWhenStopped);
    }
}

void Screenshots::on_copyCommand_clicked()
{
    if(!ui->graphicsViewWidget->scene()->items().isEmpty()){
        ui->copyCommand->setEnabled(true);
        QGraphicsItem *item = ui->graphicsViewWidget->scene()->items().first();
        QGraphicsTextItem *commandItem = static_cast<QGraphicsTextItem*>(item);
        if(clipboard==nullptr)
            clipboard = QApplication::clipboard();
        clipboard->setText(commandItem->toPlainText().trimmed().simplified());
    }else{
        ui->copyCommand->setEnabled(false);
    }
}


