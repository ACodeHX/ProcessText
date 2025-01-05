#ifndef CONTENT_H
#define CONTENT_H

#include <QDialog>

namespace Ui {
class Content;
}

class Content : public QDialog
{
    Q_OBJECT

public:
    explicit Content(QWidget *parent = nullptr);
    QString confirmContent();
    QString insertPosition();
    ~Content();

private:
    Ui::Content *ui;
};

#endif // CONTENT_H
