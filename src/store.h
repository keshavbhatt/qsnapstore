#ifndef STORE_H
#define STORE_H

#include <QObject>
#include <QtNetwork>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QDebug>


class Store : public QObject
{
    Q_OBJECT
public:
    explicit Store(QObject *parent = nullptr);

signals:
    void response(QString dataString);
    void error(QString errorStrting);
    void gotCategories(QString replyStr);
    void loadedFromCache(QString cFilePath);

public slots:
    void search(QString search_term);
    void loadCategory(const QString catName);
    void getCategories();
    void cancelAllRequests();
private slots:
    void get(const QUrl url);
    void downloadProgress(qint64 got, qint64 tot);
    void downloadError(QString errorString);
    void processResponse(QByteArray data);
private:
    QString api_end_point,_cache_path,fields;
    QNetworkAccessManager * m_netwManager = nullptr;
};

#endif // STORE_H
