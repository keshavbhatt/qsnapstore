#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsOpacityEffect>
#include <QSettings>
#include <QStyleFactory>
#include <QListWidgetItem>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSplitter>
#include <QScreen>

#include "store.h"
#include "widgets/waitingspinnerwidget.h"
#include "error.h"
#include "utils.h"
#include "remotepixmaplabel.h"
#include "md2html.h"

#include "ui_track.h"
#include "ui_wallthumb.h"
#include "ui_action_ui.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);
private slots:
    void on_queryEdit_textChanged(const QString &arg1);

    void on_searchBtn_clicked();

    void processResponse(QString reply);
    void on_queryEdit_returnPressed();

    void showError(QString message);
    void setStatus(QString message);
    void load_data_into_view(QList<QVariantList> data, bool appending);
    void setStyle(QString fname);
    void on_status_linkActivated(const QString &link);
    void append_more_results(int index);

    void on_results_currentRowChanged(int currentRow);

    void showItemDetail(QStringList item_meta);
    void updateNavigationButtons();
    void on_left_clicked();
    void on_right_clicked();
    void on_wallpaperList_currentRowChanged(int currentRow);
    void on_wallpaperList_itemClicked(QListWidgetItem *item);
    void on_homeBtn_clicked();

    void processCategories(QString reply);
    void on_categoryCombo_currentIndexChanged(int index);
    void loadCategory(const QString catName);
    QVariantList itemInfo(const QJsonObject object);
    void setAppButtons(QStringList item_meta);
    void on_description_anchorClicked(const QUrl &arg1);

    void on_meta_anchorClicked(const QUrl &arg1);

    void addToHistory(const QString arg1);
    void on_backBtn_clicked();

    void on_forwardBtn_clicked();
private:
    Ui::MainWindow *ui;
    Ui::track track_ui;
    Ui::thumbWidget _ui_listitem;
    Ui::actionWidget _ui_action;
    Store *m_store = nullptr;
    WaitingSpinnerWidget *_loader = nullptr;
    WaitingSpinnerWidget *wall_loader = nullptr;

    Error *_error = nullptr;
    RemotePixmapLabel *_wall_view = nullptr;


    QString currentResultData;
    QList<QString> elements;
    int currentResultCount;
    bool stopLoadingResults = false;
    double trackCoverWidth = 0;
    QSize trackWidgetSizeHint;
    QString cache_path;

    QString currentWallInfo = "";
    QString _currentUrl;

    QSettings settings;

    Md2Html *htmlParser = nullptr;

};

#endif // MAINWINDOW_H
