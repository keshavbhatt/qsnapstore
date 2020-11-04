#ifndef REQUEST_H
#define REQUEST_H

#include <QtNetwork>
#include <QObject>

class Request : public QObject
{
    Q_OBJECT

public:
    Request(QObject* parent=0);
    virtual ~Request();
public slots:
    void get(const QUrl url,bool preferCache = true);
    void clearCache(QUrl url);
    void cancelAll();
signals:
   void requestFinished(QString rep);
   void requestStarted();
   void downloadFinished(QNetworkReply &rep);
   void downloadStarted();
   void _downloadProgress(int progress);
   void downloadError(QString errorString);

private slots:
   void downloadProgress(qint64 got, qint64 tot);
private:
   QString _cache_path;
   QNetworkAccessManager *m_netwManager = nullptr;
   QList<QNetworkReply*>operations;
};

#endif // REQUEST_H
