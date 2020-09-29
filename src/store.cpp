#include "store.h"
#include "request.h"

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
            processResponse(response);
        }else{
            downloadError(rep->errorString());
        }
        rep->deleteLater();
    });
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
    QUrl reqUrl(api_end_point+"v2/snaps/categories");
    Request *req = new Request(this);
    connect(req,&Request::requestFinished,[=](QString replyStr){
        req->deleteLater();
        emit gotCategories(replyStr);
    });
    connect(req,&Request::downloadError,[=](){
         QUrl localCatUrl("qrc:/resources/categories.json");
         req->get(localCatUrl);
    });
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
    QNetworkRequest request(url);
    request.setRawHeader("Snap-Device-Series","16");
    m_netwManager->get(request);
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
