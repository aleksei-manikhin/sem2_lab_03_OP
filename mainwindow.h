#ifndef MAINWINDOW_H
#define MAINWINDOW_H

extern "C" {
#include "entrypoint.h"
#include "iterator.h"
#include "appcontext.h"
}

#include <QMainWindow>
#include <QString>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QEvent;
class QMimeData;
class QDropEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void chooseFileClicked();
    void loadDataClicked();
    void calculateMetricsClicked();
    void regionEditingFinished();
    void tableItemDoubleClicked(QTableWidgetItem *item);

    Ui::MainWindow *ui;
    AppContext context;

    void setupConnections();
    void setupDragAndDrop();
    void setupTable();
    void setupColumnComboBox();
    void setupRegionComboBox();
    void reloadRegionComboBox();
    int isDropWidget(const QObject* watched) const;
    int handleDragDropEvent(const QObject* watched, QEvent* event);
    int acceptDropEvent(QDropEvent* dropEvent, int shouldSelectFile);
    void setDropHintVisible(int isVisible);
    void selectFile(const QString& filePath);
    void unloadData();
    QString droppedFilePath(const QMimeData* mimeData) const;
    void setLoadedState();
    int hasLoadedData() const;
    Column selectedColumn() const;
    void fillTable(const QString& regionFilter);
    void fillTableRow(int row, const DemographyRecord* record, const QString& recordRegion);
    void clearMetricFields();
    void showLoadSummary();
    QString statusText(Status status) const;
};

#endif // MAINWINDOW_H
