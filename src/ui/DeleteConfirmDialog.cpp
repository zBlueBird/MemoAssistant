#include "ui/DeleteConfirmDialog.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

DeleteConfirmDialog::DeleteConfirmDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("deleteConfirmDialog"));
    setWindowTitle(tr("删除备忘录"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    setFixedSize(380, 212);

    auto *surface = new QFrame(this);
    surface->setObjectName(QStringLiteral("deleteDialogSurface"));

    auto *warningIcon = new QLabel(QStringLiteral("!"), surface);
    warningIcon->setObjectName(QStringLiteral("deleteWarningIcon"));
    warningIcon->setAlignment(Qt::AlignCenter);
    warningIcon->setFixedSize(42, 42);

    auto *titleLabel = new QLabel(tr("删除这条备忘录？"), surface);
    titleLabel->setObjectName(QStringLiteral("deleteDialogTitle"));

    auto *messageLabel = new QLabel(tr("删除后将无法恢复。"), surface);
    messageLabel->setObjectName(QStringLiteral("deleteDialogMessage"));

    auto *textLayout = new QVBoxLayout;
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(6);
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(messageLabel);

    auto *contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(24, 25, 24, 8);
    contentLayout->setSpacing(16);
    contentLayout->addWidget(warningIcon, 0, Qt::AlignTop);
    contentLayout->addLayout(textLayout, 1);

    auto *cancelButton = new QPushButton(tr("取消"), surface);
    cancelButton->setObjectName(QStringLiteral("deleteCancelButton"));
    cancelButton->setFixedSize(88, 36);

    auto *deleteButton = new QPushButton(tr("删除"), surface);
    deleteButton->setObjectName(QStringLiteral("deleteConfirmButton"));
    deleteButton->setFixedSize(88, 36);
    deleteButton->setDefault(false);
    deleteButton->setAutoDefault(false);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(24, 8, 24, 20);
    buttonLayout->setSpacing(10);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(deleteButton);

    auto *surfaceLayout = new QVBoxLayout(surface);
    surfaceLayout->setContentsMargins(0, 0, 0, 0);
    surfaceLayout->setSpacing(0);
    surfaceLayout->addLayout(contentLayout, 1);
    surfaceLayout->addLayout(buttonLayout);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(surface);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(deleteButton, &QPushButton::clicked, this, &QDialog::accept);

    cancelButton->setFocus();
}
