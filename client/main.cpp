#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QStyleHints>
#include <global.h>

static QString readStyleFile(const QString& path)
{
    QFile qss(path);
    if (!qss.open(QFile::ReadOnly)) {
        qDebug("Open failed");
        return {};
    }

    qDebug("open success");
    return QString::fromUtf8(qss.readAll());
}

static void applyAppStyle(QApplication& app)
{
    const bool darkMode = app.styleHints()->colorScheme() == Qt::ColorScheme::Dark;
    auto style = readStyleFile(":/style/stylesheet.qss");
    if (darkMode) {
        style += "\n";
        style += readStyleFile(":/style/dark_stylesheet.qss");
    }

    app.setProperty("llfc_theme", darkMode ? "dark" : "light");
    app.setStyleSheet(style);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    applyAppStyle(a);
    QObject::connect(a.styleHints(), &QStyleHints::colorSchemeChanged,
                     &a, [&a](Qt::ColorScheme) {
                         applyAppStyle(a);
                     });

    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    // 拼接文件名
    QString fileName = "config.ini";
    QString config_path = QDir::toNativeSeparators(app_path +
                                                   QDir::separator() + fileName);

    QSettings settings(config_path, QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix = "http://"+gate_host+":"+gate_port;
    qDebug() << "final url =" << gate_url_prefix + "/get_varifycode";
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
