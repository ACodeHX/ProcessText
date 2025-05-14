#ifndef PROCESSTEXT_H
#define PROCESSTEXT_H

#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <string>
#include <QDebug>
#include <QFileDialog>
#include <QUndoStack>
#include <QUndoCommand>
#include <QShortcut>
#include <QMessageBox>
#include <QRegularExpression>
#include <functional>
#include <QProcess>

#include "content.h"
#include "about.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class ProcessText;
}
QT_END_NAMESPACE

class ProcessText : public QMainWindow
{
    Q_OBJECT

public:
    ProcessText(QWidget *parent = nullptr);
    inline bool judgeTextExist();
    ~ProcessText();

private:
    Ui::ProcessText *ui;
    QUndoStack *undo_stack;

    std::function<void()> last_action;
    Content *content_dialog;

    void estimateVisible(QWidget *widget);
    int judgeFolder();
    int judgeFile();
    QString replaceSymbols(const QString &line);
    void processFolder(const std::function<void(QTextStream &in, QTextStream &out)> &processLine);

private slots:
    // 执行上一个操作
    void repeatLastAction();
    // 菜单栏活动
    void openFileMenu();
    void saveFileMenu();
    // 按钮
    // 文本处理
    void processText(const std::function<QString(QString &)> &lineProcessor);
    // 添加内容
    void addContentBeform();
    void addQuotation();
    void addLastComma();
    // 修改内容
    void setUpSequence();
    void setDownSequence();
    void replaceCNtoENsymbol();
    void removeAnnotators();
    void convertCase();
    void replaceSlash();
    // 提取内容
    void filterSpecialChars();
    void extractEqualContent();
    // 处理网站内容
    void getDomain();
    void getEqualDomain();
    void extractWebsite();
    void on_closeaction_triggered();
    void on_aboutaction_triggered();
    void on_openfolderaction_triggered();
    void on_ClearTextBoxaction_triggered();
    // 网络
    void filterIPv4();
};
#endif // PROCESSTEXT_H
