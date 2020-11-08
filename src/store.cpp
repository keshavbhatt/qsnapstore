#include "store.h"
#include "request.h"
#include "utils.h"
#include <QJsonDocument>
#include <QCryptographicHash>

Store::Store(QObject *parent) : QObject(parent)
{
    _cache_path   = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    api_end_point = "https://api.snapcraft.io/";
    fields        = "confinement,categories,description,"
                     "media,publisher,summary,title,type,"
                     "version,contact,website,download,"
                     "channel";

    m_netwManager = new QNetworkAccessManager(this);
    QNetworkDiskCache* diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(_cache_path);
    m_netwManager->setCache(diskCache);
    connect(m_netwManager,&QNetworkAccessManager::finished,[=](QNetworkReply* rep){
        if(rep->error() == QNetworkReply::NoError){
            QByteArray response = rep->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            if(!jsonDoc.isEmpty()){
                QString reqUrlStr = rep->request().url().toString();
                QString ci_id = QString(QCryptographicHash::hash((reqUrlStr.toUtf8()),QCryptographicHash::Md5).toHex());
                utils::saveJson(jsonDoc,utils::returnPath("store_cache")+ci_id);
            }
            processResponse(response);
        }else{
            downloadError(rep->errorString());
        }
        rep->deleteLater();
    });
}

void Store::cancelAllRequests()
{
    foreach (auto &reply, m_netwManager->findChildren<QNetworkReply *>()) {
        if (! reply->isReadable()) {
                return;
        }
        reply->abort();
        reply->deleteLater();
    }
}

void Store::loadCategory(const QString catName)
{
    QUrl reqUrl(api_end_point+"v2/snaps/find");
    QUrlQuery query;
    query.addQueryItem("category",catName);
    query.addQueryItem("fields",fields);
    reqUrl.setQuery(query);
    get(reqUrl);
}

void Store::getCategories()
{
//    We are forcing to load from local resources for now.
//    QUrl reqUrl(api_end_point+"v2/snaps/categories");
    QUrl reqUrl("qrc:/resources/categories.json");
    Request *req = new Request(this);
    connect(req,&Request::requestFinished,[=](QString replyStr){
        req->deleteLater();
        emit gotCategories(replyStr);
    });
//    connect(req,&Request::downloadError,[=](){
//         QUrl localCatUrl("qrc:/resources/categories.json");
//         req->get(localCatUrl);
//    });
    req->get(reqUrl);
}

void Store::search(QString search_term)
{
    QUrl reqUrl(api_end_point+"v2/snaps/find");
    QUrlQuery query;
    query.addQueryItem("q",search_term);
    query.addQueryItem("fields",fields);
    reqUrl.setQuery(query);
    get(reqUrl);
}

void Store::get(const QUrl url)
{
    QString reqUrlStr = url.toString();
    QString ci_id = QString(QCryptographicHash::hash((reqUrlStr.toUtf8()),QCryptographicHash::Md5).toHex());
    QFileInfo cFile(utils::returnPath("store_cache")+ci_id);
    // 2 days expiry
    bool cacheExpired = (QDateTime::currentMSecsSinceEpoch()/1000 - cFile.lastModified().toMSecsSinceEpoch()/1000) > 172800 ? true : false;
    if(!cacheExpired && cFile.isFile() && cFile.exists() && cFile.size()!=0){
        processResponse(utils::loadJson(cFile.filePath()).toJson());
        emit loadedFromCache(cFile.absoluteFilePath());
        qDebug()<<"data loaded from local cache";
    }else{
        QNetworkRequest request(url);
        request.setRawHeader("Snap-Device-Series","16");
        m_netwManager->get(request);
    }
}

void Store::downloadProgress(qint64 got,qint64 tot)
{
    double downloaded_Size  = (double)got;
    double total_Size       = (double)tot;
    double progress         = (downloaded_Size/total_Size) * 100;
    int intProgress         = static_cast<int>(progress);
    qDebug()<<intProgress;
}

void Store::downloadError(QString errorString)
{
    emit error(errorString);
}

void Store::processResponse(QByteArray data)
{
    QString dataString = QTextCodec::codecForMib(106)->toUnicode(data);
    emit response(dataString);
}
