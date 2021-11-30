#include <QtTest/QtTest>
#include "tVideoControllerWidget.h"
#include "widgets/VideoControllerWidget.h"

void tVideoControllerWidget::init() {
    widget = new VideoControllerWidget;
}
void tVideoControllerWidget::cleanup() {
    delete widget ;
    widget = nullptr;
}

void tVideoControllerWidget::tsetframe(){
}