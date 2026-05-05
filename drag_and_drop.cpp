#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QComboBox>
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QLineEdit>
#include <QMimeData>
#include <QTableWidget>
#include <QUrl>
#include <QWidget>

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    int isHandled = 0;

    if (event != nullptr) {
        if (event->type() == QEvent::MouseButtonPress) {
            if (ui->regionComboBox != nullptr && watched == ui->regionComboBox->lineEdit())
                ui->regionComboBox->showPopup();
        } else if (isDropWidget(watched) || watched == ui->centralwidget)
            isHandled = handleDragDropEvent(watched, event);
    }

    return isHandled;
}

int MainWindow::isDropWidget(const QObject* watched) const {
    return watched == ui->contentStackedWidget
           || watched == ui->emptyPage
           || watched == ui->tablePage
           || watched == ui->tableWidget
           || watched == ui->tableWidget->viewport()
           || watched == ui->emptyPageText
           || watched == ui->icon_4;
}

int MainWindow::handleDragDropEvent(const QObject* watched, QEvent* event) {
    int isHandled = 0;
    int isTableArea = isDropWidget(watched);

    if (event != nullptr) {
        if (event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove) {
            if (isTableArea && (isHandled = acceptDropEvent((QDropEvent*)event, 0)))
                setDropHintVisible(1);
            else {
                setDropHintVisible(0);
                ((QDropEvent*)event)->ignore();
                isHandled = 1;
            }
        } else if (event->type() == QEvent::Drop) {
            if (isTableArea && (isHandled = acceptDropEvent((QDropEvent*)event, 1)))
                setDropHintVisible(0);
            else {
                ((QDropEvent*)event)->ignore();
                isHandled = 1;
            }
        }
    }
    return isHandled;
}

int MainWindow::acceptDropEvent(QDropEvent* dropEvent, int shouldSelectFile) {
    int isHandled = 0;

    if (dropEvent != nullptr) {
        QString filePath = droppedFilePath(dropEvent->mimeData());
        if (!filePath.isEmpty()) {
            if (shouldSelectFile)
                selectFile(filePath);
            dropEvent->acceptProposedAction();
            isHandled = 1;
        }
    }
    return isHandled;
}

void MainWindow::setDropHintVisible(int isVisible) {
    if (hasLoadedData()) {
        QWidget* page = isVisible ? ui->emptyPage : ui->tablePage;
        if (ui->contentStackedWidget->currentWidget() != page)
            ui->contentStackedWidget->setCurrentWidget(page);
    }
}

QString MainWindow::droppedFilePath(const QMimeData* mimeData) const {
    QString filePath;

    if (mimeData != nullptr && mimeData->hasUrls()) {
        const QList<QUrl> urls = mimeData->urls();
        for (const QUrl& url : urls) {
            if (url.isLocalFile()) {
                QString droppedPath = url.toLocalFile();
                QFileInfo fileInfo(droppedPath);
                if (fileInfo.exists() && fileInfo.isFile()) {
                    filePath = droppedPath;
                    break;
                }
            }
        }
    }
    return filePath;
}

void MainWindow::setupDragAndDrop() {
    ui->centralwidget->setAcceptDrops(true);
    ui->tableWidget->viewport()->setAcceptDrops(true);

    ui->centralwidget->installEventFilter(this);
    ui->contentStackedWidget->installEventFilter(this);
    ui->emptyPage->installEventFilter(this);
    ui->tablePage->installEventFilter(this);
    ui->tableWidget->installEventFilter(this);
    ui->tableWidget->viewport()->installEventFilter(this);
    ui->emptyPageText->installEventFilter(this);
    ui->icon_4->installEventFilter(this);
}
