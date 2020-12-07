#include "AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    connect(ui->pushButton,&QPushButton::clicked,this,&AboutDialog::close);
    ui->lblAbout->setText("ESP8266 OTA Updater,<br>"
                          "You may do whatever you want with this program, instead of buying it.<br>"
                          "If you like it and want to use it,<br> you may <b>buy me a cofee at PayPal vladimir00 at gmail.com</b><br><br><br>"
                          "This program is using QT. Consider QT liceses at <a href=https://www.qt.io>www.qt.io</a>!");
    setFixedSize(size());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
