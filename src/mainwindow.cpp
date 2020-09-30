#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QDesktopServices>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(QApplication::applicationName());
    this->setWindowIcon(QIcon(":/icons/app/icon-128.png"));
    setStyle(":/qbreeze/dark.qss");

    if(settings.value("geometry").isValid()){
        restoreGeometry(settings.value("geometry").toByteArray());
        if(settings.value("windowState").isValid()){
            restoreState(settings.value("windowState").toByteArray());
        }else{
            QScreen* pScreen = QApplication::primaryScreen();// (this->mapToGlobal({this->width()/2,0}));
            QRect availableScreenSize = pScreen->availableGeometry();
            this->move(availableScreenSize.center()-this->rect().center());
        }
    }

    QSplitter *split1 = new QSplitter;
    split1->setObjectName("split1");
    split1->setOrientation(Qt::Horizontal);
    split1->addWidget(ui->resultsWidget);
    split1->addWidget(ui->detailWidget);
    split1->setStretchFactor(0,4);
    split1->setStretchFactor(3,1);
    ui->centerLayout->addWidget(split1);

    if(settings.value("split_state").isValid()){
        split1->restoreState(settings.value("split_state").toByteArray());
    }

    if(settings.value("split_geometry").isValid()){
        split1->restoreGeometry(settings.value("split_geometry").toByteArray());
    }

    _loader = new WaitingSpinnerWidget(ui->results,true,true);
    _loader->setRoundness(70.0);
    _loader->setMinimumTrailOpacity(15.0);
    _loader->setTrailFadePercentage(70.0);
    _loader->setNumberOfLines(10);
    _loader->setLineLength(8);
    _loader->setLineWidth(2);
    _loader->setInnerRadius(2);
    _loader->setRevolutionsPerSecond(3);
    _loader->setColor(QColor("#1e90ff"));


    // wall_view is the child of monitor
    _wall_view = new RemotePixmapLabel(ui->monitor);
    _wall_view->setAlignment(Qt::AlignCenter);
    _wall_view->setPixmap(QPixmap(":/icons/others/wall_placeholder_180.jpg"));
    _wall_view->setGeometry(12,26,319,180);
    _wall_view->show();
    _wall_view->installEventFilter(this);

    _wall_view->setAttribute(Qt::WA_Hover, true);
    _wall_view->setMouseTracking(true);
    connect(_wall_view,&RemotePixmapLabel::pixmapLoaded,[=](QByteArray data){
        Q_UNUSED(data);
        wall_loader->stop();
    });
    connect(_wall_view,&RemotePixmapLabel::pixmapLoading,[=](){
       wall_loader->start();
    });

    ui->wallpaperList->hide();
    ui->categoryCombo->setIconSize(QSize(16,16));
    ui->categoryCombo->setStyleSheet("background-color:"+ui->description->palette().color(QWidget::backgroundRole()).name());

    ui->queryEdit->addAction(QIcon(":/icons/others/snapcraft.png"),QLineEdit::LeadingPosition);


    ui->scrollArea->setMinimumWidth(ui->scrollAreaWidgetContents->minimumSizeHint().width()+32);
    ui->resultsWidget->setMinimumWidth(ui->scrollAreaWidgetContents->minimumSizeHint().width()+32);

    ui->scrollArea->setFrameStyle(QFrame::NoFrame);
    ui->scrollArea->setStyleSheet("QFrame#"+ui->scrollArea->objectName()+"{border:none}");

    ui->detailWidget->setStyleSheet("background-color:"+ui->description->palette().color(QWidget::backgroundRole()).name());

    ui->description->setStyleSheet("QTextBrowser { padding:9px;border:none;}");
    ui->meta->setStyleSheet("QTextBrowser { padding:9px;border:none;}");
    ui->meta_bottom->setStyleSheet("QTextBrowser { padding:9px;border:none;}");

    ui->backBtn->setEnabled(ui->navigationCombo->count()>0);
    ui->forwardBtn->setEnabled(ui->navigationCombo->count()>0);
    ui->navigationCombo->hide();

    foreach (QPushButton *btn, ui->appButtonWidget->findChildren<QPushButton*>()) {
        btn->setEnabled(false);
    }

    // _ui_action is the child of wall_view
    QWidget *actionWidget = new QWidget(_wall_view);
    actionWidget->setObjectName("actions_view");
    _ui_action.setupUi(actionWidget);
    actionWidget->setStyleSheet("QWidget#"+actionWidget->objectName()+"{background-color: transparent;}");

    connect(_ui_action.fullscreen,&QPushButton::clicked,[=](){
        QGraphicsScene* scene = new QGraphicsScene(this);
        QGraphicsView* view = new QGraphicsView(scene);
        RemotePixmapLabel *scl = new RemotePixmapLabel(nullptr);
        QGraphicsPixmapItem* item = new QGraphicsPixmapItem();
        connect(scl,&RemotePixmapLabel::pixmapLoaded,[=](QByteArray data){
            QPixmap pixmap;
            pixmap.loadFromData(data);
            item->setPixmap(pixmap);
            scene->setSceneRect(pixmap.rect());
            scene->addItem(item);
        });
        scl->setRemotePixmap(_currentUrl);
        view->setAttribute(Qt::WA_DeleteOnClose);
        view->setStyleSheet("border:none");
        view->fitInView(item,Qt::KeepAspectRatio);
        view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        view->setGeometry(ui->centerLayout->geometry());
        view->show();
    });
    actionWidget->setGeometry(_wall_view->rect());
    actionWidget->hide();

    // loader is the child of wall_view
    wall_loader = new WaitingSpinnerWidget(_wall_view,true,false);
    wall_loader->setRoundness(70.0);
    wall_loader->setMinimumTrailOpacity(15.0);
    wall_loader->setTrailFadePercentage(70.0);
    wall_loader->setNumberOfLines(40);
    wall_loader->setLineLength(20);
    wall_loader->setLineWidth(2);
    wall_loader->setInnerRadius(2);
    wall_loader->setRevolutionsPerSecond(3);
    wall_loader->setColor(QColor(159,160,164));

    QIcon left;
    left.addPixmap(QPixmap(":/icons/arrow-left-circle-line.png"), QIcon::Normal);
    left.addPixmap(QPixmap(":/icons/disabled-arrow-left-circle-line.png"), QIcon::Disabled);
    ui->left->setIcon(left);

    QIcon right;
    right.addPixmap(QPixmap(":/icons/arrow-right-circle-line.png"), QIcon::Normal);
    right.addPixmap(QPixmap(":/icons/disabled-arrow-right-circle-line.png"), QIcon::Disabled);
    ui->right->setIcon(right);

    elements <<"name"<<"confinement"<<"type"<<"version"<<"categories"
             <<"description"<<"pub_uname"<<"pub_dname"<<"pub_id"
             <<"icon"<<"screenshots"<<"contact"<<"website"
             <<"summary"<<"title"<<"snap-id"<<"size"<<"channel";

    m_store = new Store(this);
    connect(m_store,SIGNAL(response(QString)),this,SLOT(processResponse(QString)));
    connect(m_store,SIGNAL(gotCategories(QString)),this,SLOT(processCategories(QString)));
    _loader->start();
    m_store->getCategories();
    connect(m_store,&Store::error,[=](QString errorString){
        showError(errorString);
        _loader->stop();
    });
}

