#include "errorindicator.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QStyle>

ErrorIndicator::ErrorIndicator(QWidget *parent, QString errName, QSize indSize)
    : QWidget{parent}
{
    this->setLayout(new QHBoxLayout());
    this->layout()->setContentsMargins(0,0,0,0);
    fBackground = new QFrame(this);
    fBackground->setLayout(new QHBoxLayout());
    this->layout()->addWidget(fBackground);
    this->layout()->setContentsMargins(0,0,0,0);

    lIndicator = new QLabel(this);
    lIndicator->setObjectName("ErrIndicator");
    lIndicator->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    lIndicator->setFixedSize(indSize);
    fBackground->layout()->addWidget(lIndicator);
    lName = new QLabel(errName, this);
    fBackground->layout()->addWidget(lName);
    lName->setFixedSize(60,45);
    lName->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
    lName->setObjectName("lName");
}

void ErrorIndicator::setState(bool isError) {
    lIndicator->setProperty("error", isError);

    lIndicator->style()->unpolish(lIndicator);
    lIndicator->style()->polish(lIndicator);
    lIndicator->update();

}