#include "tVideoControllerWidget.h"

void tVideoControllerWidget::init() {
    widget = new VideoControllerWidget;
}
void tVideoControllerWidget::cleanup() {
    delete widget ;
    widget = nullptr;
}

void tVideoControllerWidget::tsetframe(){
}