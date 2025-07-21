#include "processtext.h"
#include "ui_processtext.h"

ProcessText::ProcessText(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ProcessText)
    , undo_stack(new QUndoStack(this))
    ,content_dialog(new Content(this))
{
    ui->setupUi(this);
    // 菜单栏
    // 文件菜单
    connect(ui->openaction, &QAction::triggered, this, &ProcessText::openFileMenu);
    connect(ui->saveaction, &QAction::triggered, this, &ProcessText::saveFileMenu);
    connect(ui->undoaction, &QAction::triggered, undo_stack, &QUndoStack::undo);
    connect(ui->redoaction, &QAction::triggered, undo_stack, &QUndoStack::redo);
    connect(ui->repeataction, &QAction::triggered, this, &ProcessText::repeatLastAction);

    // QTabWidget
    // 按钮
    // 添加
    connect(ui->addContentBeforeButton, &QPushButton::clicked, this, &ProcessText::addContentBeform);
    connect(ui->addQuotaionMarkButton, &QPushButton::clicked, this, &ProcessText::addQuotation);
    connect(ui->addCommaButton, &QPushButton::clicked, this, &ProcessText::addLastComma);

    // 修改
    connect(ui->replaceCNtoENsymbolButton, &QPushButton::clicked, this, &ProcessText::replaceCNtoENsymbol);
    connect(ui->replaceCaseButton, &QPushButton::clicked, this, &ProcessText::convertCase);
    connect(ui->setUpSequenceButton, &QPushButton::clicked, this, &ProcessText::setUpSequence);
    connect(ui->setDownSequenceButton, &QPushButton::clicked, this, &ProcessText::setDownSequence);
    connect(ui->replaceSlashButton, &QPushButton::clicked, this, &ProcessText::replaceSlash);
    connect(ui->eraseCommentSymbolButton, &QPushButton::clicked, this, &ProcessText::removeAnnotators);
    connect(ui->simplifyTextButton, &QPushButton::clicked, this, &ProcessText::simplifyText);

    // 提取
    connect(ui->filterSpecialCharsButton, &QPushButton::clicked, this, &ProcessText::filterSpecialChars);
    connect(ui->extractEqualContentBotton, &QPushButton::clicked, this, &ProcessText::extractEqualContent);
    connect(ui->extractCNValueButton, &QPushButton::clicked, this, &ProcessText::extractCNValue);
    // 网站
    connect(ui->getHTTPvalueButton, &QPushButton::clicked, this, &ProcessText::extractWebsite);
    connect(ui->getEqualDomainButton, &QPushButton::clicked, this, &ProcessText::getEqualDomain);
    connect(ui->getDomainButton, &QPushButton::clicked, this, &ProcessText::getDomain);

    // 网络
    connect(ui->filterIPv4Button, &QPushButton::clicked, this, &ProcessText::filterIPv4);
}

ProcessText::~ProcessText()
{
    delete ui;
    delete undo_stack;
    delete content_dialog;
}

// 文本的撤销与重做
class TextDispose : public QUndoCommand {
public:
    TextDispose(QTextEdit *text_edit, const QString &prev_text, const QString &new_text, QUndoCommand *parent = nullptr)
        : QUndoCommand(parent), text_edit(text_edit), prev_text(prev_text), new_text(new_text) {}

    void undo() override {
        text_edit->setPlainText(prev_text);
    }

    void redo() override {
        text_edit->setPlainText(new_text);
    }

private:
    QTextEdit *text_edit;
    QString prev_text;
    QString new_text;
};

// 打开文件菜单栏
void ProcessText::openFileMenu() {
    QString file_path = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "",
        tr("*.txt *.md *.cpp *.java *.python")
        );

    if (!file_path.isEmpty()) {
        ui->lineEdit->setText(file_path);
    } else {
        return;
    }

    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { // 打开文件并检查成功
        qWarning() << "无法打开文件:" << file.errorString(); // 输出错误信息
        return; // 如果无法打开文件，退出函数
    }

    QTextStream in(&file);
    QString line;
    while (!in.atEnd()) {
        line += in.readLine() + "\n";
    }

    ui->textEdit->setText(line);
    file.close();
}

// 打开文件夹
void ProcessText::on_openfolderaction_triggered()
{
    QString folder_path = QFileDialog::getExistingDirectory(
        nullptr,
        "Select Folder",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly
        );

    if (!folder_path.isEmpty()) {
        qDebug() << "Seleted folder: " << folder_path;
    } else {
        qDebug() << "No folder seleted.";
    }

    ui->lineEdit->setText(folder_path);
}

