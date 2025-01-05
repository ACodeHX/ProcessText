#ifndef PROCESSTEXT_H
#define PROCESSTEXT_H

#include <QMainWindow>
#include <iostream>
#include <string>
#include <QDebug>
#include <QFileDialog>
#include <QUndoStack>
#include <QUndoCommand>
#include <QShortcut>
#include <QMessageBox>
#include <QRegularExpression>
#include <functional>
#include "content.h"

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
    ~ProcessText();

private:
    Ui::ProcessText *ui;
    QUndoStack *undo_stack;
    void estimateVisible(QWidget *widget);

    Content *content_dialog;

private slots:
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
    // 处理网站内容
    void getDomain();
    void getEqualDomain();
    void pickCherryHREF();
    void getHREFvalue();
};
#endif // PROCESSTEXT_H