void MainWindow::setStyle(QString fname)
{
    QFile styleSheet(fname);
    if (!styleSheet.open(QIODevice::ReadOnly))
    {
        qWarning("Unable to open file");
        return;
    }
    qApp->setStyleSheet(styleSheet.readAll());
    styleSheet.close();
    qApp->setStyle(QStyleFactory::create("fusion"));
    QPalette palette;
    palette.setColor(QPalette::Window,QColor(53,53,53));
    palette.setColor(QPalette::WindowText,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
    palette.setColor(QPalette::Base,QColor(42,42,42));
    palette.setColor(QPalette::AlternateBase,QColor(66,66,66));
    palette.setColor(QPalette::ToolTipBase,Qt::white);
    palette.setColor(QPalette::ToolTipText,QColor(53,53,53));
    palette.setColor(QPalette::Text,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
    palette.setColor(QPalette::Dark,QColor(35,35,35));
    palette.setColor(QPalette::Shadow,QColor(20,20,20));
    palette.setColor(QPalette::Button,QColor(53,53,53));
    palette.setColor(QPalette::ButtonText,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
    palette.setColor(QPalette::BrightText,Qt::red);
    palette.setColor(QPalette::Link,QColor("skyblue"));
    palette.setColor(QPalette::Highlight,QColor(49,106,150));
    palette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
    palette.setColor(QPalette::HighlightedText,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));
    qApp->setPalette(palette);
}

void MainWindow::loadCategory(const QString catName)
{
    _loader->start();
    m_store->loadCategory(catName);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_queryEdit_textChanged(const QString &arg1)
{
    ui->searchBtn->setEnabled(!arg1.isEmpty());
}

void MainWindow::on_searchBtn_clicked()
{
    stopLoadingResults = true;  // to stop result loader loop
    ui->results->clear();
    _loader->start();

    QString term = ui->queryEdit->text().trimmed().split(":").last();
    QString protocol = ui->queryEdit->text().trimmed().split(term).first();
    QStringList protocols;
    protocols<<"snap-by:"<<"snap-search:"<<"snap-category:"<<"snap-related:";
    switch (protocols.indexOf(protocol)) {
        case 0:{
            m_store->search(term);
            setStatus("Loading snaps by <b style='color:skyblue'>'"+term+"'...</b>");
            break;
        }
        case 1:{
            m_store->search(term);
            setStatus("Searching for <b style='color:skyblue'>'"+term+"'...</b>");
            break;
        }
        case 2:{
            m_store->loadCategory(term);
            setStatus("Loading snaps from category <b style='color:skyblue'>'"+term+"'...</b>");
            break;
        }
        case 3:{
            m_store->search(term);
            setStatus("Loading snaps related to <b style='color:skyblue'>'"+term+"'...</b>");
            break;
        }
        default:{
            m_store->search(term);
            setStatus("Searching for <b style='color:skyblue'>'"+term+"'...</b>");
            break;
        }
    }
}

void MainWindow::showError(QString message)
{
    _error = new Error(this);
    _error->setAttribute(Qt::WA_DeleteOnClose);
    _error->setWindowTitle(QApplication::applicationName()+" | Error dialog");
    _error->setWindowFlags(Qt::Dialog);
    _error->setWindowModality(Qt::NonModal);
    _error->setError("An Error ocurred while processing your request!",
                     message);
    _error->show();
    _loader->stop();
}

void MainWindow::setStatus(QString message)
{
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    ui->status->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(500);
    a->setStartValue(0);
    a->setEndValue(1);
    a->setEasingCurve(QEasingCurve::InCurve);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    ui->status->setText(message);
    ui->status->show();
}

void MainWindow::processCategories(QString reply)
{
    ui->categoryCombo->blockSignals(true);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply.toUtf8());
    if(jsonResponse.isEmpty()){
        showError("API:Empty response returned from API call. Please report to developer.");
        setStatus("Request finished, unable to load category data.");
        return;
    }
    QJsonArray jsonArray = jsonResponse.object().value("categories").toArray();
    foreach (const QJsonValue &val, jsonArray) {
        QString orig_catname = val.toObject().value("name").toString().trimmed();
        QIcon icon(":/icons/categories/"+orig_catname+".png");
        QString fine_catname = orig_catname;
        fine_catname         = utils::toCamelCase(fine_catname.replace("-"," "));
        ui->categoryCombo->addItem(icon,fine_catname,orig_catname);
    }
    ui->categoryCombo->blockSignals(false);
    ui->homeBtn->click();
}

void MainWindow::processResponse(QString reply)
{
    ui->results->clear();
    addToHistory(ui->queryEdit->text().trimmed());
    currentResultCount = 0;
    currentResultData  = "";
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply.toUtf8());
    if(jsonResponse.isEmpty()){
        showError("API:Empty response returned from API call. Please report to developer.");
        setStatus("Request finished, no result found.");
        return;
    }

    currentResultData    = reply;
    QList<QVariantList> data;

    QJsonArray jsonArray = jsonResponse.object().value("results").toArray();
    currentResultCount   = jsonArray.count();
    foreach (const QJsonValue &val, jsonArray) {
        QJsonObject object = val.toObject();
        data.append(itemInfo(object));
    }
    if(data.isEmpty()){
        showError("No result found.");
        setStatus("Request finished, no result found.");
        _loader->stop();
        return;
    }
    //load data into view
    stopLoadingResults = false;
    load_data_into_view(data,false);
}

void MainWindow::addToHistory(const QString arg1)
{
    if(ui->navigationCombo->currentIndex() == ui->navigationCombo->count()-1){
        if(ui->navigationCombo->currentText()!=arg1)
            ui->navigationCombo->addItem(arg1);
        ui->navigationCombo->setCurrentIndex(ui->navigationCombo->count()-1);
    }

    bool forwardStepAvailable  = false;
    bool backwardStepAvailable = false;

    int currentItemIndex       = ui->navigationCombo->currentIndex(); //0
    int nextItemIndex          = currentItemIndex+1;
    int prevItemIndex          = currentItemIndex-1; //0-1 = -1

    forwardStepAvailable  = ui->navigationCombo->count() != 1 && nextItemIndex < ui->navigationCombo->count() ? true:false ;
    backwardStepAvailable = ui->navigationCombo->count() != 1 && prevItemIndex != -1 ? true:false ;

    ui->backBtn->setEnabled(backwardStepAvailable);
    ui->forwardBtn->setEnabled(forwardStepAvailable);

    QString nextItem,prevItem;
    nextItem = ui->navigationCombo->itemText(nextItemIndex);
    prevItem = ui->navigationCombo->itemText(prevItemIndex);

    backwardStepAvailable ? ui->backBtn->setToolTip("Go back to "+prevItem):ui->backBtn->setToolTip("Back");
    forwardStepAvailable ? ui->forwardBtn->setToolTip("Go forward to "+nextItem):ui->forwardBtn->setToolTip("Forward");
}

void MainWindow::on_backBtn_clicked()
{
    int historyItemIndex = ui->navigationCombo->currentIndex()-1;
    ui->navigationCombo->setCurrentIndex(historyItemIndex);
    QString historyItem  = ui->navigationCombo->itemText(historyItemIndex);
    ui->queryEdit->setText(historyItem);
    on_searchBtn_clicked();
}

void MainWindow::on_forwardBtn_clicked()
{
    int historyItemIndex = ui->navigationCombo->currentIndex()+1;
    ui->navigationCombo->setCurrentIndex(historyItemIndex);
    QString historyItem  = ui->navigationCombo->itemText(historyItemIndex);
    ui->queryEdit->setText(historyItem);
    on_searchBtn_clicked();
}


void MainWindow::load_data_into_view(QList<QVariantList> data,bool appending)
{
    int lastItemIndex = -1;
    int newStop       = 0;
    QListWidgetItem * firstNewitem = nullptr;
    if(appending){
        newStop = ui->results->count()+settings.value("resultsToShow",100).toInt();
        qDebug()<<"yes"<<newStop;
        if(_loader->isSpinning()==false){
            _loader->start();
        }
    }
    for (int i = 0; i < data.count(); i++)
    {
        QVariantList item_meta = data.at(i);

        if(item_meta.count()==elements.count())
        {
            QString title   = item_meta.at(elements.indexOf("title")).toString();
            QString summary = item_meta.at(elements.indexOf("summary")).toString();
            QString snapId  = item_meta.at(elements.indexOf("snap-id")).toString();
            QString iconUrl = item_meta.at(elements.indexOf("icon")).toString();

            QString p_des   = "<b>Version :</b> "+item_meta.at(elements.indexOf("version")).toString()+"<br>"+
                              "<b>By :</b> "+item_meta.at(elements.indexOf("pub_dname")).toString()+
                              "<br>";
            if(appending){
                if(ui->results->count()==newStop)
                break;
            }else{
                if(ui->results->count()==settings.value("resultsToShow",100).toInt()){
                    break;
                }
            }
            QWidget *track_widget = new QWidget(ui->results);
            track_widget->setObjectName("track-widget-"+snapId);

            track_ui.setupUi(track_widget);
            track_widget->setStyleSheet("QWidget#"+track_widget->objectName()+"{background-color: transparent;}");

            track_ui.title->setText(title);
            track_ui.summary->setText(summary);
            track_widget->setToolTip(title+" ("+summary+")");

            track_ui.meta->setTextFormat(Qt::RichText);
            track_ui.meta->setText(p_des);


            double ratio  = 200.0/200.0; //actual image aspect ratio
            double height = track_widget->minimumSizeHint().height();
            if(trackCoverWidth==0){
                trackCoverWidth  = ratio * height; //calculated width based on ratio
                trackWidgetSizeHint = track_widget->minimumSizeHint();
            }
            track_ui.icon->setFixedSize(trackCoverWidth,trackCoverWidth);

            track_ui.icon->setRemotePixmap(iconUrl);

            QListWidgetItem* item;
            item = new QListWidgetItem(ui->results);
            item->setData(Qt::UserRole,item_meta);
            item->setSizeHint(trackWidgetSizeHint);
            ui->results->setItemWidget(item,track_widget);

            ui->results->addItem(item);
            if(lastItemIndex == -1){
                firstNewitem = item;
            }
            lastItemIndex = i;
            if(stopLoadingResults){
                ui->results->clear();
                break;
            }else{
                QApplication::processEvents();
            }
        }else{
            qDebug()<<"NOT ADDED:"<<item_meta.count()<<elements.count()<<item_meta<<"\n";
        }
    }
    if(firstNewitem!=nullptr){
        ui->results->setCurrentRow(ui->results->row(firstNewitem));
        ui->results->scrollToItem(firstNewitem);
        _loader->stop();
    }

    QString statusSubString;
    //qDebug()<<data.count()<<ui->results->count();
    if(appending){
        int total = data.count();
        if(total<currentResultCount){
             total = data.count()+ui->results->count();
        }
        if(total>ui->results->count()){
            statusSubString = "<a href='data://"+QString::number(ui->results->count()+lastItemIndex)+"'>Load more results</a>";
        }
        setStatus("Showing <b>"+QString::number(ui->results->count())+"</b> results out of <b>"+QString::number(currentResultCount)+"</b> available results. "+statusSubString);
    }else{
        if(data.count()>ui->results->count()){
            statusSubString = "<a href='data://"+QString::number(lastItemIndex)+"'>Load more results</a>";
        }
        setStatus("Showing <b>"+QString::number(ui->results->count())+"</b> results out of <b>"+QString::number(data.count())+"</b> available results. "+statusSubString);
    }
}

void MainWindow::append_more_results(int index)
{
    QJsonDocument jsonResponse = QJsonDocument::fromJson(currentResultData.toUtf8());
    if(jsonResponse.isEmpty()){
        showError("API:Empty response returned from API call. Please report to developer.");
        setStatus("Request finished, no result found.");
        return;
    }

    QList<QVariantList> data;
    QJsonArray jsonArray = jsonResponse.object().value("results").toArray();
    for (int i = index; i < jsonArray.count(); i++) {
        QJsonObject object = jsonArray.at(i).toObject();
        data.append(itemInfo(object));
    }
    if(data.isEmpty()){
        showError("No result found.");
        setStatus("Request finished, no result found.");
        return;
    }
    //load data into view
    stopLoadingResults = false;
    load_data_into_view(data,true);
}

QVariantList MainWindow::itemInfo(const QJsonObject object)
{
    QVariantList item;
    foreach (QString ele, elements) {
        if(ele=="confinement"){
            QString value = object.value("revision").toObject().value(ele).toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="version"){
            QString value = object.value("revision").toObject().value(ele).toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="type"){
            QString value = object.value("revision").toObject().value(ele).toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="size"){
            double size = object.value("revision").toObject().value("download").toObject().value(ele).toDouble();
            QString value = utils::humanReadableSize(size);
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="channel"){
            QString value = object.value("revision").toObject().value(ele).toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="categories"){
            QJsonArray  cat_arr = object.value("snap").toObject().value(ele).toArray();
            QString catStr;
            foreach (const QJsonValue &cat_item, cat_arr) {
                QString cat = cat_item.toObject().value("name").toString();
                if(cat!="featured")
                catStr.append(", "+cat);
            }
            catStr.remove(0,1);
            catStr = catStr.isEmpty()?"-":catStr;
            item.append(catStr);
        } else if(ele=="description"){
            QString value = object.value("snap").toObject().value(ele).toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="pub_uname"){
            QString value = object.value("snap").toObject().value("publisher").toObject().value("username").toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="pub_dname"){
            QString value = object.value("snap").toObject().value("publisher").toObject().value("display-name").toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="pub_id"){
            QString value = object.value("snap").toObject().value("publisher").toObject().value("id").toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="screenshots"){
            QJsonArray  media_arr = object.value("snap").toObject().value("media").toArray();
            QString ssStr;
            foreach (const QJsonValue &media_item, media_arr) {
                if(media_item.toObject().value("type")=="screenshot")
                ssStr.append(", "+media_item.toObject().value("url").toString());
            }
            ssStr.remove(0,1);
            ssStr = ssStr.isEmpty()?"qrc:///icons/others/wall_placeholder_180.jpg":ssStr;
            item.append(ssStr);
        } else if(ele=="icon"){
            QJsonArray  media_arr = object.value("snap").toObject().value("media").toArray();
            QString iconUrl;
            foreach (const QJsonValue &media_item,media_arr) {
                if(media_item.toObject().value("type")=="icon")
                iconUrl.append(media_item.toObject().value("url").toString());
            }
            iconUrl = iconUrl.trimmed().isEmpty()?"qrc:///icons/others/snapcraft.png":iconUrl.trimmed();
            item.append(iconUrl);
        } else if(ele=="summary"){
            QString value = object.value("snap").toObject().value("summary").toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="title"){
            QString value = object.value("snap").toObject().value("title").toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="contact"){
            QString value = object.value("snap").toObject().value("contact").toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else if(ele=="website"){
            QString value = object.value("snap").toObject().value("website").toString();
            value = value.isEmpty()?"-":value;
            item.append(value);
        } else{
            item.append(object.value(ele).toVariant());
        }
    }
    return item;
}

void MainWindow::on_queryEdit_returnPressed()
{
    if(ui->queryEdit->text().trimmed().isEmpty()==false){
        ui->searchBtn->click();
    }
}

void MainWindow::on_status_linkActivated(const QString &link)
{
    if(link.contains("data://",Qt::CaseInsensitive)){
        append_more_results(link.split("data://").last().toInt()+1);
    }
}


void MainWindow::on_results_currentRowChanged(int currentRow)
{
    foreach (QPushButton *btn, ui->appButtonWidget->findChildren<QPushButton*>()) {
        btn->setEnabled(false);
    }
    ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->minimum());
    _wall_view->setPixmap(QPixmap(":/icons/others/wall_placeholder_180.jpg"));
    ui->wallpaperList->clear();
    QListWidgetItem *item = ui->results->item(currentRow);
    if(item!=nullptr){
        QStringList item_meta = item->data(Qt::UserRole).toStringList();
        showItemDetail(item_meta);
    }else{
        ui->description->clear();
    }
}

void MainWindow::showItemDetail(QStringList item_meta)
{
    foreach (QPushButton *btn, ui->appButtonWidget->findChildren<QPushButton*>()) {
        btn->setEnabled(true);
    }

    QStringList catLinks = item_meta.at(elements.indexOf("categories")).split(",");
    QString catStr;
    foreach (QString link, catLinks) {
        catStr.append("   ["+link.trimmed()+"](snap-category:"+link.trimmed()+")");
    }

    QString channel           = item_meta.at(elements.indexOf("channel"));
    QString descriptionPrefix = "<center>\n\n ## "+item_meta.at(elements.indexOf("title"))+"\n\n"+
                                "**"+item_meta.at(elements.indexOf("summary"))+"**\n\n\n</center>";

    QString descriptionSuffix = "*Package Name : "+item_meta.at(elements.indexOf("name"))+"*\n\n"+
                                "*Package Size : "+item_meta.at(elements.indexOf("size"))+"*\n\n"+
                                "*Confinement : "+item_meta.at(elements.indexOf("confinement"))+"*\n\n"+
                                "*Categories : "+catStr.remove(0,1)+"*\n\n\n"+
                                "`snap install "+item_meta.at(elements.indexOf("name"))+
                                                               QString(channel=="stable"?"":" --"+channel)+"`\n\n\n";
    if(htmlParser==nullptr)
    htmlParser = new Md2Html(this);

    ui->meta->setHtml(htmlParser->toHtml(descriptionPrefix));
    ui->meta->setFixedHeight(ui->meta->document()->size().height()+18);
    ui->description->setHtml(htmlParser->toHtml(item_meta.at(elements.indexOf("description"))));
    ui->description->setFixedHeight(ui->description->document()->size().height()+18);
    ui->meta_bottom->setHtml(htmlParser->toHtml(descriptionSuffix));
    ui->meta_bottom->setFixedHeight(ui->meta_bottom->document()->size().height()+18);

    setAppButtons(item_meta);
    QStringList scUrls = item_meta.at(elements.indexOf("screenshots")).split(",");
    for (int i=0;i<scUrls.count();i++) {
        QString thumbUrl = scUrls.at(i).trimmed();
        if(thumbUrl.trimmed().isEmpty()==false){
            QWidget *listwidget = new QWidget(ui->wallpaperList);
            _ui_listitem.setupUi(listwidget);
            _ui_listitem.thumbUrl->setText(thumbUrl);
            _ui_listitem.thumbUrl->hide();
            _ui_listitem.fullUrl->setText(thumbUrl);
            _ui_listitem.fullUrl->hide();
            listwidget->adjustSize();
            QListWidgetItem* item;
            item = new QListWidgetItem(ui->wallpaperList);
            _ui_listitem.thumbnail->setScaledContents(false);
            _ui_listitem.thumbnail->setPixmap(QPixmap(":/icons/others/wall_placeholder_180.jpg")
                                              .scaled(QSize(146,82),Qt::KeepAspectRatio,Qt::FastTransformation));

            item->setSizeHint(listwidget->minimumSizeHint());

            double ratio  = 146.0/82.0;
            double height = listwidget->height();
            double width  = ratio * height;

            _ui_listitem.thumbnail->setFixedSize(width,height);
            _ui_listitem.thumbnail->setRemotePixmap(thumbUrl);

            ui->wallpaperList->setItemWidget(item, listwidget);

            ui->wallpaperList->addItem(item);
            //set first item in wall_view
            if(i==0){
                ui->wallpaperList->setCurrentRow(0);
            }
            //keep updaing navigation buttons as we add items synchronously
            updateNavigationButtons();
        }
    }
}

inline QString summaryToKeywords(const QString summary,const QString appname)
{
    QString keywords;
    foreach (QString word, summary.split(" ")) {
        QString trimmedWord = word.trimmed();
        if(!trimmedWord.isEmpty() && trimmedWord.length() > 4)
            keywords.append(" "+utils::cleanString(trimmedWord).trimmed().simplified());
    }
    keywords = keywords.remove(0,1);

    QMap<QString,int> countOfStrings;
    QMap<int,QString> countOfStringsFinal;
    QStringList listOfStrings;
    listOfStrings = keywords.split(" ");
    for(int i=0;i<listOfStrings.count();i++)
    {
        countOfStrings[listOfStrings[i]]++;
    }

    for(int i = 0 ;i< countOfStrings.count();i++){
        QString word = countOfStrings.key(i);
        int key = countOfStrings.value(countOfStrings.key(i));
        if(!word.trimmed().isEmpty()
                && key > 0
                && !word.contains(appname,Qt::CaseInsensitive)
                && !countOfStringsFinal.values().contains(word+"s")
                && !countOfStringsFinal.values().contains(word+"es")
                && !countOfStringsFinal.values().contains(word.toLower()+"s")
                && !countOfStringsFinal.values().contains(word.toLower()+"es")
                && !countOfStringsFinal.values().contains(utils::toCamelCase(word+"s"))
                && !countOfStringsFinal.values().contains(utils::toCamelCase(word+"es"))
                )
        countOfStringsFinal.insert(key,word);
    }
    //qDebug()<<countOfStringsFinal;
    return countOfStringsFinal.last();
}

void MainWindow::setAppButtons(QStringList item_meta)
{
    QString devname  = item_meta.at(elements.indexOf("pub_uname")).trimmed();
    QString devdname = item_meta.at(elements.indexOf("pub_dname")).trimmed();
    ui->moreAppsDev->show();
    ui->moreAppsDev->setText("Snaps by "+devdname);

    disconnect(ui->moreAppsDev,SIGNAL(clicked()),0,0);
    ui->moreAppsDev->setToolTip("Load more snaps by developer");
    connect(ui->moreAppsDev,&QPushButton::clicked,[=](){
       ui->queryEdit->setText("snap-by:"+devname);
       ui->searchBtn->click();
    });

    disconnect(ui->appContactBtn,SIGNAL(clicked()),0,0);
    QString contactUrl = item_meta.at(elements.indexOf("contact")).trimmed();
    ui->appContactBtn->setToolTip("Open: "+contactUrl);
    connect(ui->appContactBtn,&QPushButton::clicked,[=](){
        QDesktopServices::openUrl(QUrl(contactUrl));
    });

    disconnect(ui->appWebsiteBtn,SIGNAL(clicked()),0,0);
    QString websiteUrl = item_meta.at(elements.indexOf("website")).trimmed();
    ui->appWebsiteBtn->setToolTip("Open: "+websiteUrl);
    connect(ui->appWebsiteBtn,&QPushButton::clicked,[=](){
       QDesktopServices::openUrl(QUrl(websiteUrl));
    });

    disconnect(ui->moreSimilarApps,SIGNAL(clicked()),0,0);
    QString summary    = item_meta.at(elements.indexOf("summary")).trimmed();
    QString desciption = item_meta.at(elements.indexOf("description")).trimmed();
    QString category   = item_meta.at(elements.indexOf("categories")).trimmed();
    QString appname    = item_meta.at(elements.indexOf("title")).trimmed();
    ui->moreSimilarApps->setToolTip("Open: "+websiteUrl);
    connect(ui->moreSimilarApps,&QPushButton::clicked,[=](){
        ui->queryEdit->setText("snap-search:"+summaryToKeywords(summary+" "+desciption+" "+category,appname));
        ui->searchBtn->click();
    });
}

void MainWindow::updateNavigationButtons()
{
    int currentRow = ui->wallpaperList->currentRow();
    int totalItems = ui->wallpaperList->count();
    if(currentRow < totalItems-1){
        ui->right->setEnabled(true);
    }else{
        ui->right->setEnabled(false);
    }
    if(currentRow > 0 ){
        ui->left->setEnabled(true);
    }else{
        ui->left->setEnabled(false);
    }
}

void MainWindow::on_right_clicked()
{
    ui->wallpaperList->setCurrentRow(ui->wallpaperList->currentRow()+1);
}

void MainWindow::on_left_clicked()
{
  ui->wallpaperList->setCurrentRow(ui->wallpaperList->currentRow()-1);
}

void MainWindow::on_wallpaperList_currentRowChanged(int currentRow)
{
    on_wallpaperList_itemClicked(ui->wallpaperList->item(currentRow));
}

void MainWindow::on_wallpaperList_itemClicked(QListWidgetItem *item)
{
    QWidget *listwidget = ui->wallpaperList->itemWidget(item);
    if(listwidget!=nullptr)
    {
            QLineEdit *fullUrl = listwidget->findChild<QLineEdit*>("fullUrl");
            this->_currentUrl = fullUrl->text();
            QLineEdit *thumbUrl = listwidget->findChild<QLineEdit*>("thumbUrl");
            _wall_view->setRemotePixmap(thumbUrl->text().trimmed());
    }
    updateNavigationButtons();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == _wall_view && !_currentUrl.isEmpty()) {
        const QHoverEvent* const he = static_cast<const QHoverEvent*>( event );
        QWidget *actionWidget = _wall_view->findChild<QWidget*>("actions_view");
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(actionWidget);
        switch(he->type())
        {
        case QEvent::HoverEnter:{
            if(eff!=nullptr){
                actionWidget->setGraphicsEffect(eff);
                QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
                a->setDuration(250);
                a->setStartValue(0);
                a->setEndValue(1);
                a->setEasingCurve(QEasingCurve::Linear);
                a->start(QPropertyAnimation::DeleteWhenStopped);
                actionWidget->show();
            }
            break;
        }
        case QEvent::HoverLeave:{
            if(eff!=nullptr){
                actionWidget->setGraphicsEffect(eff);
                QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
                a->setDuration(250);
                a->setStartValue(1);
                a->setEndValue(0);
                a->setEasingCurve(QEasingCurve::Linear);
                connect(a,&QPropertyAnimation::finished,[=](){
                    actionWidget->hide();
                });
                a->start(QPropertyAnimation::DeleteWhenStopped);
            }
            break;
        }
        default:
            break;
        }
    }
    return QMainWindow::eventFilter(obj,event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSplitter *split1 = this->findChild<QSplitter*>("split1");
    settings.setValue("split_state",split1->saveState());
    settings.setValue("split_geometry",split1->saveGeometry());
    settings.setValue("geometry",saveGeometry());
    settings.setValue("windowState", saveState());
    QWidget::closeEvent(event);
}


void MainWindow::on_homeBtn_clicked()
{
    loadCategory("featured");
    ui->categoryCombo->blockSignals(true);
    ui->queryEdit->setText("snap-category:featured");
    ui->categoryCombo->setCurrentText("Featured");
    ui->categoryCombo->blockSignals(false);
}

void MainWindow::on_categoryCombo_currentIndexChanged(int index)
{
    QString catname = ui->categoryCombo->itemData(index,Qt::UserRole).toString();
    ui->queryEdit->setText("snap-category:"+catname);
    ui->searchBtn->click();
}

void MainWindow::on_description_anchorClicked(const QUrl &arg1)
{
    QDesktopServices::openUrl(arg1);
}

void MainWindow::on_meta_bottom_anchorClicked(const QUrl &arg1)
{
    if(arg1.toString().contains("snap-category:",Qt::CaseSensitive)){
        ui->queryEdit->setText(arg1.toString().trimmed());
        ui->searchBtn->click();
    }else{
        QDesktopServices::openUrl(arg1);
    }
}