// 判断文件夹是否存在,逻辑基本完成
int ProcessText::judgeFolder() {
    const QString file_name = ui->lineEdit->text();

    if (file_name.isEmpty()) {
        return 0;
    }

    QFileInfo path_info(file_name);

    if (!path_info.exists()) {
        qDebug() << "The path does not exist.";
        return 0;
    }
    else if (path_info.isDir()) {
        qDebug() << "is dir";
        return 1;
    }

    return 0;
}

// 判断文件是否存在,逻辑基本完成
int ProcessText::judgeFile() {
    const QString file_name = ui->lineEdit->text();

    if (file_name.isEmpty()) {
        return 0;
    }

    QFileInfo path_info(file_name);

    if (!path_info.exists()) {
        qDebug() << "The path does not exist.";
        return 0;
    }
    else if (path_info.isFile()) {
        qDebug() << "is dir";
        return 1;
    }

    return 0;
}

// 判断按钮是否隐藏
void ProcessText::estimateVisible(QWidget *widget) {
    if (widget->isVisible()) {
        widget->hide();
    } else {
        widget->show();
    }
}

bool ProcessText::judgeTextExist() {
    QString text = ui->textEdit->toPlainText();
    if (text.isEmpty()) {
        return false;
    } else {
        return true;
    }
}

// 处理文件夹的文件
void ProcessText::processFolder(const std::function<void(QTextStream &in, QTextStream &out)> &processLine) {
    bool value = judgeFolder();

    if (!value) {
        return;
    }

    // 筛选文件
    QDir dir(ui->lineEdit->text());
    QStringList filters;
    filters << "*.txt";
    QFileInfoList file_list = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo &file_info : file_list) {
        QString file_path = file_info.absoluteFilePath();

        QFile input_file(file_path);
        if (!input_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Faild to open file for reading:" << file_path;
            continue;
        }

        QString temp_file_path = file_path + ".tmp";
        QFile temp_file(temp_file_path);
        if (!temp_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Faild to open temp file for writing:" << temp_file_path;
            input_file.close();
            continue;
        }

        QTextStream in(&input_file);
        QTextStream out(&temp_file);

        // 调用传递的处理逻辑
        processLine(in, out);

        input_file.close();
        temp_file.close();

        // 替换原文件
        if (QFile::remove(file_path)) {
            if (QFile::rename(temp_file_path, file_path)) {
                qDebug() << "File processed successfully:" << file_path;
            } else {
                qDebug() << "Failed to rename temp file.";
            }
        } else {
            qDebug() << "Failed to remove original file.";
        }
    }
}

// 获得文本,处理文本
void ProcessText::processText(const std::function<QString(QString &)> &lineProcessor)
{
    QString text = ui->textEdit->toPlainText();
    QString prev_text = text;
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);

    for (int i = 0; i < lines.size(); i++) {
        lines[i] = lineProcessor(lines[i]);
    }

    text = lines.join("\n");
    QString new_text = text;

    undo_stack->push(new TextDispose(ui->textEdit, prev_text, new_text));

    ui->textEdit->setPlainText(text);
}

// 保存到文件
void ProcessText::saveFileMenu() {
    QString file_path = ui->lineEdit->text();
    if (file_path.isEmpty()) {
        file_path = QFileDialog::getSaveFileName(this, tr("Save file"), "", tr("*.*"));
        ui->lineEdit->setText(file_path);
    }

    QFile file(file_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) { // 以写入模式打开文件
        qWarning() << "无法打开文件:" << file.errorString(); // 输出错误信息
        return;
    }

    QTextStream out(&file);
    out << ui->textEdit->toPlainText();

    file.close();
}

// 关闭程序,这个不需要修改
void ProcessText::on_closeaction_triggered()
{
    close();
}

// 执行上一个操作,逻辑基本完成
void ProcessText::repeatLastAction() {
    if (last_action) {
        last_action();
    } else {
        QMessageBox::warning(this, tr("Waring"), tr("No action"));
    }
}

