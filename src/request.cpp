#include "request.h"

Request::Request(QObject *parent) : QObject(parent)
{
    setParent(parent);
    _cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

    m_netwManager = new QNetworkAccessManager(this);
    QNetworkDiskCache* diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(_cache_path);
    m_netwManager->setCache(diskCache);
    connect(m_netwManager,&QNetworkAccessManager::finished,[=](QNetworkReply* rep){
        operations.removeOne(rep);
        if(rep->error() == QNetworkReply::NoError){
            QString repStr = rep->readAll();
            emit requestFinished(repStr);
        }else{
            emit downloadError("Unable to load: "+rep->request().url().toString()+"\n "+rep->errorString());
        }
        rep->deleteLater();
    });
}

Request::~Request()
{
    m_netwManager->deleteLater();
    this->deleteLater();
}

void Request::clearCache(QUrl url)
{
   m_netwManager->cache()->remove(url);
}

void Request::cancelAll()
{
    foreach (QNetworkReply *rep,operations) {
        if(rep!=nullptr){
            rep->abort();
            rep->deleteLater();
        }
    }
}

void Request::get(const QUrl url, bool preferCache)
{
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.125 Safari/537.36");

    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
    preferCache ? QNetworkRequest::PreferCache : QNetworkRequest::PreferNetwork);

    QNetworkReply *reply = m_netwManager->get(request);
    operations.append(reply);
    connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(downloadProgress(qint64,qint64)));
    emit requestStarted();
}

void Request::downloadProgress(qint64 got,qint64 tot)
{
    double downloaded_Size = (double)got;
    double total_Size = (double)tot;
    double progress = (downloaded_Size/total_Size) * 100;
    int intProgress = static_cast<int>(progress);
    emit _downloadProgress(intProgress);
}
