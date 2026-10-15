#include "MemoApplication.h"

#include "ui/IconFactory.h"

#include <QApplication>
#include <QCoreApplication>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("MemoAssistant"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("local.memoassistant"));
    QCoreApplication::setApplicationName(QStringLiteral("MemoAssistant"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setWindowIcon(IconFactory::applicationIcon());
    QLocale::setDefault(QLocale::system());

    MemoApplication memoApplication;
    if (!memoApplication.initialize()) {
        return 0;
    }
    return application.exec();
}