void ProcessText::keyPressEvent(QKeyEvent *event)
{
    // 检测 Ctrl + Tab
    if (event->key() == Qt::Key_Tab && event->modifiers() == Qt::ControlModifier) {
        int currentIndex = ui->tabWidget->currentIndex();
        int nextIndex = (currentIndex + 1) % ui->tabWidget->count();
        ui->tabWidget->setCurrentIndex(nextIndex);
    }
    // 检测 Ctrl + Shift + Tab
    else if (event->key() == Qt::Key_Backtab && event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
        int currentIndex = ui->tabWidget->currentIndex();
        int previousIndex = (currentIndex - 1 + ui->tabWidget->count()) % ui->tabWidget->count();
        ui->tabWidget->setCurrentIndex(previousIndex);
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

// 添加
// 在文本前添加内容
void ProcessText::addContentBeform() {
    QString text = ui->textEdit->toPlainText();
    QString perv_text = text;
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);

    QString content_text;
    Content get_content;

    if (content_dialog->exec() == QDialog::Accepted) {
        content_text = content_dialog->confirmContent();

        qDebug() << get_content.insertPosition();

        if (get_content.insertPosition() == "before") {
            for (int i = 0; i < lines.size(); i++) {
                lines[i] = content_text + lines[i];
            }
        } else if (get_content.insertPosition() == "back") {
            for (int i = 0; i < lines.size(); i++) {
                lines[i] = lines[i] + content_text;
            }
        } else if (get_content.insertPosition() == "both end") {
            for (int i = 0; i < lines.size(); i++) {
                lines[i] = content_text + lines[i] + content_text;
            }
        }
    }

    text = lines.join("\n");
    QString new_text = text;

    undo_stack->push(new TextDispose(ui->textEdit, perv_text, new_text));
    ui->textEdit->setPlainText(new_text);

    last_action = [this]() { addContentBeform(); };
}

// 添加引号按钮
void ProcessText::addQuotation() {
    int value = judgeFolder();

    if (value == 1) {
        processFolder([](QTextStream &in,QTextStream &out) {
            while (!in.atEnd()) {
                QString line = in.readLine();
                out << "\"" << line << "\"" << "\n";
            }
        });
    }

    if (judgeTextExist()) {
        processText([](const QString &line) {
            return "\"" + line + "\"";
        });
    }

    last_action = [this]() { addQuotation(); };
}

// 末尾添加逗号
void ProcessText::addLastComma() {
    int value = judgeFolder();

    if (value == 1) {
        processFolder([](QTextStream &in,QTextStream &out) {
            while (!in.atEnd()) {
                QString line = in.readLine();
                out << line << ",";
            }
        });
    }

    if (judgeTextExist()){
        processText([](const QString &line) {
            return line + ",";
        });
    }

    last_action = [this]() { addLastComma(); };
}

// 修改
// 将中文符号替换成英文按钮
void ProcessText::replaceCNtoENsymbol() {
    int value = judgeFolder();

    if (value == 1) {
        processFolder([](QTextStream &in,QTextStream &out) {
            while (!in.atEnd()) {
                QString line = in.readLine();
                line.replace(QChar(0xff08), QChar('('));
                line.replace(QChar(0xff09), QChar(')'));
                line.replace(QChar(0x3002), QChar('.'));
                line.replace(QChar(0xff1a), QChar(':'));
                line.replace(QChar(0xff0c), QChar(','));
                out << line <<"\n";
            };
        });
    }

    if (judgeTextExist()) {
        processText([](QString &line) {
            line.replace(QChar(0xff08), QChar('('));
            line.replace(QChar(0xff09), QChar(')'));
            line.replace(QChar(0x3002), QChar('.'));
            line.replace(QChar(0xff1a), QChar(':'));
            line.replace(QChar(0xff0c), QChar(','));
            return line;
        });
    }

    last_action = [this]() { replaceCNtoENsymbol(); };
}

// 去除注释符
void ProcessText::removeAnnotators() {
    int value = judgeFolder();

    if (value == 1) {
        processFolder([](QTextStream &in,QTextStream &out){
            QString line = in.readLine();
            int index = line.indexOf("//");
            if (index != -1) {
                line = line.left(index) + line.mid(index + 2); // 去掉注释
            }
            out << line << "\n";
        });
    }

    if (judgeTextExist()) {
        processText([](QString &line) {
            int index = line.indexOf("//");
            if (index != -1) {
                line = line.left(index) + line.mid(index + 2); // 去掉注释
            }
            return line.trimmed();
        });
    }

    last_action = [this]() { removeAnnotators(); };
}

// 替换大小写字母
void ProcessText::convertCase() {
    int value = judgeFolder();

    if (value == 1) {
        processFolder([](QTextStream &in,QTextStream &out) {
            while (!in.atEnd()) {
                QString line = in.readLine();
                for (int i = 0; i < line.size(); i++) {
                    if (line[i].isLower()) {
                        line[i] = line[i].toUpper();
                    } else if (line[i].isUpper()) {
                        line[i] = line[i].toLower();
                    }
                }
                out << line << "\n";
            };
        });
    }

    if (judgeTextExist()) {
        processText([](QString &line) {
            for (int i = 0; i < line.size(); i++) {
                if (line[i].isLower()) {
                    line[i] = line[i].toUpper();
                } else if (line[i].isUpper()) {
                    line[i] = line[i].toLower();
                }
            }
            return line;
        });
    }

    last_action = [this]() { convertCase(); };
}

// 将文本排序
void ProcessText::setUpSequence() {
    QString text = ui->textEdit->toPlainText();
    QString perv_text = text;
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);

    std::sort(lines.begin(), lines.end(), [](const QString &a, const QString &b) {
        return a.toLower() < b.toLower(); // 忽略大小写排序
    });

    text = lines.join("\n");
    QString new_text = text;
    undo_stack->push(new TextDispose(ui->textEdit, perv_text, new_text));
    ui->textEdit->setText(text);

    last_action = [this]() { setUpSequence(); };
}

