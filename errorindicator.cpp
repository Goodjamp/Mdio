#include "errorindicator.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>

ErrorIndicator::ErrorIndicator(QWidget *parent, QString errName, QSize indSize)
    : QWidget{parent}
{
    this->setLayout(new QHBoxLayout());
    this->layout()->setContentsMargins(0,0,0,0);
    fBackground = new QFrame(this);
    fBackground->setLayout(new QHBoxLayout());
    this->layout()->addWidget(fBackground);
    this->layout()->setContentsMargins(0,0,0,0);

    pbIndicator = new QPushButton(this);
    pbIndicator->setObjectName("ErrIndicator");
    pbIndicator->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    pbIndicator->setFixedSize(indSize);
    fBackground->layout()->addWidget(pbIndicator);
    lName = new QLabel(errName, this);
    fBackground->layout()->addWidget(lName);
    pbIndicator->setCheckable(false);
    pbIndicator->setChecked(false);
    lName->setFixedSize(60,45);
    lName->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
    lName->setObjectName("lName");
}

void ErrorIndicator::setState(bool isError) {
    pbIndicator->setChecked(isError);
}