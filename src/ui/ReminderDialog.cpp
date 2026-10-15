#include "ui/ReminderDialog.h"

#include "ui/IconFactory.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ReminderDialog::ReminderDialog(QWidget *parent)
    : QDialog(parent),
      m_messageLabel(new QLabel(this))
{
    setObjectName(QStringLiteral("reminderDialog"));
    setWindowTitle(tr("备忘录提醒"));
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowModality(Qt::NonModal);
    setFixedSize(420, 188);

    auto *surface = new QFrame(this);
    surface->setObjectName(QStringLiteral("reminderDialogSurface"));

    auto *iconLabel = new QLabel(surface);
    iconLabel->setObjectName(QStringLiteral("reminderDialogIcon"));
    iconLabel->setPixmap(IconFactory::applicationIcon().pixmap(30, 30));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(42, 42);

    auto *titleLabel = new QLabel(tr("备忘录提醒"), surface);
    titleLabel->setObjectName(QStringLiteral("reminderDialogTitle"));

    m_messageLabel->setObjectName(QStringLiteral("reminderDialogMessage"));
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto *textLayout = new QVBoxLayout;
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(7);
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(m_messageLabel);

    auto *contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(22, 22, 22, 8);
    contentLayout->setSpacing(14);
    contentLayout->addWidget(iconLabel, 0, Qt::AlignTop);
    contentLayout->addLayout(textLayout, 1);

    auto *confirmButton = new QPushButton(tr("知道了"), surface);
    confirmButton->setObjectName(QStringLiteral("reminderDialogConfirmButton"));
    confirmButton->setFixedSize(92, 36);
    confirmButton->setDefault(true);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(22, 6, 22, 18);
    buttonLayout->addStretch();
    buttonLayout->addWidget(confirmButton);

    auto *surfaceLayout = new QVBoxLayout(surface);
    surfaceLayout->setContentsMargins(0, 0, 0, 0);
    surfaceLayout->setSpacing(0);
    surfaceLayout->addLayout(contentLayout, 1);
    surfaceLayout->addLayout(buttonLayout);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(surface);

    connect(confirmButton, &QPushButton::clicked, this, &QDialog::accept);
}

void ReminderDialog::setMessage(const QString &message)
{
    m_messageLabel->setText(message);
}