void ProcessText::setDownSequence() {
    QString text = ui->textEdit->toPlainText();
    QString perv_text = text;
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);

    std::sort(lines.begin(), lines.end(), [](const QString &a, const QString &b) {
        return a.toLower() > b.toLower(); // 忽略大小写排序
    });

    text = lines.join("\n");
    QString new_text = text;
    undo_stack->push(new TextDispose(ui->textEdit, perv_text, new_text));
    ui->textEdit->setText(text);

    last_action = [this]() { setDownSequence(); };
}

// 繁简替换
void ProcessText::simplifyText() {
    int value = judgeFolder();
    if (value == 1) {
        QString path = ui->lineEdit->text();
        QString program = "./data/opencc_batch.exe";
        QStringList arguments;
        arguments << "--reverse" << path;

        QProcess process;
        process.start(program, arguments);
        process.waitForFinished();
        QString output = process.readAllStandardOutput();
        qDebug() << output;
    }
}

// 提取
// 过滤特殊符号
void ProcessText::filterSpecialChars() {
    QString text = ui->textEdit->toPlainText();
    QString prev_text = text;
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);

    QString new_text;

    // 保留：中文、英文、数字、空格
    QRegularExpression clean_regex(R"([^a-zA-Z0-9])"); // 过滤所有特殊符号

    for (QString line : lines) {
        line.replace(clean_regex, "");  // 去除特殊符号
        new_text += line + "\n";
    }

    undo_stack->push(new TextDispose(ui->textEdit, prev_text, new_text));
    ui->textEdit->setText(new_text);

    last_action = [this]() { filterSpecialChars(); };
}

// 提取相同内容
void ProcessText::extractEqualContent() {
    QString text = ui->textEdit->toPlainText();
    QString prev_text = text;
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);

    QHash<QString, int> line_counts;
    for (const QString &line : lines) {
        line_counts[line]++;
    }

    QStringList duplicate_lines;
    for (const QString &line : lines) {
        if (line_counts.value(line) > 1 && !duplicate_lines.contains(line)) {
            duplicate_lines.append(line);
        }
    }

    QString new_text = duplicate_lines.join("\n") + "\n";

    undo_stack->push(new TextDispose(ui->textEdit, prev_text, new_text));
    ui->textEdit->setText(new_text);

    last_action = [this]() { extractEqualContent(); };
}

// 提取中文字段
void ProcessText::extractCNValue() {
    QString text = ui->textEdit->toPlainText();
    QString prev_text = text;

    // 匹配所有中文字符（包括简体和繁体）
    QRegularExpression re("[\u4e00-\u9fa5]+");
    QRegularExpressionMatchIterator it = re.globalMatch(text);

    QStringList cn_values;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        cn_values.append(match.captured(0));
    }

    QString new_text = cn_values.join("\n") + "\n";

    undo_stack->push(new TextDispose(ui->textEdit, prev_text, new_text));
    ui->textEdit->setText(new_text);

    last_action = [this]() { extractCNValue(); };
}

