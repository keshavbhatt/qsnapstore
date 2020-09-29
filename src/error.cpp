#include "error.h"
#include "ui_error.h"

Error::Error(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Error)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    ui->ok->setShortcut(QKeySequence("Return"));
}

void Error::setError(QString shortMessage,QString detail)
{
    ui->detail->setText(detail);
    ui->message->setText(shortMessage);
    ui->ok->setFocus();
}


Error::~Error()
{
    delete ui;
}

void Error::on_ok_clicked()
{
    this->close();
}
