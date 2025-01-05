#include "processtext.h"
#include "src/ui_processtext.h"

ProcessText::ProcessText(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ProcessText)
    , undo_stack(new QUndoStack(this))
    ,content_dialog(new Content(this))
{
    ui->setupUi(this);
    // 菜单栏
    // 文件菜单
    // connect(ui->openFileMenuAction, &QAction::triggered, this, &ProcessText::openFileMenu);
    // connect(ui->saveFileMenuAction, &QAction::triggered, this, &ProcessText::saveFileMenu);
    // connect(ui->undoAction, &QAction::triggered, undo_stack, &QUndoStack::undo);
    // connect(ui->redoAction, &QAction::triggered, undo_stack, &QUndoStack::redo);

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

    // 网站
    connect(ui->getHTTPvalueButton, &QPushButton::clicked, this, &ProcessText::pickCherryHREF);
    connect(ui->getEqualDomainButton, &QPushButton::clicked, this, &ProcessText::getEqualDomain);
    connect(ui->getDomainButton, &QPushButton::clicked, this, &ProcessText::getDomain);
    connect(ui->getHREFvalueButton, &QPushButton::clicked, this, &ProcessText::getHREFvalue);
}

ProcessText::~ProcessText()
{
    delete ui;
    delete undo_stack;
    delete content_dialog;
}

// 判断按钮是否隐藏
void ProcessText::estimateVisible(QWidget *widget) {
    if (widget->isVisible()) {
        widget->hide();
    } else {
        widget->show();
    }
}

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
    QString file_path = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("*.txt *.md *.cpp *.java *.python"));
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

}
// 添加引号按钮
void ProcessText::addQuotation() {
    processText([](const QString &line) {
        return "\"" + line + "\"";
    });
}

// 将中文符号替换成英文按钮
void ProcessText::replaceCNtoENsymbol() {
    processText([](QString &line) {
        line.replace(QChar(0xff08), QChar('('));
        line.replace(QChar(0xff09), QChar(')'));
        line.replace(QChar(0x3002), QChar('.'));
        return line;
    });
}

// 去除注释符
void ProcessText::removeAnnotators() {
    processText([](QString &line) {
        int index = line.indexOf("//");
        if (index != -1) {
            line = line.left(index) + line.mid(index + 2); // 去掉注释
        }
        return line.trimmed();
    });
}

// 替换大小写字母
void ProcessText::convertCase() {
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

// 将文本按字母升序排序
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
}

// 将文本按字母降序排序
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
}

// 提取 href 字段
void ProcessText::getHREFvalue() {
    QString text = ui->textEdit->toPlainText();
    QString perv_text = text;
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);
    QSet<QString> urls;

    QRegularExpression website_regular(R"(href="([^"]+)"")");

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
}

//
void ProcessText::pickCherryHREF() {
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
}

// 末尾添加逗号
void ProcessText::addLastComma() {
    QString text = ui->textEdit->toPlainText();
    QString perv_text = text;
    QStringList lines = text.split("\n", Qt::SkipEmptyParts);

    for (int i = 0; i < lines.size(); i++) {
        lines[i] = lines[i] + ",";
    }

    text = lines.join("\n");
    QString new_text = text;
    undo_stack->push(new TextDispose(ui->textEdit, perv_text, new_text));
    ui->textEdit->setText(text);
}

// 替换正反斜杠
void ProcessText::replaceSlash() {
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
}