// 提取网站
void ProcessText::extractWebsite() {
    QString text = ui->textEdit->toPlainText();
    QString perv_text = text;
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);
    QSet<QString> urls;

    QRegularExpression website_regular(R"(https?://[a-zA-Z0-9._~:/?#@!$&'()*+,;=%-]+)");

    QString new_text;

    for (const QString &line : lines) {
        QRegularExpressionMatchIterator i = website_regular.globalMatch(line);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString url = match.captured(0);

            if (!urls.contains(url)) {
                urls.insert(url);
                new_text += url + "\n";
            }
        }
    }

    undo_stack->push(new TextDispose(ui->textEdit, perv_text, new_text));
    ui->textEdit->setText(new_text);

    last_action = [this]() { extractWebsite(); };
}

// 提取相同域名的网站
void ProcessText::getEqualDomain() {
    QString text = ui->textEdit->toPlainText();
    QString perv_text = text;

    QStringList lines = text.split("\n", Qt::SkipEmptyParts);
    QSet<QString> domains;
    QStringList duplicates; // 用于存储重复域名的网址

    for (const QString& line : lines) {
        QUrl url(line.trimmed());
        if (url.isValid()) {
            QString domain = url.host(); // 提取域名
            if (domains.contains(domain)) {
                duplicates << line; // 如果域名已存在，则视为重复
            } else {
                domains.insert(domain); // 不重复则添加到集合
            }
        }
    }

    // 将重复域名的网址显示在文本框中
    QString new_text = duplicates.join("\n");
    undo_stack->push(new TextDispose(ui->textEdit, perv_text, new_text));
    ui->textEdit->setText(new_text);

    last_action = [this]() { getEqualDomain(); };
}

// 域名提取
void ProcessText::getDomain() {
    QString text = ui->textEdit->toPlainText();
    QString perv_text = text;

    // 使用正则表达式提取完整的域名（保留 http:// 和 www)
    QRegularExpression domainPattern(R"((https?://(?:www\.)?[a-zA-Z0-9-]+\.[a-zA-Z]{2,}))");
    QRegularExpressionMatchIterator it = domainPattern.globalMatch(text);

    QStringList domains;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        domains << match.captured(1);
    }

    QString new_text = domains.join("\n");

    // 将提取后的文本放回 QTextEdit
    undo_stack->push(new TextDispose(ui->textEdit, perv_text, new_text));
    ui->textEdit->setText(new_text);

    last_action = [this]() { getDomain(); };
}

// 替换正反斜杠
void ProcessText::replaceSlash() {
    // int value = judgeFolder();

    // if (value == 1) {
    //     processFolder([](QTextStream &in, QTextStream &out) {
    //         while (!in.atEnd()) {
    //             QString line = in.readLine();
    //             line.replace("\\", "/"); // 将所有反斜杠替换为正斜杠
    //             out << line << "\n";
    //         }
    //     });
    // } else if (value == 0) {
    //     processText([](QString &line) -> QString {
    //         line.replace("\\", "/"); // 将所有反斜杠替换为正斜杠
    //         return line;
    //     });
    // }

    QString text = ui->textEdit->toPlainText();
    QString perv_text = text;
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);

    for (int i = 0; i < lines.size(); i++) {
        QString line = lines[i];
        for (int j = 0; j < line.size(); j++) {
            if (line[j] == '\\') {
                lines[i].replace(QChar(0x005c), QChar('/'));
            } else if (line[j] == '/') {
                lines[i].replace(QChar(0x002f), QChar('\\'));
            }
        }
    }

    text = lines.join("\n");
    QString new_text = text;
    undo_stack->push(new TextDispose(ui->textEdit, perv_text, new_text));
    ui->textEdit->setText(text);

    last_action = [this]() { replaceSlash(); };
}

// 网络
// 筛选IPv4
void ProcessText::filterIPv4() {
    QString text = ui->textEdit->toPlainText();
    QString perv_text = text;
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);

    // IPv4 正则表达式匹配
    QRegularExpression ipv4Regex(R"(\b((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\b)");

    QStringList filteredLines;
    for (const QString &line : lines) {
        QRegularExpressionMatch match = ipv4Regex.match(line);
        if (match.hasMatch()) {
            filteredLines.append(match.captured(0)); // 添加匹配的 IPv4 地址
        }
    }

    text = filteredLines.join("\n");
    QString new_text = text;

    undo_stack->push(new TextDispose(ui->textEdit, perv_text, new_text));
    ui->textEdit->setText(text);

    last_action = [this]() { filterIPv4(); };
}

// 关于 的活动按钮
void ProcessText::on_aboutaction_triggered()
{
    About *dialog = new About();
    // dialog->exec();
}

// 清除文本框
void ProcessText::on_ClearTextBoxaction_triggered()
{
    ui->textEdit->clear();
}

