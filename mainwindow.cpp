#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QClipboard>
#include <QComboBox>
#include <QDropEvent>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHeaderView>
#include <QIcon>
#include <QLineEdit>
#include <QMimeData>
#include <QMessageBox>
#include <QStatusBar>
#include <QStandardPaths>
#include <QTableWidgetItem>
#include <QUrl>

#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/icons/CSV.svg"));
    setWindowTitle("CSV Worker");

    doOperation(INITIALIZE, &context, nullptr);
    setupTable();
    setupColumnComboBox();
    setupRegionComboBox();
    setupConnections();
    setupDragAndDrop();
    setLoadedState();
    clearMetricFields();
    statusBar()->showMessage(statusText(context.status), STATUS_BAR_MESSAGE_TIMEOUT_MS);
}

MainWindow::~MainWindow()
{
    doOperation(DISPOSE, &context, nullptr);
    delete ui;
}

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

void MainWindow::setupConnections() {
    connect(ui->chooseFileButton, &QPushButton::clicked, this, &MainWindow::chooseFileClicked);
    connect(ui->calculateMetricsButton, &QPushButton::clicked, this, &MainWindow::calculateMetricsClicked);
    connect(ui->regionComboBox, QOverload<int>::of(&QComboBox::activated),this, &MainWindow::regionEditingFinished);
    connect(ui->regionComboBox->lineEdit(), &QLineEdit::editingFinished,this, &MainWindow::regionEditingFinished);
    connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &MainWindow::tableItemDoubleClicked);
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

void MainWindow::setupTable() {
    ui->tableWidget->setColumnCount(COLUMN_COUNT);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setHorizontalHeaderLabels(
        {"Year", "Region", "Growth", "Birth", "Death", "Weight", "Urban"});
}

void MainWindow::setupColumnComboBox() {
    ui->columnComboBox->clear();
    ui->columnComboBox->addItem("Year", YEAR);
    ui->columnComboBox->addItem("Growth", NPG);
    ui->columnComboBox->addItem("Birth", BIRTH_RATE);
    ui->columnComboBox->addItem("Death", DEATH_RATE);
    ui->columnComboBox->addItem("Weight", GDW);
    ui->columnComboBox->addItem("Urban", URBANIZATION);
    ui->columnComboBox->setCurrentIndex(0);
}

void MainWindow::setupRegionComboBox() {
    ui->regionComboBox->clear();
    ui->regionComboBox->addItem("");
    ui->regionComboBox->setCurrentIndex(0);
    ui->regionComboBox->lineEdit()->installEventFilter(this);
}

void MainWindow::reloadRegionComboBox() {
    Iterator it;

    setupRegionComboBox();
    if (context.list != nullptr) {
        it = begin(context.list);
        while (isSet(&it)) {
            DemographyRecord* record = (DemographyRecord*)get(&it);
            if (record != nullptr) {
                QString region = QString::fromLocal8Bit(record->region).trimmed();
                int foundIndex = -1;

                for (int i = 0; i < ui->regionComboBox->count(); i++) {
                    if (ui->regionComboBox->itemText(i).compare(region, Qt::CaseInsensitive) == 0) {
                        foundIndex = i;
                        break;
                    }
                }

                if (!region.isEmpty() && foundIndex < 0)
                    ui->regionComboBox->addItem(region);
            }
            next(&it);
        }
    }
}

void MainWindow::selectFile(const QString& filePath) {
    if (!filePath.isEmpty()) {
        unloadData();
        ui->filePathLineEdit->setText(filePath);
        loadDataClicked();
    }
}

void MainWindow::unloadData() {
    if (hasLoadedData()) {
        doOperation(DISPOSE, &context, nullptr);
        doOperation(INITIALIZE, &context, nullptr);
    }

    reloadRegionComboBox();
    setLoadedState();
    clearMetricFields();
}


Column MainWindow::selectedColumn() const {
    int selectedIndex = ui->columnComboBox->currentIndex();
    return selectedIndex >= 0 ? static_cast<Column>(ui->columnComboBox->itemData(selectedIndex).toInt()): YEAR;
}

int MainWindow::hasLoadedData() const {
    return (context.list != nullptr && context.list->size > 0);
}

void MainWindow::setLoadedState() {
    int isLoaded = hasLoadedData();
    ui->calculateMetricsButton->setEnabled(isLoaded);
    ui->contentStackedWidget->setCurrentWidget(isLoaded ? ui->tablePage : ui->emptyPage);
    if (!isLoaded)
        ui->tableWidget->setRowCount(0);
}

void MainWindow::clearMetricFields() {
    ui->minValueLineEdit->clear();
    ui->medianValueLineEdit->clear();
    ui->maxValueLineEdit->clear();
}

QString MainWindow::statusText(Status status) const {
    QString text;

    switch (status) {
    case OK:
        text = "OK";
        break;
    case ERR_FILE_OPEN:
        text = "Cannot open file";
        break;
    case ERR_INVALID_HEADER:
        text = "Invalid CSV header";
        break;
    case ERR_MEMORY:
        text = "Memory allocation error";
        break;
    case ERR_EMPTY_DATA:
        text = "No valid data found";
        break;
    case ERR_INVALID_REGION:
        text = "Region is not found";
        break;
    case ERR_INVALID_COLUMN:
        text = "Invalid column for metrics";
        break;
    default:
        text = "Unknown error";
        break;
    }

    return text;
}

