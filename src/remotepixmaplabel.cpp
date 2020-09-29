#include "remotepixmaplabel.h"

#include <QPixmap>
#include <QUrl>
#include <QDebug>
#include <QNetworkDiskCache>


RemotePixmapLabel::~RemotePixmapLabel()
{

    delete diskCache;
    delete networkManager_;

}

void RemotePixmapLabel::setRemotePixmap(const QString& url )
{
    if (!networkManager_) {
        retries = 2;
        QString net_cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        diskCache  = new QNetworkDiskCache(this);
        diskCache->setCacheDirectory(net_cache_path);
        networkManager_ = new QNetworkAccessManager(this);
        networkManager_->setCache(diskCache);
        connect(networkManager_, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(pixmapReceived(QNetworkReply*)));
    }
    networkManager_->get(QNetworkRequest(QUrl(url)));
    emit pixmapLoading();
}

void RemotePixmapLabel::pixmapReceived(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QString posterUrl = "qrc:///icons/others/snapcraft.png";
        qDebug() << Q_FUNC_INFO << "pixmap receiving error" << reply->error();
        if(retries>0){
            retries = retries - 1;
            networkManager_->get(QNetworkRequest(QUrl(reply->request().url())));
        }else{
            networkManager_->get(QNetworkRequest(QUrl(posterUrl)));
        }
    }else{
        const QByteArray data(reply->readAll());
        if (data.size() == 0)
            qDebug() << Q_FUNC_INFO << "received pixmap looks like nothing";

        QPixmap pixmap;
        pixmap.loadFromData(data);
        setPixmap(pixmap.scaled(this->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
        if(pixmap.isNull()==false){
            emit pixmapLoaded(data);
        }
    }
    reply->deleteLater();
    networkManager_->deleteLater();
    diskCache->deleteLater();
    networkManager_ = nullptr;
    diskCache = nullptr;
}
