#ifndef MD2HTML_H
#define MD2HTML_H

#include <QObject>
#include <QDebug>
#include <QRegularExpression>
#include <QSettings>

#include "helpers/codetohtmlconverter.h"
#include "libraries/md4c/md2html/render_html.h"
#include "libraries/md4c/md4c/md4c.h"


class Md2Html : public QObject
{
    Q_OBJECT
public:
    explicit Md2Html(QObject *parent = nullptr);

signals:

public slots:
    QString toHtml(QString str);
};

#endif // MD2HTML_H
