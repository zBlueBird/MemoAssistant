#include "ui/MemoDetailsDialog.h"

#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace {
QString formatDateTime(const QDateTime &dateTime)
{
    return dateTime.isValid()
               ? dateTime.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm"))
               : QObject::tr("未设置");
}

void addDetailRow(QGridLayout *layout, int row, const QString &name, const QString &value)
{
    auto *nameLabel = new QLabel(name);
    nameLabel->setObjectName(QStringLiteral("detailsFieldName"));
    auto *valueLabel = new QLabel(value);
    valueLabel->setObjectName(QStringLiteral("detailsFieldValue"));
    valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueLabel->setWordWrap(true);
    layout->addWidget(nameLabel, row, 0, Qt::AlignTop);
    layout->addWidget(valueLabel, row, 1);
}
}

MemoDetailsDialog::MemoDetailsDialog(const Memo &memo, QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("memoDetailsDialog"));
    setWindowTitle(tr("备忘录详情"));
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(460, 410);
    resize(460, 450);

    auto *surface = new QFrame(this);
    surface->setObjectName(QStringLiteral("detailsSurface"));

    auto *heading = new QLabel(tr("备忘录详情"), surface);
    heading->setObjectName(QStringLiteral("detailsHeading"));
    auto *closeButton = new QPushButton(QStringLiteral("×"), surface);
    closeButton->setObjectName(QStringLiteral("detailsCloseButton"));
    closeButton->setFixedSize(34, 32);

    auto *header = new QHBoxLayout;
    header->setContentsMargins(20, 12, 9, 4);
    header->addWidget(heading);
    header->addStretch();
    header->addWidget(closeButton);

    auto *titleLabel = new QLabel(memo.title, surface);
    titleLabel->setObjectName(QStringLiteral("detailsTitle"));
    titleLabel->setWordWrap(true);
    titleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto *statusLabel = new QLabel(memo.completed ? tr("已完成") : tr("未完成"), surface);
    statusLabel->setObjectName(QStringLiteral("detailsStatus"));
    statusLabel->setProperty("completed", memo.completed);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFixedSize(62, 25);

    auto *titleRow = new QHBoxLayout;
    titleRow->setContentsMargins(20, 6, 20, 4);
    titleRow->setSpacing(10);
    titleRow->addWidget(titleLabel, 1);
    titleRow->addWidget(statusLabel, 0, Qt::AlignTop);

    auto *content = new QTextBrowser(surface);
    content->setObjectName(QStringLiteral("detailsContent"));
    content->setPlainText(memo.content.isEmpty() ? tr("没有补充说明。") : memo.content);
    content->setMinimumHeight(105);
    content->setMaximumHeight(150);

    auto *details = new QGridLayout;
    details->setContentsMargins(20, 10, 20, 6);
    details->setHorizontalSpacing(16);
    details->setVerticalSpacing(8);
    details->setColumnStretch(1, 1);
    addDetailRow(details, 0, tr("计划日期"), memo.scheduledDate.toString(QStringLiteral("yyyy-MM-dd dddd")));
    addDetailRow(details, 1, tr("提醒时间"), formatDateTime(memo.reminderAt));
    addDetailRow(details, 2, tr("创建时间"), formatDateTime(memo.createdAt));
    addDetailRow(details, 3, tr("更新时间"), formatDateTime(memo.updatedAt));
    if (memo.completed) {
        addDetailRow(details, 4, tr("完成时间"), formatDateTime(memo.completedAt));
    }

    auto *confirmButton = new QPushButton(tr("关闭"), surface);
    confirmButton->setObjectName(QStringLiteral("detailsConfirmButton"));
    confirmButton->setFixedSize(88, 34);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->setContentsMargins(20, 6, 20, 16);
    buttonRow->addStretch();
    buttonRow->addWidget(confirmButton);

    auto *surfaceLayout = new QVBoxLayout(surface);
    surfaceLayout->setContentsMargins(0, 0, 0, 0);
    surfaceLayout->setSpacing(0);
    surfaceLayout->addLayout(header);
    surfaceLayout->addLayout(titleRow);
    surfaceLayout->addWidget(content, 1);
    surfaceLayout->addLayout(details);
    surfaceLayout->addLayout(buttonRow);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(surface);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(confirmButton, &QPushButton::clicked, this, &QDialog::accept);
}