void MainWindow::showLoadSummary() {
    size_t totalRows = context.parseInfo.accepted + context.parseInfo.rejected;
    QString summaryHtml = QString(
                              "<div style='font-size: 20px; font-weight: 700; line-height: 1.45;'>"
                              "<div style='color: #111111;'>Total rows: %1</div>"
                              "<div style='color: #1f8f3a;'>Valid rows: %2</div>"
                              "<div style='color: #c73535;'>Invalid rows: %3</div>"
                              "</div>")
                          .arg(totalRows)
                          .arg(context.parseInfo.accepted)
                          .arg(context.parseInfo.rejected);
    QMessageBox messageBox(this);
    QIcon csvIcon(":/icons/icons/CSV.svg");
    QIcon infoIcon(":/icons/icons/info.svg");
    messageBox.setWindowIcon(csvIcon);
    messageBox.setIconPixmap(infoIcon.pixmap(44, 44));
    messageBox.setWindowTitle("Load Result");
    messageBox.setText(summaryHtml);
    messageBox.resize(580, 320);
    messageBox.setStyleSheet(
        "QPushButton { background: #ffffff; color: #2c3f59; border: 1px solid #93a8c6; border-radius: 10px; padding: 8px 18px; min-width: 96px; font-weight: 600; }"
        "QPushButton:hover { background: #f5f9ff; }");
    messageBox.exec();
}


void MainWindow::fillTable(const QString& regionFilter) {
    int isRegionEmpty = regionFilter.trimmed().isEmpty();
    int isRegionFound = 0;
    int row = 0;
    Iterator it;

    ui->tableWidget->setRowCount(0);
    if (context.list == nullptr)
        return;

    it = begin(context.list);
    while (isSet(&it)) {
        DemographyRecord* record = (DemographyRecord*)get(&it);
        if (record != nullptr) {
            QString recordRegion = QString::fromLocal8Bit(record->region);
            if (isRegionEmpty || !recordRegion.compare(regionFilter, Qt::CaseInsensitive)) {
                if (!isRegionEmpty)
                    isRegionFound = 1;
                ui->tableWidget->insertRow(row);
                fillTableRow(row, record, recordRegion);
                row++;
            }
        }
        next(&it);
    }
    if (!isRegionEmpty && !isRegionFound)
        statusBar()->showMessage("Region is not found in loaded data", STATUS_BAR_MESSAGE_TIMEOUT_MS);
}

void MainWindow::fillTableRow(int row, const DemographyRecord* record, const QString& recordRegion) {
    ui->tableWidget->setItem(row, YEAR, new QTableWidgetItem(QString::number(record->year)));
    ui->tableWidget->setItem(row, REGION, new QTableWidgetItem(recordRegion));
    ui->tableWidget->setItem(row, NPG, new QTableWidgetItem(QString::number(record->naturalPopulationGrowth)));
    ui->tableWidget->setItem(row, BIRTH_RATE, new QTableWidgetItem(QString::number(record->birthRate)));
    ui->tableWidget->setItem(row, DEATH_RATE, new QTableWidgetItem(QString::number(record->deathRate)));
    ui->tableWidget->setItem(row, GDW, new QTableWidgetItem(QString::number(record->generalDemographicWeight)));
    ui->tableWidget->setItem(row, URBANIZATION, new QTableWidgetItem(QString::number(record->urbanization)));
}

void MainWindow::chooseFileClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select CSV file",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        "CSV Files (*.csv);;All Files (*)");

    selectFile(fileName);
}

void MainWindow::loadDataClicked() {
    AppParams params;
    std::string filePath = ui->filePathLineEdit->text().toStdString();

    params.str = filePath.c_str();
    params.column = YEAR;

    doOperation(LOAD_DATA, &context, &params);
    statusBar()->showMessage(statusText(context.status), STATUS_BAR_MESSAGE_TIMEOUT_MS);
    showLoadSummary();
    reloadRegionComboBox();
    setLoadedState();

    if (context.status == OK)
        fillTable(ui->regionComboBox->currentText().trimmed());
    else
        clearMetricFields();

}

void MainWindow::calculateMetricsClicked() {
    AppParams params;
    std::string region = ui->regionComboBox->currentText().trimmed().toStdString();

    params.str = region.c_str();
    params.column = selectedColumn();

    doOperation(CALCULATE_METRICS, &context, &params);
    statusBar()->showMessage(statusText(context.status), STATUS_BAR_MESSAGE_TIMEOUT_MS);

    if (context.status == OK) {
        ui->minValueLineEdit->setText(QString::number(context.metrics.min));
        ui->medianValueLineEdit->setText(QString::number(context.metrics.median));
        ui->maxValueLineEdit->setText(QString::number(context.metrics.max));
    } else
        clearMetricFields();
}

void MainWindow::regionEditingFinished() {
    if (hasLoadedData())
        fillTable(ui->regionComboBox->currentText().trimmed());
}

void MainWindow::tableItemDoubleClicked(QTableWidgetItem *item) {
    if (item && item->column() == REGION)
        QGuiApplication::clipboard()->setText(item->text());
}

